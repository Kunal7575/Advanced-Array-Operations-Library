#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mxarr.h"  // Include the header file with the structures and typedefs

// Function prototype for endswap
/**
 * @brief Reverse the order of bytes in the input and store the result in output.
 * 
 * This function reverses the order of the bytes pointed to by input and stores them in output.
 * The number of bytes to be reversed is specified by bits.
 * 
 * @param bits The number of bytes in input to be reversed.
 * @param input Pointer to the input byte array.
 * @param output Pointer to the output byte array where the reversed bytes will be stored.
 * 
 * @note The input and output arrays can point to the same address (in-place reversal).
 * @note Two different methods of byte reversal should be implemented for efficiency when input!=output.
 */
void endswap(unsigned char bits, void *input, void *output);

// Global variables defined in mxarr.c file
ERROR_CODES ERROR_CODE = ERR_NONE;
char ERROR_STRING[256] = "";

Array* create_array(unsigned char dimno, ELEMENT_TYPES type, uint32_t* dims) {
    // Allocate memory for the Array structure
    Array* arr = (Array*)malloc(sizeof(Array));
    if (arr == NULL) {
        ERROR_CODE = ERR_MEMORY;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "Memory allocation failed.");
        return NULL;
    }

    // Set the components of the Array structure
    arr->dimno = dimno;
    arr->type = type;

    // Copy the dimensions
    for (unsigned char i = 0; i < dimno; i++) {
        arr->dims[i] = dims[i];
    }

    // Calculate elno
    arr->elno = 1;
    for (unsigned char i = 0; i < dimno; i++) {
        arr->elno *= dims[i];
    }

    // Allocate memory for the data based on the size
    arr->data = (unsigned char*)malloc(arr->elno * ELEMENT_SIZE(type));
    if (arr->data == NULL) {
        ERROR_CODE = ERR_MEMORY;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "Memory allocation for data failed.");
        free(arr);
        return NULL;
    }

    return arr;
}

void destroy_array(Array* arr) {
    // Free the allocated memory for data and the Array structure
    if (arr != NULL) {
        free(arr->data);
        free(arr);
    }
}

void endswap(unsigned char bits, void *input, void *output) {
    // TODO: Implement the byte reversal logic for endswap function
    // You can refer to the provided description to implement this function
}

Array* newarray(uint32_t dim0, ELEMENT_TYPES type) {
    // Allocate memory for the Array structure
    Array* arr = (Array*)malloc(sizeof(Array));
    if (arr == NULL) {
        ERROR_CODE = ERR_MEMORY;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "newarray - malloc failed\n");
        return NULL;
    }

    // Allocate memory for data based on size
    arr->data = (unsigned char*)malloc(dim0 * ELEMENT_SIZE(type));
    if (arr->data == NULL) {
        ERROR_CODE = ERR_MEMORY;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "newarray - malloc failed\n");
        free(arr);
        return NULL;
    }

    // Set the components of the Array structure for a one-dimensional array
    arr->dimno = 1;
    arr->type = type;
    arr->dims[0] = dim0;
    arr->elno = dim0;

    return arr;
}

unsigned char inflate(Array *arr, uint32_t dim) {
    // Check if the number of dimensions is already MAX_DIMS
    if (arr->dimno >= MAX_DIMS) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "inflate - dimensionality error\n");
        return 0;
    }

    // Check if the last dimension is divisible by dim
    if (arr->dims[arr->dimno - 1] % dim != 0) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "inflate - dimensionality error\n");
        return 0;
    }

    // Update the last dimension and add a new dimension
    arr->dims[arr->dimno - 1] = dim;
    arr->dims[arr->dimno] = arr->elno / dim;
    arr->dimno++;
    arr->elno *= (arr->dims[arr->dimno - 1]);

    return 1;
}

void flatten(Array *arr) {
    // TODO: Implement the reverse of inflate operation to flatten the array
    // You need to adjust the dimensions and elno accordingly
}

Array* readarray(FILE *fp) {
    // Read the "magic" bytes
    unsigned char magic_bytes[4];
    if (fread(magic_bytes, sizeof(unsigned char), 4, fp) != 4) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "readarray - fread error\n");
        return NULL;
    }

    // Determine endianness and extract type and dimno
    ELEMENT_TYPES type;
    unsigned char dimno;
    if (magic_bytes[0] == 0 && magic_bytes[1] == 0) {
        // Big-endian format
        type = (ELEMENT_TYPES)magic_bytes[2];
        dimno = magic_bytes[3];
    } else if (magic_bytes[2] == 0 && magic_bytes[3] == 0) {
        // Little-endian format
        type = (ELEMENT_TYPES)magic_bytes[1];
        dimno = magic_bytes[0];
    } else {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "readarray - file format violation\n");
        return NULL;
    }

    // Check for MAX_DIMS
    if (dimno > MAX_DIMS) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "readarray – dimensionality error\n");
        return NULL;
    }

    // Read dimensions and compute elno
    uint32_t dims[MAX_DIMS];
    if (fread(dims, sizeof(uint32_t), dimno, fp) != dimno) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "readarray - fread error\n");
        return NULL;
    }

    // Reverse endianness of dimensions
    for (unsigned char i = 0; i < dimno; i++) {
        endswap(sizeof(uint32_t), &dims[i], &dims[i]);
    }

    // Create the array
    Array *arr = newarray(dims[0], type);
    if (arr == NULL) {
        // Error already set by newarray
        return NULL;
    }

    // Inflate the array
    for (unsigned char i = 1; i < dimno; i++) {
        if (!inflate(arr, dims[i])) {
            // Error already set by inflate
            destroy_array(arr);
            return NULL;
        }
    }

    // Read the data
    if (fread(arr->data, ELEMENT_SIZE(type), arr->elno, fp) != arr->elno) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "readarray - fread error\n");
        destroy_array(arr);
        return NULL;
    }

    return arr;
}

int main() {
    // Example usage
    uint32_t dimensions[MAX_DIMS] = {2, 3, 4, 5};
    unsigned char num_dimensions = 4;
    ELEMENT_TYPES data_type = INT_TYPE;

    // Create an array
    Array* arr = create_array(num_dimensions, data_type, dimensions);

    if (arr != NULL) {
        printf("Array created successfully.\n");

        // Perform byte reversal using endswap
        unsigned int value_to_reverse = 0x12345678;
        unsigned int reversed_value = 0;

        endswap(sizeof(unsigned int), &value_to_reverse, &reversed_value);
        printf("Original Value: 0x%08X, Reversed Value: 0x%08X\n", value_to_reverse, reversed_value);

        // Inflate the array
        if (inflate(arr, 6)) {
            printf("Array inflated successfully.\n");
            printf("New dimensions: ");
            for (unsigned char i = 0; i < arr->dimno; i++) {
                printf("%u ", arr->dims[i]);
            }
            printf("\n");

            // Flatten the array
            flatten(arr);
            printf("Array flattened successfully.\n");
            printf("Dimensions after flattening: ");
            for (unsigned char i = 0; i < arr->dimno; i++) {
                printf("%u ", arr->dims[i]);
            }
            printf("\n");

            // Destroy the array when done
            destroy_array(arr);
            printf("Array destroyed.\n");
        } else {
            printf("Failed to inflate the array.\n");
        }
    } else {
        printf("Failed to create the array.\n");
    }

    return 0;
}
