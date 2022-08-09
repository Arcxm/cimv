#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <getopt.h>

#include <Windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#define CIMV_VERSION "0.1"
#define DEFAULT_BLOCK ((char) 219)

/**
 * @brief Prints the version
 * 
 * @see CIMV_VERSION
 */
void print_version() {
    fprintf(stdout, "cimv %s, an image viewer for the terminal.\n", CIMV_VERSION);
}

/**
 * @brief Prints the help
 */
void print_help() {
    fprintf(stdout, "usage: cimv [options] image\n");
    fprintf(stdout, "  options:\n");
    fprintf(stdout, "    -h            Print this help and exit\n");
    fprintf(stdout, "    -v            Print the version and exit\n");
}

/**
 * @brief Get the size of the terminal.
 * 
 * @param width Pointer to an int to which the width of the terminal is assigned
 * @param height Pointer to an int to which the height of the terminal is assigned
 */
bool get_term_size(int *width, int *height) {
    // TODO: Also support other operating systems like linux

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hStdOut != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        GetConsoleScreenBufferInfo(hStdOut, &csbi);
        *width = csbi.dwSize.X;
        *height = csbi.dwSize.Y;

        return true;
    } else {
        return false;
    }
}

/**
 * @brief Prints the image to stdout.
 * 
 * @param image A stbi_uc pointer to the raw image data
 * @param width The width of the image
 * @param height The height of the image
 * @param comp The components e.g. STBI_rgb
 * @param block The character to print
 */
void print_image(const stbi_uc *image, int width, int height, int comp, char block) {
    // TODO: Can this be made more efficient?

    if (!image)
        return;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            stbi_uc r = image[((y * width) + x) * comp];
            stbi_uc g = image[((y * width) + x) * comp + 1];
            stbi_uc b = image[((y * width) + x) * comp + 2];

            fprintf(stdout, "\x1B[38;2;%d;%d;%dm%c%c\x1B[0m", r, g, b, block, block);
        }

        fprintf(stdout, "\n");
    }
}

/**
 * @brief The entry point.
 * 
 * @param argc The count of arguments
 * @param argv The arguments
 */
int main(int argc, char **argv) {
    // Set defaults
    bool help = false;
    bool version = false;

    // Process the arguments
    int c;
    while ((c = getopt(argc, argv, "vh")) != -1) {
        switch (c) {
            case 'h':
                help = true;
                break;
            case 'v':
                version = true;
                break;
        }
    }

    // Print help and/or version if the corresponding flags have been set
    if (help) {
        print_version();
        print_help();

        return EXIT_SUCCESS;
    } else if (version) {
        print_version();

        return EXIT_SUCCESS;
    }

    // Get the image file name
    if (optind < argc) {
        const char *const file_name = argv[optind++];

        // Load the image
        int image_width = 0;
        int image_height = 0;
        int image_comp = 0;
        
        stbi_uc *data = stbi_load(file_name, &image_width, &image_height, &image_comp, STBI_rgb);
        if (data) {
            // Get the size of the terminal
            int term_width = 0;
            int term_height = 0;

            if (get_term_size(&term_width, &term_height)) {
                // Calculate ratio for the resize operation
                double ratio_x = (double) term_width / image_width;
                double ratio_y = (double) term_height / image_height;
                double ratio = min(ratio_x, ratio_y);

                // Calculate resized width and height
                int out_width = image_width * ratio;
                int out_height = image_height * ratio;

                stbi_uc *resized_data = (stbi_uc*) malloc(sizeof(stbi_uc) * (out_width * out_height * image_comp));
                if (resized_data) {
                    // Resize the image
                    //INSPECT OTHER RESIZE FUNCTIONS, FOR ALPHA CHANNEL SUPPORT
                    if (stbir_resize_uint8(data, image_width, image_height, 0, resized_data, out_width, out_height, out_width * image_comp, image_comp)) {
                        // From here on the original image is no longer needed
                        stbi_image_free(data);
                        data = NULL;

                        // Print the image to the terminal
                        print_image(resized_data, out_width, out_height, image_comp, DEFAULT_BLOCK);

                        // Free the resized image
                        free(resized_data);
                        resized_data = NULL;

                        // Exit
                        return EXIT_SUCCESS;
                    } else {
                        fprintf(stderr, "cimv: could not resize the image\n");

                        stbi_image_free(data);
                        return EXIT_SUCCESS;
                    }
                } else {
                    fprintf(stderr, "cimv: could not allocate memory for the resizing operation\n");

                    stbi_image_free(data);
                    return EXIT_SUCCESS;
                }
            } else {
                fprintf(stderr, "cimv: could not determine terminal size\n");

                stbi_image_free(data);
                return EXIT_SUCCESS;
            }
        } else {
            fprintf(stderr, "cimv: could not find or open '%s'\n", file_name);

            return EXIT_FAILURE;
        }
    } else {
        // If none was supplied, print an error and the help
        fprintf(stdout, "cimv: missing image file name\n");
        print_help();

        return EXIT_FAILURE;
    }
}