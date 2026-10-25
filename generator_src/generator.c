#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int	is_red(int r, int g, int b)
{
	if (r > 102 && r > g + b)
		return (1);
	return (0);
}

double	rand01()
{
	return((double)rand() / RAND_MAX);
}

double	tri_offset(double k)
{
	return((rand01() + rand01() - 1) * k);
}

//int	clamp(int x)
//{
//	if (x > 255)
//		return (255);
//	if (x < 0)
//		return (0);
//	return (x);
//}

void	gen_near_threshold(int *r, int *g, int *b)
{
	*r = 102 + lround(tri_offset(10));
	*g = rand() % (*r/4);
	*b = rand() % (*r/4);
}

void	gen_near_sum(int *r, int *g, int *b)
{
	int s = 115 + rand() % 126;

	*g = rand() % (s + 1);
	*b = s - *g;
	*r = s + lround(tri_offset(10));
}

void	generat_dataset(int seed, int count, char *filename, int ratio, int boundry)
{
	int r = 0;
	int g = 0;
	int b = 0;
	int roll = 0;
	FILE *file = fopen(filename, "w");

	srand(seed);
	for(int i = 0; i < count; i++)
	{
		roll = rand() % 100;
		if (roll < boundry)
		{
			if (rand() % 2 == 0)
				gen_near_threshold(&r, &g, &b);
			else
				gen_near_sum(&r, &g, &b);
		}
		else if (ratio >= 0)
		{
			roll = rand() % 100;
			if (roll < ratio)
			{
				do
				{
					r = 103 + (rand() % 153);
					g = rand() % 256;
					b = rand() % 256;
				} while(!is_red(r, g, b));
			}else
			{
				do
				{
					r = rand() % 256;
					g = rand() % 256;
					b = rand() % 256;
				} while(is_red(r, g, b));
			}
		}else
		{
			r = rand() % 256;
			g = rand() % 256;
			b = rand() % 256;
		}
		fprintf(file, "%d,%d,%d,%d\n", r, g, b, is_red(r, g, b));
	}
	fclose(file);
}

int	main(int argc, char **argv)
{
	if (argc < 5)
	{
		printf("Usage: ./generator <seed> <count> <output file> <boundry 0-100> <ratio 0-100(opt)>\n");
		return (1);
	}

	char	*endptr = NULL;
	int		seed = strtol(argv[1], &endptr, 10);
	if (endptr == argv[1] || *endptr != '\0')
	{
		printf("Usage: ./generator <seed> <count> <output file> <boundry 0-100> <ratio 0-100(opt)>\n");
		return (1);
	}

	int	count = strtol(argv[2], &endptr, 10);
	if (endptr == argv[2] || *endptr != '\0')
	{
		printf("Usage: ./generator <seed> <count> <output file> <boundry 0-100> <ratio 0-100(opt)>\n");
		return (1);
	}

	int	boundry = strtol(argv[4], &endptr, 10);
	if (endptr == argv[4] || *endptr != '\0' || boundry < 0 || boundry > 100)
	{
		printf("Usage: ./generator <seed> <count> <output file> <boundry 0-100> <ratio 0-100(opt)>\n");
		return (1);
	}

	int	ratio = -1;
	if (argc == 6)
	{
		ratio = strtol(argv[5], &endptr, 10);
		if (endptr == argv[5] || *endptr != '\0' || ratio < 0 || ratio > 100)
		{
			printf("Usage: ./generator <seed> <count> <output file> <boundry 0-100> <ratio 0-100(opt)>\n");
			return (1);
		}
	}

	generat_dataset(seed, count, argv[3], ratio, boundry);
	return(0);
}
