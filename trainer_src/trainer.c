#include "trainer.h"

double	learn_epoch(t_trainer *trainer, t_dataset *dataset, double lr)
{
	double	total_loss = 0;
	double	target[1];

	for (size_t i = 0; i < dataset->size; i++)
	{
		if (trainer_forward(trainer, dataset->colors[i].rgb) != 0)
			return (-1);
		target[0] = dataset->colors[i].is_red;
		total_loss += trainer_loss(trainer, target);
		if (trainer_backward(trainer, target))
			return (-1);
		trainer_update(trainer, lr);
	}
	return (total_loss/dataset->size);
}

double	test_model(t_trainer *trainer, t_dataset *testset)
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
		if(trainer_forward(trainer, testset->colors[i].rgb) != 0)
			return (-1);
		p = trainer_output(trainer)[0];
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
	printf("Test: accuracy - %lf,", accuracy);
	if (total_red > 0)
	{
		red_accuracy = (double)correct_red / (double)total_red * 100.0;
		printf(" red accuracy - %lf,", red_accuracy);
	}
	else
		printf(" red accuracy - n/a,");
	if (total_not_red > 0)
	{
		not_red_accuracy = (double)correct_not_red / (double)total_not_red * 100.0;
		printf(" not red accuracy - %lf\n", not_red_accuracy);
	}
	else
		printf(" not red accuracy - n/a\n");
	return (accuracy);
}

void	train_model(t_trainer *trainer, t_dataset *dataset, t_dataset *testset,
	int epochs, double lr)
{
	double	avrage_loss = 0;

	for	(int i = 1; i <= epochs; i++)
	{
		scramble_dataset(dataset);
		avrage_loss = learn_epoch(trainer, dataset, lr);
		printf("Epoch %d: avrage loss - %lf\n", i, avrage_loss);
		if (i % 10 == 0)
		{
			test_model(trainer, testset);
			//printf("Test %d: accuracy - %lf\n", i / 10, accuracy);
		}
		lr *= 0.96;
	}
}

int main(int argc, char **argv)
{
	t_dataset	*dataset;
	t_dataset	*testset;
	size_t		shape[4] = {3, 4, 4, 1};
	t_model		*model;
	t_trainer	*trainer;

	srand(1234567890);
	if (argc != 3 && argc != 4)
		fail_exit("Usage ./trainer <dataset file> <test file> <model output file(optional)>");
	dataset = load_dataset(argv[1]);
	if (dataset->size == 0)
		fail_exit("Dataset is empty!");
	print_dataset_stats(argv[1], dataset);
	testset = load_dataset(argv[2]);
	if (testset->size == 0)
		fail_exit("Testset is empty");
	print_dataset_stats(argv[2], testset);
	model = model_create(shape, 4);
	if (!model)
	{
		free_dataset(dataset);
		free_dataset(testset);
		fail_exit("Something went wrong while allocationg the model!");
	}
	model_init_random(model, 1);
	trainer = trainer_create(model);
	if (!trainer)
	{
		free_dataset(dataset);
		free_dataset(testset);
		model_free(model);
		fail_exit("Something went wrong while allocationg the trainer!");
	}
	train_model(trainer, dataset, testset, 150, 0.5);
	if (argc == 4)
		model_save(model, argv[3]);
	trainer_free(trainer);
	model_free(model);
	free_dataset(dataset);
	free_dataset(testset);
	return (0);
}
