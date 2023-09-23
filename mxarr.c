#include <stdio.h>
//  used for fixed-width integer types
#include <stdint.h>
#include <stdlib.h>
// string manipulation and memory operations
#include <string.h>
// Include the header file with the structures and typedefs
#include "mxarr.h"  

// Function prototype for endswap(used chatGPT to fix the error)
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


void endswap(unsigned char bits, void *input, void *output) {
    unsigned char *in = (unsigned char *)input;
    unsigned char *out = (unsigned char *)output;
//  number of bytes in the input array to be reversed
    if (input == output) {
        // In-place reversal
        for (unsigned char i = 0; i < bits / 2; i++) {
            unsigned char temp = in[i];
            in[i] = in[bits - 1 - i];
            in[bits - 1 - i] = temp;
        }
    } else {
        // Reversal with different input and output
        for (unsigned char i = 0; i < bits; i++) {
            out[i] = in[bits - 1 - i];
        }
    }
}

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

    // Calculate elno for total number of elements
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
        //free is being used from <stdlib.h> to deallocate the memory from array
        free(arr->data);
        free(arr);
    }
}


// type will hold the integer
Array* newarray(uint32_t dim0, ELEMENT_TYPES type) {
    // Allocate memory for the Array structur using malloc for the array and malloc can return a pointer to allocate memory(returns null)
    Array* arr = (Array*)malloc(sizeof(Array));
    if (arr == NULL) {
        ERROR_CODE = ERR_MEMORY;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "newarray - malloc failed\n");
        return NULL;
    }

    // Allocate memory for data based on size if it fails will return null
    arr->data = (unsigned char*)malloc(dim0 * ELEMENT_SIZE(type));
    if (arr->data == NULL) {
        ERROR_CODE = ERR_MEMORY;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "newarray - malloc failed\n");
        free(arr);
        return NULL;
    }

    // Set the components of the Array structure for a one-dimensional array
    arr->dimno = 1;//1 dimentional array
    arr->type = type;//provide elemets
    arr->dims[0] = dim0;//provide dimension
    arr->elno = dim0;//total number of elemnts to set to provide dimension

    return arr;
}

unsigned char inflate(Array *arr, uint32_t dim) {
    //It checks if the number of dimensions in the array (dimno) has already reached the maximum allowed (MAX_DIMS)
    //if it has reached to maximum it sets an error code
    if (arr->dimno >= MAX_DIMS) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "inflate - dimensionality error\n");
        return 0;
    }

    // It checks if the last dimension of the array is divisible by the new dimension 
    // If not, it sets an error code and error string and returns 0, indicating an error.
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
    // Ensure there's at least one dimension to flatten
    if (arr->dimno < 1) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "flatten - array has no dimensions\n");
        return;
    }

    // Decrease the dimensions and update elno
    arr->elno /= arr->dims[arr->dimno - 1];
    arr->dimno--;

    // Update the dimensions array
    for (unsigned char i = 0; i < arr->dimno; i++) {
        arr->dims[i] *= arr->dims[arr->dimno];
    }
}

Array* readarray(FILE *fp) {
    // read an array from the file and return the array
    // Read the 4"magic" bytes
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
            // Error
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

int writearray(FILE *fp, unsigned char bigendian, Array *arr) {
    // Determine type and dimno
    ELEMENT_TYPES type = arr->type;
    unsigned char dimno = arr->dimno;

    // Determine endianness for writing
    unsigned char magic_bytes[4];
    if (bigendian) {
        magic_bytes[0] = 0;
        magic_bytes[1] = 0;
        magic_bytes[2] = (unsigned char)type;
        magic_bytes[3] = dimno;
    } else {
        magic_bytes[0] = dimno;
        magic_bytes[1] = (unsigned char)type;
        magic_bytes[2] = 0;
        magic_bytes[3] = 0;
    }

    // Write the "magic" bytes
    if (fwrite(magic_bytes, sizeof(unsigned char), 4, fp) != 4) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "writearray - write error\n");
        return 0;
    }

    // Write dimensions
    if (fwrite(arr->dims, sizeof(uint32_t), dimno, fp) != dimno) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "writearray - write error\n");
        return 0;
    }

    // Write the data
    if (fwrite(arr->data, ELEMENT_SIZE(type), arr->elno, fp) != arr->elno) {
        ERROR_CODE = ERR_VALUE;
        snprintf(ERROR_STRING, sizeof(ERROR_STRING), "writearray - write error\n");
        return 0;
    }

    return 1;
}

void freearray(Array *arr) {
    // Free the data and the structure
    if (arr != NULL) {
        free(arr->data);
        free(arr);
    }
}
int main(int argc, char **argv ) {
    // uint32_t from stdint.h
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
            // Write the array to a file
            FILE *write_fp = fopen("array.bin", "wb");
            if (write_fp == NULL) {
                perror("Failed to open file for writing");
                destroy_array(arr);
                return 1;
            }

            int success = writearray(write_fp, 1, arr);
            fclose(write_fp);
            if (success) {
                printf("Array written to file successfully.\n");
            } else {
                printf("Failed to write array to file.\n");
            }
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
