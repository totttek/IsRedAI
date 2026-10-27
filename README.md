# IsRedAI

A tiny neural network written from scratch in C, with no ML libraries: it learns to tell whether an RGB pixel is "red".

The point of the project wasn't the classifier. It was to understand what actually happens inside a neural network by building every piece by hand: generating the data, deriving backpropagation on paper, writing the training loop, and then running experiments to see *why* things work or fail.

## The task

A pixel counts as red when **both** conditions hold:

```
R > 102          (enough red, about 40% of 255)
R > G + B        (red dominates the other two channels combined)
```

The network never sees this rule. It only sees labelled examples and has to discover the rule from them.

## Project structure

```
IsRedAI/
├── Makefile
├── generator_src/
│   └── generator.c        dataset generator (writes CSV files)
├── libmodel/              general neural network library: create, train, save, load, run
│                          (see libmodel/README.md)
├── predictor_src/
│   └── predictor.c        loads a .model file and classifies one RGB value
├── trainer_src/
│   ├── trainer.h
│   ├── trainer.c          training loop and evaluation, built on libmodel
│   └── dataset_utils.c    CSV loading, stats, shuffling
├── *.model                trained models (libmodel format)
└── *.csv                  generated datasets
```

## Build & run

```sh
make            # builds libmodel, ./generator, ./trainer and ./predictor
make data       # regenerates all datasets from fixed seeds (identical every time)
./trainer train_uniform_boundry.csv test.csv my.model   # trains, saves the model to my.model
./predictor my.model 200 30 20                          # classifies one pixel
```

### Generator

```
./generator <seed> <count> <output.csv> <boundary 0-100> [ratio 0-100]
```

Each line of the output is `r,g,b,label` with integer channels 0-255. Labels always come from the rule, never from how a pixel was generated.

- **seed**: fixed seed, so the same arguments always produce the same file
- **boundary**: percentage of pixels generated *close to the decision boundary*: half near `R = 102`, half near `R = G + B`, using a triangular offset (sum of two uniform randoms), so most of them sit right next to the boundary
- **ratio** *(optional)*: forces this percentage of the non-boundary pixels to be red. Without it, the pixels are uniform random (about 15.6% come out red).

Datasets produced by `make data`:

| File | Contents |
|---|---|
| `test.csv` | 2000 uniform pixels: the shared, never-trained-on test set |
| `train_uniform.csv` | 5000 uniform pixels |
| `train_50.csv` | 5000 pixels, about 50% red |
| `train_uniform_boundry.csv` | 5000 pixels, 30% of them near the boundary |

### Trainer

```
./trainer <train.csv> <test.csv> [output.model]
```

Loads the CSVs (normalising channels to 0-1), prints dataset stats including the majority-class baseline, trains a network with `libmodel`, evaluates on the test set every 10 epochs (overall, red-only and non-red-only accuracy), and optionally saves the trained model. The network shape, seed, epochs, learning rate and decay are set in `main` / `train_model` (currently shape `{3, 4, 4, 1}`, 150 epochs, lr 0.5 × 0.96 per epoch, fixed seed).

### Predictor

```
./predictor <model file> <R> <G> <B>
```

Loads a trained model and prints whether the colour (channels 0-255) is red.

### Model files

| File | Network | Trained with |
|---|---|---|
| `is_red_ai.model` | 3 → 2 → 1 | the original hand-written two-layer trainer (converted to the v2 format) |
| `is_red_ai_v2.model` | 3 → 2 → 1 | the `libmodel` trainer |
| `is_red_ai_v3.model` | 3 → 4 → 4 → 1 | the `libmodel` trainer |

## The model

**Version 1, a single neuron (logistic regression):**

```
z = w1*R + w2*G + w3*B + b
p = sigmoid(z) = 1 / (1 + e^-z)
```

**Version 2, a hidden layer (3 → 2 → 1):**

```
h1 = sigmoid(w11*R + w12*G + w13*B + b1)
h2 = sigmoid(w21*R + w22*G + w23*B + b2)
p  = sigmoid(v1*h1 + v2*h2 + c)
```

11 parameters in total.

**Version 3, any shape (`libmodel`):** the fixed two-layer code was replaced by a general library, where a network is a list of layers, each a flat weight matrix plus biases and an activation. The same training loop works for `3 → 2 → 1`, `3 → 4 → 4 → 1` or anything else.

Training (all versions): binary cross-entropy loss, plain SGD (one update per pixel), Fisher-Yates shuffle every epoch, weights initialised uniformly in [-1, 1] (to break symmetry between the hidden neurons), learning rate 0.5 multiplied by 0.96 after each epoch.

### Backpropagation, derived by hand

Loss for one pixel with label `y`:

```
L = -( y*log(p) + (1-y)*log(1-p) )
```

Using the chain rule and the sigmoid derivative `p*(1-p)`, the cross-entropy and the sigmoid cancel out:

```
dL/dz_out = p - y
```

Output layer: `dL/dv_i = (p - y) * h_i` and `dL/dc = p - y`

Hidden layer: the error is passed back through `v_i` and the sigmoid slope:

```
δ_i      = (p - y) * v_i * h_i * (1 - h_i)
dL/dw_ij = δ_i * input_j
dL/db_i  = δ_i
```

Every gradient has the same shape: **(error at the neuron) × (input to the weight)**.

For any number of layers, the hidden-layer rule generalises: neuron `i` of layer `k` collects error from **every** neuron `n` of layer `k+1` it feeds, through the connecting weight, then scales it by its own sigmoid slope:

```
incoming_i = Σ_n  δ_(k+1),n * w_(k+1),n,i
δ_k,i      = incoming_i * h_i * (1 - h_i)
```

All deltas are computed (from the output layer backwards) before any weight changes, so the whole gradient describes one consistent model.

## Results

All runs are evaluated on the same uniform test set (baseline: always answering "not red" gives about 84%).

| Model | Training data | Test accuracy | Red | Not red |
|---|---|---|---|---|
| 1 neuron | uniform | 98.4% | ~92% | ~99.7% |
| 1 neuron | 50% red | 98.1% | ~99.4% | ~97.8% |
| 2 layers | uniform, lr decay | 99.85% | 99.7% | 99.9% |
| **2 layers** | **30% boundary, lr decay, 150 epochs** | **100%** | **100%** | **100%** |
| 3 → 2 → 1 on `libmodel` | 30% boundary, same settings | 100% | 100% | 100% |
| 3 → 4 → 4 → 1 on `libmodel` | 30% boundary, same settings | 100% | 100% | 100% |

The hand-written two-layer model held 100% at every checkpoint from epoch 80 to epoch 150. The same network trained with `libmodel` reached 100% at epoch 70 and held it from epoch 90 on, with nearly the same final loss (0.035 vs 0.036), which verified the library's general backpropagation. The deeper `3 → 4 → 4 → 1` network held 100% from epoch 60 on, with a final loss of 0.012.

*Note: the test set was regenerated once partway through, when the generator changed (320 → 330 red pixels). The two-layer rows use the newer test set.*

## What the experiments showed

**1. One neuron can't learn a two-condition rule.** A single neuron is one linear inequality, a single straight cut through RGB space. The rule needs two cuts at once, so the loss plateaus (about 0.046) no matter how long it trains. Reading its weights shows the compromise it settles on:

```
R > 0.85*(G + B) + 0.135
```

That's one tilted cut sitting between the two real ones, which misclassifies dark reds and pixels near `R ≈ G + B`.

**2. Training data decides *which* mistakes a limited model makes.** Training the single neuron on 50% red data instead of about 15% barely changed the overall accuracy, but flipped the balance: far fewer missed reds, more false alarms. The cut shifted toward the non-red side, because the loss is a sum over training examples.

**3. Learning rate trades speed for stability.** lr 0.5 converges fast but bounces; lr 0.05 is smooth but slow. Both found the same cut. Weight *magnitude* turned out to be the model's confidence: scaling all weights keeps the cut in place but sharpens the sigmoid.

**4. A hidden layer discovers the rule's structure on its own.** With 2 hidden neurons, each one learned one of the two conditions, with no supervision on the hidden layer:

| Neuron | Learned weights (rounded) | Meaning |
|---|---|---|
| h1 | `55*R + 0.7*G + 0.3*B - 22.1` | R > 0.402 → **R > 102.5** |
| h2 | `-43*R + 43*G + 43*B - 0.02` | **G + B > R** (the second rule, inverted) |
| out | `26.6*h1 - 26.9*h2 - 13.5` | **h1 AND NOT h2** |

Hidden neuron 2 learned the *negation* of `R > G + B`, and the output neuron compensates with a negative weight. Features can be learned in either polarity.

**5. Boundary-focused data places the cuts precisely.** Trained on uniform data, the threshold neuron settled at R > ~109 instead of 102, because very few uniform pixels sit near the boundary. With 30% boundary pixels, it moved to **102.5**, exactly halfway between the last non-red value (102) and the first red one (103). Training loss on that dataset is *higher* (it's full of hard examples), which is a reminder that training losses on different datasets aren't comparable. The shared test set is the fair comparison.

**6. A bigger network learns the same rule, with redundancy.** `3 → 4 → 4 → 1` (41 parameters instead of 11) reached 100% sooner and ended with a third of the loss. Reading its weights shows why:

| Layer | What the neurons learned |
|---|---|
| hidden 1 | both real rules again, both inverted (`R < ~0.41`, `G + B > R`), plus one tilted compromise cut and one weak, barely used neuron |
| hidden 2 | two near-identical "one of the not-red reasons is true" detectors, one "neither reason is true" (red) detector, and one barely used neuron |
| output | red when the red detector is on and the two not-red detectors are off |

The extra capacity didn't find a better rule. It found the same one several times over, and that redundancy is where the extra confidence (lower loss) comes from. It also costs about four times the parameters and compute for the same test accuracy, and it shows why big networks are hard to interpret: even here, the rule is spread over duplicates, inverted features and half-unused neurons.

## Next steps

- MNIST handwritten digits (softmax output, real data) on top of `libmodel`
- Mixture-of-Experts style routing between small sub-networks
- Comparing sparse/routed models against dense ones with the same number of active and total parameters
