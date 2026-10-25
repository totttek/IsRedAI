#include "trainer.h"

void	fail_exit(char *message)
{
	printf("Error: %s\n", message);
	exit(1);
}

void	free_dataset(t_dataset *dataset)
{
	free(dataset->colors);
	free(dataset);
}

void	print_dataset_stats(char *dataset_name, t_dataset *dataset)
{
	size_t reds = 0;
	double red_prc = 0;
	double baseline = 0;

	for (size_t i = 0; i < dataset->size; i++)
	{
		if (dataset->colors[i].is_red == 1)
			reds++;
	}
	red_prc = (double)reds / (double)dataset->size * 100.0;
	if (red_prc < 50.0)
		baseline = 100.0 - red_prc;
	else
		baseline = red_prc;
	printf("%s: %ld colors, %ld reds (%.2lf%%), baseline %.2lf%%\n",
		dataset_name, dataset->size, reds, red_prc, baseline);
}

t_dataset	*load_dataset(char *filename)
{
	t_dataset	*dataset = NULL;
	t_color	*colors = NULL;
	size_t	size_of_colors = 2;

	dataset = calloc(1, sizeof(t_dataset));
	if (!dataset)
		fail_exit("Failed to allocate memory for dataset!");
	dataset->colors = calloc(size_of_colors, sizeof(t_color));
	if (!dataset->colors)
	{
		free_dataset(dataset);
		fail_exit("Failed to allocate memory for collors!");
	}
	FILE *file = fopen(filename, "r");
	if (!file)
	{
		free_dataset(dataset);
		fail_exit("Failed to open a file containing provided dataset!");
	}
	colors = dataset->colors;
	for (size_t i = 0; i <= size_of_colors; i++)
	{
		if (i == size_of_colors)
		{
			size_of_colors *= 2;
			colors = realloc(colors, size_of_colors * sizeof(t_color));
			if (!colors)
			{
				free_dataset(dataset);
				fail_exit("Failed to reallocate memory for colors!");
			}
			dataset->colors = colors;
		}
		if (fscanf(file, "%lf,%lf,%lf,%d\n", 
				&colors[i].rgb[0], &colors[i].rgb[1],
				&colors[i].rgb[2], &colors[i].is_red) < 4)
		{
			dataset->size = i;
			break;
		}
		colors[i].rgb[0] /= 255.0;
		colors[i].rgb[1] /= 255.0;
		colors[i].rgb[2] /= 255.0;
	}
	fclose(file);
	return (dataset);
}

void	scramble_dataset(t_dataset *dataset)
{
	t_color tmp;
	size_t	j;

	if (dataset->size == 0)
		return ;
	for(size_t i = dataset->size - 1; i > 0; i--)
	{
		j = rand() % (i + 1);
		tmp = dataset->colors[j];
		dataset->colors[j] = dataset->colors[i];
		dataset->colors[i] = tmp;
	}
}
