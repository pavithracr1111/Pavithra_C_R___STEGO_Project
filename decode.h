#ifndef DECODE_H
#define DECODE_H

#include "types.h" //contains user defined types

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *decode_stego_image_fname;
    FILE *fptr_decode_stego_image;
    char decode_image_data[8];
    char *string;

    /* Output file Info */
    char *output_fname;
    FILE *fptr_output;

    /* Secret file Info */
    int secret_file_sizes;

} DecodeInfo;

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* call a function to Decode the magic string */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode the magic string */
Status decode_data_from_image(int size, DecodeInfo *decInfo);

/* Decode a byte from LSB of image data array */
char decode_byte_from_lsb(char data,char *image_buffer);

/* call a function Decode the secret file extension size */
Status decode_secret_file_ext_size(DecodeInfo *decInfo);

/* Function to Decode the secret file extension size */
int decode_size_from_lsb(char *buffer);

/* Function to decode secret file extension */
Status decode_secret_file_ext(DecodeInfo *decInfo);

/* Function to decode the secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Function to decode the secret data */
Status decode_secret_data(int size,DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

#endif