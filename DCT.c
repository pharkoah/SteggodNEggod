#include "DCT.h"
void save_image(const char *image_path,struct Image *image) {
    printf("Saving image: %s\n", image_path);

    FILE *file = fopen(image_path, "wb");
    if (!file) {
        printf("Failed to create image file: %s\n", image_path);
        exit(1);
    }

    fwrite(&image->width, sizeof(int), 1, file);
    fwrite(&image->height, sizeof(int), 1, file);
    fwrite(&image->channels, sizeof(int), 1, file);

    unsigned int data_size = image->width * image->height * image->channels;
    if (fwrite(image->data, sizeof(unsigned char), data_size, file) != data_size) {
        printf("Failed to write image data.\n");
        fclose(file);
        exit(1);
    }

    fclose(file);
    printf("struct Image saved successfully: %s\n", image_path);
}


void convert_rgb_to_ycbcr(struct Image *image) {
    printf("Converting RGB to YCbCr...\n");
    //image = malloc(sizeof(struct Image));
    int width = image->width;
    int height = image->height; 
    printf("%d %d", width, height);
    int num_pixels = image->width * image->height;

    for (int i = 0; i < num_pixels; i++) {
        unsigned char *pixel = &image->data[i * image->channels];

        if (!pixel) {
            printf("Error: Null pixel pointer encountered.\n");
            break;
        }
        printf("%d\n",pixel[1]);
        unsigned char r = pixel[0];
        unsigned char g = pixel[1];
        unsigned char b = pixel[2];

        pixel[0] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);    // Y
        pixel[1] = (unsigned char)(128 - 0.168736 * r - 0.331264 * g + 0.5 * b);   // Cb
        pixel[2] = (unsigned char)(128 + 0.5 * r - 0.418688 * g - 0.081312 * b);   // Cr
    }

    printf("Conversion from RGB to YCbCr completed.\n");
    free(image);
}

void convert_ycbcr_to_rgb(struct Image *image) {
    printf("Converting YCbCr to RGB...\n");
    int num_pixels = image->width * image->height;

    for (int i = 0; i < num_pixels; i++) {
        unsigned char *pixel = &image->data[i * image->channels];

        if (!pixel) {
            printf("Error: Null pixel pointer encountered.\n");
            break;
        }

        unsigned char y = pixel[0];
        unsigned char cb = pixel[1];
        unsigned char cr = pixel[2];

        int r = (int)(y + 1.402 * (cr - 128));
        int g = (int)(y - 0.344136 * (cb - 128) - 0.714136 * (cr - 128));
        int b = (int)(y + 1.772 * (cb - 128));

        pixel[0] = (unsigned char)(r < 0 ? 0 : (r > 255 ? 255 : r));    // Red
        pixel[1] = (unsigned char)(g < 0 ? 0 : (g > 255 ? 255 : g));    // Green
        pixel[2] = (unsigned char)(b < 0 ? 0 : (b > 255 ? 255 : b));    // Blue
    }

    printf("Conversion from YCbCr to RGB completed.\n");
}

void perform_dct(int *block) {
    printf("Performing DCT...\n");

    int i, j, k;
    double temp[BLOCK_SIZE * BLOCK_SIZE];

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            double sum = 0.0;

            for (k = 0; k < BLOCK_SIZE; k++) {
                double cos_val = cos((2 * k + 1) * i * M_PI / (2.0 * BLOCK_SIZE));
                sum += block[k * BLOCK_SIZE + j] * cos_val;
            }

            temp[i * BLOCK_SIZE + j] = sum * sqrt(2.0 / BLOCK_SIZE);
        }
    }

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            double sum = 0.0;

            for (k = 0; k < BLOCK_SIZE; k++) {
                double cos_val = cos((2 * k + 1) * j * M_PI / (2.0 * BLOCK_SIZE));
                sum += temp[i * BLOCK_SIZE + k] * cos_val;
            }

            block[i * BLOCK_SIZE + j] = (int)(sum * sqrt(2.0 / BLOCK_SIZE));
        }
    }

    printf("DCT done.\n");
}

void perform_inverse_dct(int *block) {
    printf("Performing inverse DCT...\n");

    int i, j, k;
    double temp[BLOCK_SIZE * BLOCK_SIZE];

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            double sum = 0.0;

            for (k = 0; k < BLOCK_SIZE; k++) {
                double cos_val = cos((2 * i + 1) * k * M_PI / (2.0 * BLOCK_SIZE));
                sum += block[i * BLOCK_SIZE + k] * cos_val;
            }

            temp[i * BLOCK_SIZE + j] = sum * sqrt(2.0 / BLOCK_SIZE);
        }
    }

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            double sum = 0.0;

            for (k = 0; k < BLOCK_SIZE; k++) {
                double cos_val = cos((2 * j + 1) * k * M_PI / (2.0 * BLOCK_SIZE));
                sum += temp[k * BLOCK_SIZE + j] * cos_val;
            }

            block[i * BLOCK_SIZE + j] = (int)(sum * sqrt(2.0 / BLOCK_SIZE));
        }
    }

    printf("Inverse DCT done.\n");
}

void embed_secret_data(int *dct_coeffs, const char *secret_data) {
    printf("Embedding secret data...\n");

    unsigned int index = 0;
    unsigned int bit_index = 0;

    while (secret_data[index] != '\0') {
        unsigned char secret_byte = secret_data[index];
        for (bit_index = 0; bit_index < 8; bit_index++) {
            int coeff = dct_coeffs[index * 8 + bit_index];
            int bit = (secret_byte >> (7 - bit_index)) & 1;
            if (bit == 0 && coeff % 2 != 0) {
                coeff--;
            } else if (bit == 1 && coeff % 2 == 0) {
                coeff++;
            }
            dct_coeffs[index * 8 + bit_index] = coeff;
        }
        index++;
    }

    printf("Secret data embedded.\n");
}

struct Image load_image(const char *image_path) {
    printf("Loading image: %s\n", image_path);

    FILE *file = fopen(image_path, "rb");
    if (!file) {
        printf("Failed to open image file: %s\n", image_path);
        exit(1);
    }

    fread(&image->width, sizeof(int), 1, file);
    fread(&image->height, sizeof(int), 1, file);
    fread(&image->channels, sizeof(int), 1, file);

    unsigned int data_size = image->width * image->height * image->channels;
    image->data = (unsigned char *)malloc(data_size);
    fread(image->data, sizeof(unsigned char), data_size, file);

    fclose(file);
    printf("struct Image loaded successfully.\n");
    return image;
}

void image_steganography_dct(const char *cover_image_path, const char *secret_message, const char *output_image_path) {
    printf("Performing image steganography with DCT...\n");

    struct Image *image;

    // Load cover image
    image = load_image(cover_image_path, &image);

    // Convert cover image to YCbCr color space
    convert_rgb_to_ycbcr(&image);

    // Calculate the number of blocks in the cover image
    int num_blocks = (image.width * image.height) / (BLOCK_SIZE * BLOCK_SIZE);

    // Embed the secret message into the cover image
    for (int block = 0; block < num_blocks; block++) {
        int dct_coeffs[BLOCK_SIZE * BLOCK_SIZE];

        // Extract the current block from the cover image
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                int y = image.data[((block / (image.width / BLOCK_SIZE)) * BLOCK_SIZE + i) * image.width + ((block % (image.width / BLOCK_SIZE)) * BLOCK_SIZE + j)];
                dct_coeffs[i * BLOCK_SIZE + j] = y;
            }
        }

        // Perform DCT on the block
        perform_dct(dct_coeffs);

        // Embed the secret message into the DCT coefficients
        embed_secret_data(dct_coeffs, secret_message);

        // Perform inverse DCT on the modified block
        perform_inverse_dct(dct_coeffs);

        // Update the cover image with the modified block
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                image.data[((block / (image.width / BLOCK_SIZE)) * BLOCK_SIZE + i) * image.width + ((block % (image.width / BLOCK_SIZE)) * BLOCK_SIZE + j)] = dct_coeffs[i * BLOCK_SIZE + j];
            }
        }
    }

    // Convert the cover image back to RGB color space
    convert_ycbcr_to_rgb(&image);

    // Save the modified image to the output file
    save_image(output_image_path, &image);

    // Clean up resources
    free(image.data);

    printf("struct Image steganography with DCT completed.\n");
}

int main() {
    char cover_image_path[256];
    char secret_message[1024];

    printf("Enter the path of the cover image: ");
    fgets(cover_image_path, sizeof(cover_image_path), stdin);
    cover_image_path[strcspn(cover_image_path, "\n")] = 0;  // Remove trailing newline from fgets()

    printf("Enter the secret message to embed: ");
    fgets(secret_message, sizeof(secret_message), stdin);
    secret_message[strcspn(secret_message, "\n")] = 0;  // Remove trailing newline from fgets()

    const char *output_image_path = "output_image.jpg";

    image_steganography_dct(cover_image_path, secret_message, output_image_path);

    printf("Secret message embedded successfully!\n");

    return 0;
}
