#include "multithreading.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define NUM_PIXELS(img) ((img)->w * (img)->h)
#define PORTION_START_INDEX(p) ((p)->y * (p)->img->w + (p)->x)
#define PORTION_END_INDEX(p) (((p)->y + (p)->h) * (p)->img->w)

void apply_blur_to_pixel(const blur_portion_t *portion, size_t target_index);

int is_valid_neighbor(const blur_portion_t *portion, int neighbor_index,
size_t target_index);

/**
 * blur_portion - Applies Gaussian Blur to a specific portion of an image
 * @portion: Pointer to the data structure describing the portion of the image
 * Author: Frank Onyema Orji
 */
void blur_portion(const blur_portion_t *portion)
{
	size_t row, col, i, end;

	end = MIN(PORTION_END_INDEX(portion), NUM_PIXELS(portion->img));

	for (row = PORTION_START_INDEX(portion); row < end; row += portion->img->w)
	{
		for (col = 0; col < portion->w; col++)
		{
			i = row + col;
			if (col && i % portion->img->w == 0)
				break;
			apply_blur_to_pixel(portion, i);
		}
	}
}

/**
 * apply_blur_to_pixel - Applies Gaussian Blur to a single pixel
 * @portion: Pointer to the structure describing the image portion
 * @target_index: Index of the pixel to blur
 */
void apply_blur_to_pixel(const blur_portion_t *portion, size_t target_index)
{
	float r = 0, g = 0, b = 0, sum = 0, weight;

	pixel_t *pixel;
	int neighbor_index;

	size_t i, j;

	neighbor_index = target_index - (portion->kernel->size / 2) *
	  (1 + portion->img->w);

	for (i = 0; i < portion->kernel->size; i++)
	{
		for (j = 0; j < portion->kernel->size; j++)
		{
			if (is_valid_neighbor(portion, neighbor_index + j, target_index))
			{
				pixel = &(portion->img->pixels[neighbor_index + j]);
				weight = portion->kernel->matrix[i][j];
				r += pixel->r * weight;
				g += pixel->g * weight;
				b += pixel->b * weight;
				sum += weight;
			}
		}

		neighbor_index += portion->img->w;
	}

	pixel = &(portion->img_blur->pixels[target_index]);
	pixel->r = (int)(r / sum);
	pixel->g = (int)(g / sum);
	pixel->b = (int)(b / sum);
}

/**
 * is_valid_neighbor - Checks if a given pixel index is a valid neighbor
 * @portion: Pointer to the structure describing the image portion
 * @neighbor_index: Index of the candidate neighbor pixel
 * @target_index: Index of the target pixel
 * Return: 1 if valid, 0 otherwise
 */
int is_valid_neighbor(const blur_portion_t *portion, int neighbor_index,
size_t target_index)
{
	int target_col, neighbor_col, kernel_size = (int)portion->kernel->size;
	int total_pixels = portion->img->h * portion->img->w;
	int row_width = (int)portion->img->w;

	/* Ensure neighbor_index is within valid range */
	if (neighbor_index < 0 || neighbor_index >= total_pixels)
		return (0);

	/* Handle boundary cases for target_index */
	target_col = (int)(target_index % row_width);
	neighbor_col = (int)(neighbor_index % row_width);

	if (target_col - (kernel_size / 2) < 0)
		return (neighbor_col + (kernel_size / 2) < row_width);

	if (target_col + (kernel_size / 2) >= row_width)
		return (neighbor_col - (kernel_size / 2) >= 0);

	return (1);
}
