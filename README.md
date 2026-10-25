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
├── libmodel/               shared model library (see libmodel/README.md)
├── predictor_src/
│   └── predictor.c        loads model.csv and classifies one RGB value
├── trainer_src/
│   ├── trainer.h
│   ├── trainer.c          model, forward/backward pass, training loop
│   └── dataset_utils.c    CSV loading, stats, shuffling
└── *.csv                  generated datasets
```

## Build & run

```sh
make            # builds ./generator and ./trainer
make data       # regenerates all datasets from fixed seeds (identical every time)
./trainer train_uniform_boundry.csv test.csv model.csv   # trains, saves weights to model.csv
./predictor model.csv 200 30 20                          # classifies one pixel
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

Loads the CSVs (normalising channels to 0-1), prints dataset stats including the majority-class baseline, trains, and evaluates on the test set every 10 epochs, reporting overall, red-only and non-red-only accuracy. Hyperparameters (epochs, learning rate, decay) are set in `main` / `train_model`.

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

11 parameters in total. Training: binary cross-entropy loss, plain SGD (one update per pixel), Fisher-Yates shuffle every epoch, weights initialised uniformly in [-1, 1] (to break symmetry between the hidden neurons), learning rate 0.5 multiplied by 0.96 after each epoch.

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

## Results

All runs are evaluated on the same uniform test set (baseline: always answering "not red" gives about 84%).

| Model | Training data | Test accuracy | Red | Not red |
|---|---|---|---|---|
| 1 neuron | uniform | 98.4% | ~92% | ~99.7% |
| 1 neuron | 50% red | 98.1% | ~99.4% | ~97.8% |
| 2 layers | uniform, lr decay | 99.85% | 99.7% | 99.9% |
| **2 layers** | **30% boundary, lr decay, 150 epochs** | **100%** | **100%** | **100%** |

The final model held 100% at every checkpoint from epoch 80 to epoch 150.

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

## Next steps

- General layers of any size (heap-allocated weight matrices instead of fixed `[2][3]` arrays)
- More hidden neurons: what do the extra ones learn when two cuts are enough?
- MNIST handwritten digits (softmax output, real data)
- Mixture-of-Experts style routing between small sub-networks
