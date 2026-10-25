#ifndef TRAINER_H
# define TRAINER_H

# include <stdlib.h>
# include <stdio.h>
# include <math.h>

typedef struct	s_color
{
	double	rgb[3];
	int	is_red;
}				t_color;

typedef struct	s_dataset
{
	size_t size;
	t_color	*colors;
}				t_dataset;

typedef struct	s_model
{
	double	w[2][3];
	double	b[2];
	double	v[2];
	double	c;
	size_t	h_neurons;
	size_t	h_weights;
	size_t	o_neurons;
	size_t	o_weights;
}				t_model;

void		fail_exit(char *message);
void		free_dataset(t_dataset *dataset);
void		print_dataset_stats(char *dataset_name, t_dataset *dataset);
t_dataset	*load_dataset(char *filename);
void		scramble_dataset(t_dataset *dataset);

#endif
