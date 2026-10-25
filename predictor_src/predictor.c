#include "model.h"

int	main(int argc, char **argv)
{
	double	rgb[3];
	char	*endptr = NULL;
	t_model	*model = NULL;
	double	*answer = NULL;

	if (argc != 5)
	{
		printf("Usage ./predictor <model file (.csv)> <RED> <GREEN> <BLUE>\n");
		return (1);
	}

	rgb[0] = strtod(argv[2], &endptr) / 255;
	if (endptr == argv[2] || *endptr != '\0')
	{
		printf("Usage ./predictor <model file (.csv)> <RED> <GREEN> <BLUE>\n");
		return (1);
	}

	rgb[1] = strtod(argv[3], &endptr) / 255;
	if (endptr == argv[3] || *endptr != '\0')
	{
		printf("Usage ./predictor <model file (.csv)> <RED> <GREEN> <BLUE>\n");
		return (1);
	}

	rgb[2] = strtod(argv[4], &endptr) / 255;
	if (endptr == argv[4] || *endptr != '\0')
	{
		printf("Usage ./predictor <model file (.csv)> <RED> <GREEN> <BLUE>\n");
		return (1);
	}

	model = load_model(argv[1]);
	if (!model)
	{
		printf("Something went wrong while trying to load the model!\n");
		return (1);
	}

	answer = forward(model, rgb);
	if (!answer)
	{
		printf("Something went wrong while trying to generate the answer!\n");
		return (free_model(model), 1);
	}
	if (answer[0] > 0.5)
		printf("Red!\n");
	else
		printf("Not Red!\n");
	return (free_model(model), free(answer), 0);
}
