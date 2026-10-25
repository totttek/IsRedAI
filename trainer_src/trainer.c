#include "trainer.h"

double	predict_neuron(double *weights, double bias, double *input, size_t size)
{
	double	z = 0;

	for(size_t i = 0; i < size; i++)
		z += weights[i] * input[i];
	z += bias;
	return (1.0 / (1.0 + exp(-z)));
}

double	predict_model(t_model *model, double *input)
{
	double	h[2] = {0.0, 0.0};

	h[0] = predict_neuron(model->w[0], model->b[0], input,
		model->h_weights);
	h[1] = predict_neuron(model->w[1], model->b[1], input,
		model->h_weights);
	return (predict_neuron(model->v, model->c, h, model->h_neurons));
}

void	teach_neuron(double	*weights, double *bias, double *input,
		double error, double lr, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		weights[i] -= input[i] * error * lr;
	}
	*bias -= error * lr;
}

double	learn_epoch(t_model *model, t_dataset *dataset, double lr)
{
	double	total_loss = 0;
	double	h[2] = {0.0, 0.0};
	double	p = 0;
	double	y = 0;
	double	error = 0;
	double	h_error[2] = {0.0, 0.0};
	double	pc = 0;

	for (size_t i = 0; i < dataset->size; i++)
	{
		h[0] = predict_neuron(model->w[0], model->b[0], dataset->colors[i].rgb,
			model->h_weights);
		h[1] = predict_neuron(model->w[1], model->b[1], dataset->colors[i].rgb,
			model->h_weights);
		p = predict_neuron(model->v, model->c, h,
			model->o_weights);
		pc = fmax(1e-7, fmin(p, 1.0 - 1e-7));
		y = dataset->colors[i].is_red;
		error = p - y;
		h_error[0] = error * model->v[0] * h[0] * (1 - h[0]);
		h_error[1] = error * model->v[1] * h[1] * (1 - h[1]);

		total_loss -= y * log(pc) + (1 - y) * log(1 - pc);

		teach_neuron(model->w[0], &(model->b[0]), dataset->colors[i].rgb, h_error[0], lr, model->h_weights);
		teach_neuron(model->w[1], &(model->b[1]), dataset->colors[i].rgb, h_error[1], lr, model->h_weights);
		teach_neuron(model->v, &model->c, h, error, lr, model->o_weights);
	}
	return (total_loss/dataset->size);
}

double	test_model(t_model *model, t_dataset *testset)
{
	double	p = 0;
	int	correct_red = 0;
	int	total_red = 0;
	int	correct_not_red = 0;
	int	total_not_red = 0;
	double	red_accuracy = 0;
	double	not_red_accuracy = 0;
	double	accuracy = 0;

	for (size_t i = 0; i < testset->size; i++)
	{
		p = predict_model(model, testset->colors[i].rgb);
		if (testset->colors[i].is_red == 1)
		{
			total_red++;
			if (p > 0.5)
				correct_red++;
		}
		else if (testset->colors[i].is_red == 0)
		{
			total_not_red++;
			if (p <= 0.5)
				correct_not_red++;
		}
	}
	accuracy = (double)(correct_red +correct_not_red) / (double)testset->size * 100.0;
	red_accuracy = (double)correct_red / (double)total_red * 100.0;
	not_red_accuracy = (double)correct_not_red / (double)total_not_red * 100.0;
	printf("Test: accuracy - %lf, red accuracy - %lf, not red accuracy - %lf\n",
			accuracy, red_accuracy, not_red_accuracy);
	return (accuracy);
}

void	train_model(t_model *model, t_dataset *dataset, t_dataset *testset,
	int epochs, double lr)
{
	double	avrage_loss = 0;

	for	(int i = 1; i <= epochs; i++)
	{
		scramble_dataset(dataset);
		avrage_loss = learn_epoch(model, dataset, lr);
		printf("Epoch %d: avrage loss - %lf\n", i, avrage_loss);
		if (i % 10 == 0)
		{
			test_model(model, testset);
			//printf("Test %d: accuracy - %lf\n", i / 10, accuracy);
		}
		lr *= 0.96;
	}
}

t_model	*init_model()
{
	t_model	*new_model;

	new_model = calloc(1, sizeof(t_model));
	if (!new_model)
		return (NULL);
	new_model->h_neurons = 2;
	new_model->h_weights = 3;
	new_model->o_neurons = 1;
	new_model->o_weights = 2;
	for (size_t	i = 0; i < new_model->h_neurons; i++)
	{
		for (size_t j = 0; j < new_model->h_weights; j++)
		{
			new_model->w[i][j] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
		}
		new_model->b[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
		new_model->v[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
	}
	new_model->c = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
	return (new_model);
}

void	save_model_csv(t_model* model, char *filename)
{
	FILE *file = fopen(filename, "w");
	if (!file)
	{
		printf("Warning: Could not create a file!");
		return ;
	}
	fprintf(file, "%zu,%zu,%zu\n", model->h_weights, model->h_neurons, model->o_neurons);
	for (size_t i = 0; i < model->h_neurons; i++)
	{
		for (size_t j = 0; j < model->h_weights; j++)
		{
			fprintf(file, "%.17g,", model->w[i][j]);
		}
		fprintf(file, "%.17g\n", model->b[i]);
	}
	for (size_t i = 0; i < model->o_weights; i++)
		fprintf(file, "%.17g,", model->v[i]);
	fprintf(file, "%.17g\n", model->c);
	fclose(file);
}

int main(int argc, char **argv)
{
	t_dataset	*dataset;
	t_dataset	*testset;
	t_model		*model;

	if (argc != 3 && argc != 4)
		fail_exit("Usage ./trainer <dataset file> <test file> <model output file(optional)>");
	dataset = load_dataset(argv[1]);
	print_dataset_stats(argv[1], dataset);
	testset = load_dataset(argv[2]);
	print_dataset_stats(argv[2], testset);
	model = init_model();
	if (!model)
	{
		free_dataset(dataset);
		free_dataset(testset);
		fail_exit("Failed to allocate memory for the model!");
	}
	train_model(model, dataset, testset, 150, 0.5);
	printf("Hidden Neuron 1: w1 %lf, w2 %lf, w3 %lf, b %lf\n",
		model->w[0][0], model->w[0][1], model->w[0][2], model->b[0]);
	printf("Hidden Neuron 2: w1 %lf, w2 %lf, w3 %lf, b %lf\n",
		model->w[1][0], model->w[1][1], model->w[1][2], model->b[1]);
	printf("Output Neuron: v1 %lf, v2 %lf, c %lf\n",
		model->v[0], model->v[1], model->c);
	if (argc == 4)
		save_model_csv(model, argv[3]);
	free_dataset(dataset);
	free_dataset(testset);
	free(model);
	return (0);
}
