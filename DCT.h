#ifndef DCT_H
#define DCT_H

#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define BLOCK_SIZE 8
#define M_PI 3.14159265358979323846
 struct Image{
    int width;
    int height;
    int channels;
    unsigned char *data;
} ;

// DCT and inverse DCT functions are given, but don't forget about the coefficient
// that needs to be adjusted for the i or j equal to 0
void perform_dct(int *block);
void perform_inverse_dct(int *block);

// Functions for loading, saving, and converting image color space
struct Image load_image(const char *image_path);
void save_image(const char *image_path,  struct Image *image);
void convert_rgb_to_ycbcr(struct Image *image);
void convert_ycbcr_to_rgb(struct Image *image);

// Functions for embedding and extracting the secret data
void embed_secret_data(int *dct_coeffs, const char *secret_data);
void extract_secret_data(int *dct_coeffs, char *secret_data, unsigned int secret_data_size);

// Main function for steganography
void image_steganography_dct(const char *cover_image_path, const char *secret_message, const char *output_image_path);

#endif  // DCT_H
