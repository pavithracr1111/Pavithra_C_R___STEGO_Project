#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

// function to read and validate Decode args from argv
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0) // checking for .bmp file
    {
        decInfo->decode_stego_image_fname = argv[2]; // if .bmp file is passed, store it in src_image_fname
    }
    else
    {
        return d_failure; // if not passed return failure
    }

    if (argv[3] != NULL) // checking for stego file passed or not
    {
        decInfo->output_fname = argv[3]; // if passed store it in stego_image_fname
    }
    else
    {
        decInfo->output_fname = "output.txt"; // if not passed creat a stego.bmp file
    }
    return d_success;
}

// open all decode arg files
Status open_decode_files(DecodeInfo *decInfo)
{
    // Secret file
    decInfo->fptr_decode_stego_image = fopen(decInfo->decode_stego_image_fname, "r");

    // Do Error handling
    if (decInfo->fptr_decode_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->decode_stego_image_fname);

        return d_failure;
    }

    // Stego Image file
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");

    // Do Error handling
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname);

        return d_success;
    }

    // No failure return e_success
    return d_success;
}

/* Function to decode a byte from LSB of image data array */
char decode_byte_from_lsb(char data, char *image_buffer)
{

    for (int i = 0; i < 8; i++)
    {
        data = data | ((image_buffer[i] & 1) << (7 - i));
    }
    return data;
}

/* Decode the magic string */
Status decode_data_from_image(int size, DecodeInfo *decInfo)
{
    decInfo->string = (char *)calloc((size + 1), sizeof(char)); // Allocate space for string (+1 for null terminator)

    if (decInfo->string == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return d_failure;
    }

    // Read data from the file and decode it
    for (int i = 0; i < size; i++) // fetch 8 bytes of rgb data from stego image to decode
    {
        fread(decInfo->decode_image_data, 8, sizeof(char), decInfo->fptr_decode_stego_image); // read 8 bytes from fptr_decode_stego_image and store it to decode_image_data

        // call a function to decode bytes from lsb of stego image
        decInfo->string[i] = decode_byte_from_lsb(decInfo->string[i], decInfo->decode_image_data);
    }
    decInfo->string[size] = '\0';

    if (strcmp(decInfo->string, MAGIC_STRING) == 0) // check if decoded magic string is equal to magic string or not
    {
        return d_success;
    }

    if (strcmp(decInfo->string, ".txt") == 0)
    {
        return d_success;
    }

    free(decInfo->string);

    return d_failure;
}

/* call a function to Decode the magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_decode_stego_image, 54, SEEK_SET); // move the fptr of stego file to end of 54 bytes of header to fetch magic string

    decode_data_from_image(strlen(MAGIC_STRING), decInfo); // call a function to fetch 8 bytes each to get the magic string to decode and it is equal or not

    return d_success;
}

// function to decode secret file extension size
int decode_size_from_lsb(char *buffer)
{
    int size = 0;

    // loop until the number of bits of data
    for (int i = 0; i < 32; i++)
    {
        // fetch the bit from MSB of data and or it with image buffer
        size = size | (buffer[i] & 1) << (31 - i);
    }
    return size;
}

// function to decode the secret file extension size
Status decode_secret_file_ext_size(DecodeInfo *decInfo)
{

    char str[32]; // declare 32 size of string

    fread(str, 32, sizeof(char), decInfo->fptr_decode_stego_image); // read 32 bytes from fptr_src to str

    decInfo->secret_file_sizes = decode_size_from_lsb(str); // call function to encode

    printf("Secret file extension size is -> %d\n", decInfo->secret_file_sizes);

    return d_success;
}

// function to decode secret file extension
Status decode_secret_file_ext(DecodeInfo *decInfo)
{
    if (decode_data_from_image(strlen(".txt"), decInfo) == d_success)
    {
        printf("Extension of secret file is -> %s\n", decInfo->string);
        return d_success;
    }
}

// function to decode the secret file size
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char str[32]; // declare 32 size of string

    fread(str, 32, sizeof(char), decInfo->fptr_decode_stego_image); // read 32 bytes from fptr_src to str

    decInfo->secret_file_sizes = decode_size_from_lsb(str); // call function to encode

    return d_success;
}

// function to decode the secret data
Status decode_secret_data(int size,DecodeInfo *decInfo)
{
    
    char buffer[8];

    char *secret_file_data = (char *)malloc(size); // Allocate memory for secret data

    for (int i = 0; i < size; i++)
    {
        secret_file_data[i] = 0;

        fread(buffer, 8, 1, decInfo -> fptr_decode_stego_image); //fetch 8 bytes from stego image to decode 1 byte of secret data

        secret_file_data[i] = decode_byte_from_lsb(secret_file_data[i], buffer); //store char by char to secret file data string to write in output.txt
    }

    fwrite(secret_file_data, size, 1, decInfo->fptr_output); // write the fetched string to output.txt

    //free allocated memory when done
    free(secret_file_data);

    return d_success;

}

// start the decoding operation
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == d_success) // call a function to open all decode files
    {
        printf("Successfully -> Opened all the required files\n");

        if (decode_magic_string(decInfo) == d_success) // call a function to decode the magic string
        {
            printf("Successfully -> Decoded the magic string\n");

            if (decode_secret_file_ext_size(decInfo) == d_success) // call a function to decode the secret file extension size
            {
                printf("Successfully -> Decoded the secret file extension size\n");

                if (decode_secret_file_ext(decInfo) == d_success) // call a function to decode secret file extension
                {
                    printf("Successfully -> Decoded the secret file extension\n");

                    if (decode_secret_file_size(decInfo) == d_success) // call a function to decode the secret file size
                    {
                        printf("Successfully -> Decoded the secret file size\n");

                        if (decode_secret_data(decInfo->secret_file_sizes,decInfo) == d_success) // call a function to decode the secret data
                        {
                            printf("Successfully -> Decoded the secret file data\n");
                        }
                        else
                        {
                            printf("Failed -> to decode the secret data\n");
                            return d_failure;
                        }
                    }
                    else
                    {
                        printf("Failed -> to decode the secret file size\n");
                        return d_failure;
                    }
                }
                else
                {
                    printf("Failed -> to decode the secret file extension\n");
                    return d_failure;
                }
            }
            else
            {
                printf("Failed -> to decode the secret file extension size\n");
                return d_failure;
            }
        }
        else
        {
            printf("Failed -> Magic string is not available\n");
            return d_failure;
        }
    }
    else
    {
        printf("Failed -> to open all files\n");
        return d_failure;
    }

    return d_success;
}
