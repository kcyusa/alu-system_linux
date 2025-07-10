#include "multithreading.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "10-blur_portion.c"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define NUM_PIXELS(img) ((img)->w * (img)->h)
#define PORTION_END_INDEX(p) (((p)->y + (p)->h) * (p)->img->w)
#define PORTION_START_INDEX(p) ((p)->y * (p)->img->w + (p)->x)
#define MAX_THREADS 16

/**
 * blur_image - Applies Gaussian Blur to the entire image
 * @img_blur: Address where the blurred image will be stored
 * @img: Original image to be blurred
 * @kernel: Convolution kernel to be used for blurring
 * Author: Frank Onyema Orji
 */
void blur_image(img_t *img_blur, img_t const *img, kernel_t const *kernel)
{
	size_t i, num_portions;
	blur_portion_t *portions;
	pthread_t *threads;

	num_portions = divide_image_into_portions(&portions, img_blur, img, kernel);

	/* Create threads */
	threads = malloc(sizeof(pthread_t) * num_portions);
	for (i = 0; i < num_portions; i++)
	{
		/* Each thread will call blur_portion_mt(&portions[i]) */
		pthread_create(&threads[i], NULL, &blur_portion_mt, &portions[i]);
	}

	/* Wait for threads to finish */
	for (i = 0; i < num_portions; i++)
	{
		pthread_join(threads[i], NULL);
	}

	/* Clean up */
	free(portions);
	free(threads);
}

/**
 * divide_image_into_portions - Splits an image into portions for processing
 * @portions: Array of portions to fill
 * @img_blur: Pointer to the blurred image
 * @img: Pointer to the original image
 * @kernel: Pointer to the convolution kernel
 * Return: Number of portions
 */
size_t divide_image_into_portions(blur_portion_t **portions, img_t *img_blur,
								  img_t const *img, kernel_t const *kernel)
{
	size_t portion_grid_size = calculate_portion_grid_size(MAX_THREADS);
	size_t num_portions = portion_grid_size * portion_grid_size;
	size_t i, x, y, w, h, remainder;

	*portions = malloc(sizeof(blur_portion_t) * num_portions);
	if (*portions == NULL)
		return 0;

	w = MAX(img->w / portion_grid_size, 1);
	for (i = 0, x = 0; x < img->w; x += w)
	{
		h = MAX(img->h / portion_grid_size, 1);
		remainder = img->w - (x + w);
		if (remainder && remainder < w)
			w += remainder;

		for (y = 0; y < img->h; y += h)
		{
			remainder = img->h - (y + h);
			if (remainder && remainder < h)
				h += remainder;

			initialize_portion(&(*portions)[i], img_blur, img, kernel, x, y, w, h);
			i++;
		}
	}

	return num_portions;
}

/**
 * calculate_portion_grid_size - Determines the grid size based on the max thread count
 * @max_threads: Maximum number of threads allowed
 * Return: Grid size for the portions
 */
size_t calculate_portion_grid_size(size_t max_threads)
{
	size_t n = 1;

	while (n * n <= max_threads)
		n++;

	return n - 1;
}

/**
 * initialize_portion - Initializes a portion of the image
 * @portion: Pointer to the portion to initialize
 * @img_blur: Pointer to the image to be blurred
 * @img: Pointer to the original image
 * @kernel: Pointer to the convolution kernel
 * @x: X-coordinate of the portion
 * @y: Y-coordinate of the portion
 * @w: Width of the portion
 * @h: Height of the portion
 */
void initialize_portion(blur_portion_t *portion, img_t *img_blur, img_t const *img,
						kernel_t const *kernel, size_t x, size_t y, size_t w, size_t h)
{
	if (portion)
	{
		portion->img = img;
		portion->img_blur = img_blur;
		portion->kernel = kernel;
		portion->x = x;
		portion->y = y;
		portion->w = w;
		portion->h = h;
	}
}

/**
 * blur_portion_mt - Wrapper for blur_portion to be used in multithreading
 * @portion: Pointer to the portion structure describing the image portion to blur
 * Return: NULL
 */
void *blur_portion_mt(void *portion)
{
	blur_portion((blur_portion_t const *)portion);
	pthread_exit(NULL);
}
