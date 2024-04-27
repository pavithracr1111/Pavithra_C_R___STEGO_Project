#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

// function is to read and validate the encoding arguments passed or not
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0) // checking for .bmp file
    {
        encInfo->src_image_fname = argv[2]; // if .bmp file is passed, store it in src_image_fname
    }
    else
    {
        return e_failure; // if not passed return failure
    }
    if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0) // checking for .txt file
    {
        encInfo->secret_fname = argv[3]; // if .txt file is passed, store it in secret_fname
    }
    else
    {
        return e_failure; // if not passed return failure
    }
    if (argv[4] != NULL) // checking for stego file passed or not
    {
        encInfo->stego_image_fname = argv[4]; // if passed store it in stego_image_fname
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp"; // if not passed creat a stego.bmp file
    }
    return e_success;
}

// function to get the file size
uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END); // move the fptr to end (EOF) to get file size
    ftell(fptr_secret);              // to get the position/size
}

// checking the capacity of source image(beautiful.bmp) file to encode
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); // get the source image size and store it in image_capacity
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);           // get the seceret file size and store it in size_secret_file

    if (encInfo->image_capacity > (54 + 16 + 32 + 32 + 32 + (encInfo->size_secret_file))) // check the file size 16 - magic string,32,32,32, for secret file
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

// function to copy the header of source image to stego image
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char header[54];                                    // declare an array that holds the 54 bytes of header of source image file
    fseek(fptr_src_image, 0, SEEK_SET);                 // bring the fptr_src_image to starting position, bz to get the file size we moved it to end
    fread(header, sizeof(char), 54, fptr_src_image);    // read ->  copy the (54 bytes) content of source file
    fwrite(header, sizeof(char), 54, fptr_stego_image); // write -> the copied 54 bytes content of source file to stego file
    return e_success;
}

// function to encode the magic string and
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned char mask = 1 << 7; // mask to fetch the bit from data

    // loop until the number of bits of data
    for (int i = 0; i < 8; i++)
    {
        // fetch the bit from MSB of data and or it with image buffer
        image_buffer[i] = (image_buffer[i] & 0xFE) | (data & mask) >> (7 - i);
        mask = mask >> 1;
    }
    return e_success;
}

// function to encode the data to image
Status encode_data_to_image(const char *data, int size, EncodeInfo *encInfo)
{
    // sizeof data
    for (int i = 0; i < size; i++) // fetch 8 bytes of rgb data from source image to encode
    {
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image); // read 8 bytes from fptr_src_image and store it to image_data

        // call encode byte_to_lsb function
        encode_byte_to_lsb(data[i], encInfo->image_data);

        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image); // write the encoded data to fptr_stego_image
    }
    return e_success;
}

// function to call to encode the magic string
Status encode_magic_string(const char *magic_str, EncodeInfo *encInfo)
{
    // every encoding data should call encode_data_to_image function
    encode_data_to_image(magic_str, strlen(magic_str), encInfo);
    return e_success;
}

// function to encode the secret file extension size and
Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 1 << 31; // mask to fetch the bit from data

    // loop until the number of bits of data
    for (int i = 0; i < 32; i++)
    {
        // fetch the bit from MSB of data and or it with image buffer
        image_buffer[i] = (image_buffer[i] & 0xFE) | (size & mask) >> (31 - i);
        mask = mask >> 1;
    }
    return e_success;
}

// function to call to encode secret file extension size (int)
Status encode_size(int size, FILE *fptr_src, FILE *fptr_stego)
{
    char str[32]; // declare 32 size of string

    fread(str, 32, sizeof(char), fptr_src); // read 32 bytes from fptr_src to str

    encode_size_to_lsb(str, size); // call function to encode

    fwrite(str, 32, sizeof(char), fptr_stego); // write the encoded data to fptr_stego

    return e_success;
}

// function to call to encode secret file extension
Status encode_secret_file_extn(const char *extn, EncodeInfo *encInfo)
{
    extn = ".txt"; // in this project the type of secret file is .txt

    encode_data_to_image(extn, strlen(extn), encInfo); // call a function to encode secret file extension
    return e_success;
}

// function to call to encode the secret file size
Status encode_secret_file_size(long int file_size, EncodeInfo *encInfo)
{
    char str[32]; // declare 32 size of string

    fread(str, 32, sizeof(char), encInfo->fptr_src_image); // read 32 bytes from fptr_src to str

    encode_size_to_lsb(str, file_size); // call function to encode

    fwrite(str, 32, sizeof(char), encInfo->fptr_stego_image); // write the encoded data to fptr_stego

    return e_success;
}

// function to encode the secret data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    fseek(encInfo->fptr_secret, 0, SEEK_SET); // bring it back to starting position of secret file

    for (int i = 0; i < encInfo->size_secret_file; i++) // run a loop
    {
        fread(&ch, sizeof(char), 1, encInfo->fptr_secret);                       // read char by char from secret file and store it in ch
        
        fread(encInfo->image_data, sizeof(char), 8, encInfo->fptr_src_image);    // read 8 bytes from rgb data and store it in (image_data) to encode
        
        encode_byte_to_lsb(ch, encInfo->image_data);                             // call a function--pass char by char to encode
        
        fwrite(encInfo->image_data, sizeof(char), 8, encInfo->fptr_stego_image); // after encoding store it in stego image file
    }
    return e_success;
}

// function to copy remaining bytes from source image file to stego file
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_stego)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) > 0) // read char by char and store it in ch to write
    {
        fwrite(&ch, 1, 1, fptr_stego); // write char by char to stego file
    }
    return e_success;
}
// function for doing encoding
Status do_encoding(EncodeInfo *encInfo)
{
    // call the rest of the function in these

    if (open_files(encInfo) == e_success) // call a function to open all files
    {
        printf("Successfully -> opened all the required files\n");

        if (check_capacity(encInfo) == e_success) // call a function to check the capacity of .bmp file (source image)
        {
            printf("Check Capacity : Possible to encode the secret data\n"); // print a mess if capacity of .bmp is satisfied

            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success) // call a function to copy the header of source image to stego image
            {
                printf("Successfully -> Copied header successfully\n");

                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success) // call a function to encode the magic string
                {
                    printf("Successfully -> Magic string encoded successfully\n");

                    if (encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success) // call a function to encode the
                    {
                        printf("Successfully -> Encoded secret file extension size successfully\n");

                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success) // call a function to encode the secret file extension
                        {
                            printf("Successfully -> Encoded the secret file extension\n");

                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success) // call a function to encode the secret file size
                            {
                                printf("Successfully -> Encoded secret file size that is %ld\n", encInfo->size_secret_file);

                                if (encode_secret_file_data(encInfo) == e_success) // call a function to encode the secret data
                                {
                                    printf("Successfully -> Encoded the secret data\n");

                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success) // call a function to copy remaining data from source image file to stego file
                                    {
                                        printf("Successfully -> Copied the remaining bytes\n");
                                    }
                                    else
                                    {
                                        printf("Failed -> to copy remaining data\n"); // error mess, if Failed to copy remaining data
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed -> to encode the secret data\n"); // error mess, if Failed to encode the secret data
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed -> to encode secret file size\n"); // error mess, if
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("Failed -> to encode the secret file extension\n"); // error mess, if Failed to encode the secret file extension
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed -> to encode the secret file extension size\n"); // error mess, if Failed to encode the secret file extension size
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed -> to encode the magic string\n"); // error mess, if failed to encode the magic string
                    return e_failure;
                }
            }
            else
            {
                printf("Failed -> to copy the header\n"); // error mess, if failed to copy the header
                return e_failure;
            }
        }
        else
        {
            printf("Check Capacity : Not Possible to encode the secret data\n"); // error mess, if capacity of .bmp is not satisfied
            return e_failure;
        }
    }
    else
    {
        printf("Failed -> to open the required files\n"); // error mess if files, not opened
        return e_failure;
    }
    return e_success;
}