/*
NAME        : Pavithra C R
DATE        : 22/04/2024
DESCRIPTION : LSB STEGANOGRAPHY
*/
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    // check operation type
    if (check_operation_type(argv) == e_encode) // if operation is -e (encoding)
    {
        EncodeInfo encInfo; // struct _EncodeInfo of a variable is encInfo

        printf("--------------------------------Selected Encoding--------------------------------\n"); // print a mess for selecting encoding

        if (read_and_validate_encode_args(argv, &encInfo) == e_success) // read and validate encode arg
        {
            printf("Successfully -> Read and validate encode argumnents\n");

            if (do_encoding(&encInfo) == e_success) // after checking all the CLA, start the encoding operation
            {
                printf("--------------------------------Completed Encoding--------------------------------\n");
            }
            else
            {
                printf("Failed -> to encode the message\n"); // error mess, if failed to encode
                return e_failure;
            }
        }
        else
        {
            printf("Falied -> Read and validate argumnent\n"); // error mess, if not valid CLA
            return e_failure;
        }
    }
    else if (check_operation_type(argv) == e_decode) // if operation is -d (decoding)
    {
        DecodeInfo decInfo;
        printf("--------------------------------Selected Decoding--------------------------------\n"); // print a mess for selecting decoding

        if (read_and_validate_decode_args(argv, &decInfo) == d_success) // read and validate decode arg
        {
            printf("Successfully -> Read and validate the decode argumnents\n");

            if (do_decoding(&decInfo) == d_success) // after checking all the CLA, start the decoding operation
            {
                printf("--------------------------------Completed Decoding--------------------------------\n");
            }
            else
            {
                printf("Failed -> to decode the message\n"); // error mess, if failed to decode
            }
        }
        else
        {
            printf("Falied -> Read and validate argumnent\n"); // error mess, if not valid CLA
            return d_failure;
        }
    }
    else
    {
        printf("\n##################################  Invalid option  ##################################\n\n"); // if no encoding nor decoding, print a usage mess to user to give valid CLA
        printf("--------------------------------------USAGE--------------------------------------\n");
        printf("Encoding : ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("Decoding : ./a.out -d stego.bmp output.txt\n");
        printf("---------------------------------------------------------------------------------\n");
    }

    return 0;
}

// check operation type function
OperationType check_operation_type(char *argv[])
{
    if ((strcmp(argv[1], "-e")) == 0) // checking for encoding
    {
        return e_encode;
    }
    else if ((strcmp(argv[1], "-d")) == 0) // checking for decoding
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
