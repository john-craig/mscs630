#ifdef TEST
#include <stdio.h>              /* used for debugging */
#include <string.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <openssl/ssl.h>
#include "../src/maru/maru_types.h"
//#include "utils/assert.h"

#include "aes.h"

/*****************************************************/

void printHex(m_u8 **hexMatrix){
    m_u8 *curString;

    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            m_u8 c1 = hexMatrix[j][i] / 16;
            m_u8 c2 = hexMatrix[j][i] % 16;

            switch(c1){
                case 0:
                    c1 = '0';
                    break;
                case 1:
                    c1 = '1';
                    break;
                case 2:
                    c1 = '2';
                    break;
                case 3:
                    c1 = '3';
                    break;
                case 4:
                    c1 = '4';
                    break;
                case 5:
                    c1 = '5';
                    break;
                case 6:
                    c1 = '6';
                    break;
                case 7:
                    c1 = '7';
                    break;
                case 8:
                    c1 = '8';
                    break;
                case 9:
                    c1 = '9';
                    break;
                case 10:
                    c1 = 'A';
                    break;
                case 11:
                    c1 = 'B';
                    break;
                case 12:
                    c1 = 'C';
                    break;
                case 13:
                    c1 = 'D';
                    break;
                case 14:
                    c1 = 'E';
                    break;
                case 15:
                    c1 = 'F';
                    break;
            }

            switch(c2){
                case 0:
                    c2 = '0';
                    break;
                case 1:
                    c2 = '1';
                    break;
                case 2:
                    c2 = '2';
                    break;
                case 3:
                    c2 = '3';
                    break;
                case 4:
                    c2 = '4';
                    break;
                case 5:
                    c2 = '5';
                    break;
                case 6:
                    c2 = '6';
                    break;
                case 7:
                    c2 = '7';
                    break;
                case 8:
                    c2 = '8';
                    break;
                case 9:
                    c2 = '9';
                    break;
                case 10:
                    c2 = 'A';
                    break;
                case 11:
                    c2 = 'B';
                    break;
                case 12:
                    c2 = 'C';
                    break;
                case 13:
                    c2 = 'D';
                    break;
                case 14:
                    c2 = 'E';
                    break;
                case 15:
                    c2 = 'F';
                    break;
            }

            printf("%c",c1);
            printf("%c",c2);
        }
    }

    printf("\n");
}

void free2DArray(m_u8 **arr, int len){
    // for(int i=0;i<len;i++){
    //     free(arr[i]);
    // }

    free(arr);
}

static m_u8 aesBox(m_u8 inHex){
    m_u8 row = inHex / 16;
    m_u8 col = inHex % 16;

    m_u8 sBox[16][16] = {
        {0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76},
        {0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0},
        {0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15},
        {0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75},
        {0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84},
        {0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF},
        {0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8},
        {0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2},
        {0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73},
        {0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB},
        {0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79},
        {0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08},
        {0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A},
        {0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E},
        {0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF},
        {0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16}

    };

    return sBox[row][col];
}

static m_u8 aesInvBox(m_u8 inHex){
    m_u8 row = inHex / 16;
    m_u8 col = inHex % 16;

    m_u8 invSBox[16][16] = {
        {0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB},
        {0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB},
        {0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E},
        {0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25},
        {0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92},
        {0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84},
        {0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06},
        {0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B},
        {0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73},
        {0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E},
        {0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B},
        {0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4},
        {0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F},
        {0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF},
        {0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61},
        {0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D}
    };

    return invSBox[row][col];
}

// Decryption: Multiply by 9 for InverseMixColumns
static m_u8 mul9[256] =
{
	0x00,0x09,0x12,0x1b,0x24,0x2d,0x36,0x3f,0x48,0x41,0x5a,0x53,0x6c,0x65,0x7e,0x77,
	0x90,0x99,0x82,0x8b,0xb4,0xbd,0xa6,0xaf,0xd8,0xd1,0xca,0xc3,0xfc,0xf5,0xee,0xe7,
	0x3b,0x32,0x29,0x20,0x1f,0x16,0x0d,0x04,0x73,0x7a,0x61,0x68,0x57,0x5e,0x45,0x4c,
	0xab,0xa2,0xb9,0xb0,0x8f,0x86,0x9d,0x94,0xe3,0xea,0xf1,0xf8,0xc7,0xce,0xd5,0xdc,
	0x76,0x7f,0x64,0x6d,0x52,0x5b,0x40,0x49,0x3e,0x37,0x2c,0x25,0x1a,0x13,0x08,0x01,
	0xe6,0xef,0xf4,0xfd,0xc2,0xcb,0xd0,0xd9,0xae,0xa7,0xbc,0xb5,0x8a,0x83,0x98,0x91,
	0x4d,0x44,0x5f,0x56,0x69,0x60,0x7b,0x72,0x05,0x0c,0x17,0x1e,0x21,0x28,0x33,0x3a,
	0xdd,0xd4,0xcf,0xc6,0xf9,0xf0,0xeb,0xe2,0x95,0x9c,0x87,0x8e,0xb1,0xb8,0xa3,0xaa,
	0xec,0xe5,0xfe,0xf7,0xc8,0xc1,0xda,0xd3,0xa4,0xad,0xb6,0xbf,0x80,0x89,0x92,0x9b,
	0x7c,0x75,0x6e,0x67,0x58,0x51,0x4a,0x43,0x34,0x3d,0x26,0x2f,0x10,0x19,0x02,0x0b,
	0xd7,0xde,0xc5,0xcc,0xf3,0xfa,0xe1,0xe8,0x9f,0x96,0x8d,0x84,0xbb,0xb2,0xa9,0xa0,
	0x47,0x4e,0x55,0x5c,0x63,0x6a,0x71,0x78,0x0f,0x06,0x1d,0x14,0x2b,0x22,0x39,0x30,
	0x9a,0x93,0x88,0x81,0xbe,0xb7,0xac,0xa5,0xd2,0xdb,0xc0,0xc9,0xf6,0xff,0xe4,0xed,
	0x0a,0x03,0x18,0x11,0x2e,0x27,0x3c,0x35,0x42,0x4b,0x50,0x59,0x66,0x6f,0x74,0x7d,
	0xa1,0xa8,0xb3,0xba,0x85,0x8c,0x97,0x9e,0xe9,0xe0,0xfb,0xf2,0xcd,0xc4,0xdf,0xd6,
	0x31,0x38,0x23,0x2a,0x15,0x1c,0x07,0x0e,0x79,0x70,0x6b,0x62,0x5d,0x54,0x4f,0x46
};

// Decryption: Multiply by 11 for InverseMixColumns
static m_u8 mul11[256] =
{
	0x00,0x0b,0x16,0x1d,0x2c,0x27,0x3a,0x31,0x58,0x53,0x4e,0x45,0x74,0x7f,0x62,0x69,
	0xb0,0xbb,0xa6,0xad,0x9c,0x97,0x8a,0x81,0xe8,0xe3,0xfe,0xf5,0xc4,0xcf,0xd2,0xd9,
	0x7b,0x70,0x6d,0x66,0x57,0x5c,0x41,0x4a,0x23,0x28,0x35,0x3e,0x0f,0x04,0x19,0x12,
	0xcb,0xc0,0xdd,0xd6,0xe7,0xec,0xf1,0xfa,0x93,0x98,0x85,0x8e,0xbf,0xb4,0xa9,0xa2,
	0xf6,0xfd,0xe0,0xeb,0xda,0xd1,0xcc,0xc7,0xae,0xa5,0xb8,0xb3,0x82,0x89,0x94,0x9f,
	0x46,0x4d,0x50,0x5b,0x6a,0x61,0x7c,0x77,0x1e,0x15,0x08,0x03,0x32,0x39,0x24,0x2f,
	0x8d,0x86,0x9b,0x90,0xa1,0xaa,0xb7,0xbc,0xd5,0xde,0xc3,0xc8,0xf9,0xf2,0xef,0xe4,
	0x3d,0x36,0x2b,0x20,0x11,0x1a,0x07,0x0c,0x65,0x6e,0x73,0x78,0x49,0x42,0x5f,0x54,
	0xf7,0xfc,0xe1,0xea,0xdb,0xd0,0xcd,0xc6,0xaf,0xa4,0xb9,0xb2,0x83,0x88,0x95,0x9e,
	0x47,0x4c,0x51,0x5a,0x6b,0x60,0x7d,0x76,0x1f,0x14,0x09,0x02,0x33,0x38,0x25,0x2e,
	0x8c,0x87,0x9a,0x91,0xa0,0xab,0xb6,0xbd,0xd4,0xdf,0xc2,0xc9,0xf8,0xf3,0xee,0xe5,
	0x3c,0x37,0x2a,0x21,0x10,0x1b,0x06,0x0d,0x64,0x6f,0x72,0x79,0x48,0x43,0x5e,0x55,
	0x01,0x0a,0x17,0x1c,0x2d,0x26,0x3b,0x30,0x59,0x52,0x4f,0x44,0x75,0x7e,0x63,0x68,
	0xb1,0xba,0xa7,0xac,0x9d,0x96,0x8b,0x80,0xe9,0xe2,0xff,0xf4,0xc5,0xce,0xd3,0xd8,
	0x7a,0x71,0x6c,0x67,0x56,0x5d,0x40,0x4b,0x22,0x29,0x34,0x3f,0x0e,0x05,0x18,0x13,
	0xca,0xc1,0xdc,0xd7,0xe6,0xed,0xf0,0xfb,0x92,0x99,0x84,0x8f,0xbe,0xb5,0xa8,0xa3
};

// Decryption: Multiply by 13 for InverseMixColumns
static m_u8 mul13[256] =
{
	0x00,0x0d,0x1a,0x17,0x34,0x39,0x2e,0x23,0x68,0x65,0x72,0x7f,0x5c,0x51,0x46,0x4b,
	0xd0,0xdd,0xca,0xc7,0xe4,0xe9,0xfe,0xf3,0xb8,0xb5,0xa2,0xaf,0x8c,0x81,0x96,0x9b,
	0xbb,0xb6,0xa1,0xac,0x8f,0x82,0x95,0x98,0xd3,0xde,0xc9,0xc4,0xe7,0xea,0xfd,0xf0,
	0x6b,0x66,0x71,0x7c,0x5f,0x52,0x45,0x48,0x03,0x0e,0x19,0x14,0x37,0x3a,0x2d,0x20,
	0x6d,0x60,0x77,0x7a,0x59,0x54,0x43,0x4e,0x05,0x08,0x1f,0x12,0x31,0x3c,0x2b,0x26,
	0xbd,0xb0,0xa7,0xaa,0x89,0x84,0x93,0x9e,0xd5,0xd8,0xcf,0xc2,0xe1,0xec,0xfb,0xf6,
	0xd6,0xdb,0xcc,0xc1,0xe2,0xef,0xf8,0xf5,0xbe,0xb3,0xa4,0xa9,0x8a,0x87,0x90,0x9d,
	0x06,0x0b,0x1c,0x11,0x32,0x3f,0x28,0x25,0x6e,0x63,0x74,0x79,0x5a,0x57,0x40,0x4d,
	0xda,0xd7,0xc0,0xcd,0xee,0xe3,0xf4,0xf9,0xb2,0xbf,0xa8,0xa5,0x86,0x8b,0x9c,0x91,
	0x0a,0x07,0x10,0x1d,0x3e,0x33,0x24,0x29,0x62,0x6f,0x78,0x75,0x56,0x5b,0x4c,0x41,
	0x61,0x6c,0x7b,0x76,0x55,0x58,0x4f,0x42,0x09,0x04,0x13,0x1e,0x3d,0x30,0x27,0x2a,
	0xb1,0xbc,0xab,0xa6,0x85,0x88,0x9f,0x92,0xd9,0xd4,0xc3,0xce,0xed,0xe0,0xf7,0xfa,
	0xb7,0xba,0xad,0xa0,0x83,0x8e,0x99,0x94,0xdf,0xd2,0xc5,0xc8,0xeb,0xe6,0xf1,0xfc,
	0x67,0x6a,0x7d,0x70,0x53,0x5e,0x49,0x44,0x0f,0x02,0x15,0x18,0x3b,0x36,0x21,0x2c,
	0x0c,0x01,0x16,0x1b,0x38,0x35,0x22,0x2f,0x64,0x69,0x7e,0x73,0x50,0x5d,0x4a,0x47,
	0xdc,0xd1,0xc6,0xcb,0xe8,0xe5,0xf2,0xff,0xb4,0xb9,0xae,0xa3,0x80,0x8d,0x9a,0x97
};

// Decryption: Multiply by 14 for InverseMixColumns
static m_u8 mul14[256] =
{
	0x00,0x0e,0x1c,0x12,0x38,0x36,0x24,0x2a,0x70,0x7e,0x6c,0x62,0x48,0x46,0x54,0x5a,
	0xe0,0xee,0xfc,0xf2,0xd8,0xd6,0xc4,0xca,0x90,0x9e,0x8c,0x82,0xa8,0xa6,0xb4,0xba,
	0xdb,0xd5,0xc7,0xc9,0xe3,0xed,0xff,0xf1,0xab,0xa5,0xb7,0xb9,0x93,0x9d,0x8f,0x81,
	0x3b,0x35,0x27,0x29,0x03,0x0d,0x1f,0x11,0x4b,0x45,0x57,0x59,0x73,0x7d,0x6f,0x61,
	0xad,0xa3,0xb1,0xbf,0x95,0x9b,0x89,0x87,0xdd,0xd3,0xc1,0xcf,0xe5,0xeb,0xf9,0xf7,
	0x4d,0x43,0x51,0x5f,0x75,0x7b,0x69,0x67,0x3d,0x33,0x21,0x2f,0x05,0x0b,0x19,0x17,
	0x76,0x78,0x6a,0x64,0x4e,0x40,0x52,0x5c,0x06,0x08,0x1a,0x14,0x3e,0x30,0x22,0x2c,
	0x96,0x98,0x8a,0x84,0xae,0xa0,0xb2,0xbc,0xe6,0xe8,0xfa,0xf4,0xde,0xd0,0xc2,0xcc,
	0x41,0x4f,0x5d,0x53,0x79,0x77,0x65,0x6b,0x31,0x3f,0x2d,0x23,0x09,0x07,0x15,0x1b,
	0xa1,0xaf,0xbd,0xb3,0x99,0x97,0x85,0x8b,0xd1,0xdf,0xcd,0xc3,0xe9,0xe7,0xf5,0xfb,
	0x9a,0x94,0x86,0x88,0xa2,0xac,0xbe,0xb0,0xea,0xe4,0xf6,0xf8,0xd2,0xdc,0xce,0xc0,
	0x7a,0x74,0x66,0x68,0x42,0x4c,0x5e,0x50,0x0a,0x04,0x16,0x18,0x32,0x3c,0x2e,0x20,
	0xec,0xe2,0xf0,0xfe,0xd4,0xda,0xc8,0xc6,0x9c,0x92,0x80,0x8e,0xa4,0xaa,0xb8,0xb6,
	0x0c,0x02,0x10,0x1e,0x34,0x3a,0x28,0x26,0x7c,0x72,0x60,0x6e,0x44,0x4a,0x58,0x56,
	0x37,0x39,0x2b,0x25,0x0f,0x01,0x13,0x1d,0x47,0x49,0x5b,0x55,0x7f,0x71,0x63,0x6d,
	0xd7,0xd9,0xcb,0xc5,0xef,0xe1,0xf3,0xfd,0xa7,0xa9,0xbb,0xb5,0x9f,0x91,0x83,0x8d
};

/**********************************************************************************/

static m_u8 aesRcon(int round){
    m_u8 rConstants[256] = {
        0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A,
        0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39,
        0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A,
        0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8,
        0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF,
        0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC,
        0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B,
        0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3,
        0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94,
        0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
        0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35,
        0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F,
        0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04,
        0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63,
        0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD,
        0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D
    };

    return rConstants[round];
}

// static m_u8 **AESRoundKeysMatrix(m_u8 **keyMatrix){
//     m_u8 **workMatrix = (m_u8 **) malloc(sizeof (m_u8 *) * 4);

//     for(int i=0;i<4;i++){
//         workMatrix[i] = (m_u8 *) malloc(sizeof (m_u8) * 44);
//     }

//     //Round zero
//     for(int k=0;k<4;k++){
//         workMatrix[k][0] = keyMatrix[k][0];
//         workMatrix[k][1] = keyMatrix[k][1];
//         workMatrix[k][2] = keyMatrix[k][2];
//         workMatrix[k][3] = keyMatrix[k][3];
//     }

//     m_u8 *tempMatrix = (m_u8 *) malloc(sizeof (m_u8) * 4);
//     int round;

//     //Remaining rounds
//     for(int j=4;j<44;j++){
//         if(j % 4 != 0){
//             workMatrix[0][j] = workMatrix[0][j-4] ^ workMatrix[0][j-1];
//             workMatrix[1][j] = workMatrix[1][j-4] ^ workMatrix[1][j-1];
//             workMatrix[2][j] = workMatrix[2][j-4] ^ workMatrix[2][j-1];
//             workMatrix[3][j] = workMatrix[3][j-4] ^ workMatrix[3][j-1];
//         } else {
//             round = j / 4;

//             //Shift previous matrix column by one
//             tempMatrix[0] = workMatrix[1][j-1];
//             tempMatrix[1] = workMatrix[2][j-1];
//             tempMatrix[2] = workMatrix[3][j-1];
//             tempMatrix[3] = workMatrix[0][j-1];

//             //Apply sBox transformation and round constant modulation
//             tempMatrix[0] = aesBox(tempMatrix[0]) ^ aesRcon(round);
//             tempMatrix[1] = aesBox(tempMatrix[1]); //^ aesRcon(round);
//             tempMatrix[2] = aesBox(tempMatrix[2]); //^ aesRcon(round);
//             tempMatrix[3] = aesBox(tempMatrix[3]); //^ aesRcon(round);

//             workMatrix[0][j] = tempMatrix[0] ^ workMatrix[0][j-4];
//             workMatrix[1][j] = tempMatrix[1] ^ workMatrix[1][j-4];
//             workMatrix[2][j] = tempMatrix[2] ^ workMatrix[2][j-4];
//             workMatrix[3][j] = tempMatrix[3] ^ workMatrix[3][j-4];
//         }
//     }

//     free(tempMatrix);

//     return workMatrix;
// }

static m_u8 **AESRoundKeysMatrix(m_u8 **keyMatrix, m_u8 keyRoundMatrix[4][44]){
    //Round zero
    for(int k=0;k<4;k++){
        keyRoundMatrix[k][0] = keyMatrix[k][0];
        keyRoundMatrix[k][1] = keyMatrix[k][1];
        keyRoundMatrix[k][2] = keyMatrix[k][2];
        keyRoundMatrix[k][3] = keyMatrix[k][3];
    }

    m_u8 *tempMatrix = (m_u8 *) malloc(sizeof (m_u8) * 4);
    int round;

    //Remaining rounds
    for(int j=4;j<44;j++){
        if(j % 4 != 0){
            keyRoundMatrix[0][j] = keyRoundMatrix[0][j-4] ^ keyRoundMatrix[0][j-1];
            keyRoundMatrix[1][j] = keyRoundMatrix[1][j-4] ^ keyRoundMatrix[1][j-1];
            keyRoundMatrix[2][j] = keyRoundMatrix[2][j-4] ^ keyRoundMatrix[2][j-1];
            keyRoundMatrix[3][j] = keyRoundMatrix[3][j-4] ^ keyRoundMatrix[3][j-1];
        } else {
            round = j / 4;

            //Shift previous matrix column by one
            tempMatrix[0] = keyRoundMatrix[1][j-1];
            tempMatrix[1] = keyRoundMatrix[2][j-1];
            tempMatrix[2] = keyRoundMatrix[3][j-1];
            tempMatrix[3] = keyRoundMatrix[0][j-1];

            //Apply sBox transformation and round constant modulation
            tempMatrix[0] = aesBox(tempMatrix[0]) ^ aesRcon(round);
            tempMatrix[1] = aesBox(tempMatrix[1]); //^ aesRcon(round);
            tempMatrix[2] = aesBox(tempMatrix[2]); //^ aesRcon(round);
            tempMatrix[3] = aesBox(tempMatrix[3]); //^ aesRcon(round);

            keyRoundMatrix[0][j] = tempMatrix[0] ^ keyRoundMatrix[0][j-4];
            keyRoundMatrix[1][j] = tempMatrix[1] ^ keyRoundMatrix[1][j-4];
            keyRoundMatrix[2][j] = tempMatrix[2] ^ keyRoundMatrix[2][j-4];
            keyRoundMatrix[3][j] = tempMatrix[3] ^ keyRoundMatrix[3][j-4];
        }
    }

    free(tempMatrix);
}

static m_u8 **AESGetRoundKey(m_u8 AESRoundKeysMatrix[4][44], int round){
    m_u8 **keyMatrix = (m_u8 **) malloc(sizeof (m_u8 *) * 4);

    for(int i=0;i<4;i++){
        keyMatrix[i] = (m_u8 *) malloc(sizeof (m_u8) * 44);
    }

    int startIndex = (round - 1) * 4;

    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            keyMatrix[i][j] = AESRoundKeysMatrix[i][j+startIndex];
        }
    }

    return keyMatrix;
}

static m_u8 **AESStateXOR(m_u8 **sHex, m_u8 **keyHex, int rows, int cols){
    m_u8 **outStateHex = (m_u8 **) malloc(sizeof (m_u8 *) * rows);

    for(int i=0;i<rows;i++){
        outStateHex[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            outStateHex[i][j] = sHex[i][j] ^ keyHex[i][j];
        }
    }

    free(sHex);

    return outStateHex;
}

static m_u8 **AESNibbleSub(m_u8 **inStateHex, int rows, int cols){
    m_u8 **outStateHex = (m_u8 **) malloc(sizeof (m_u8 *) * rows);

    for(int i=0;i<rows;i++){
        outStateHex[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            outStateHex[i][j] = aesBox(inStateHex[i][j]);
        }
    }

    free2DArray(inStateHex, rows);

    return outStateHex;
}

static m_u8 **AESShiftRow(m_u8 **inStateHex, int rows, int cols){
    m_u8 **outStateHex = (m_u8 **) malloc(sizeof (m_u8 *) * rows);

    for(int i=0;i<rows;i++){
        outStateHex[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            if(j+i >= 4){
                outStateHex[i][j] = inStateHex[i][j+i-4];
            } else {
                outStateHex[i][j] = inStateHex[i][j+i];
            }
        }
    }

    free2DArray(inStateHex,rows);

    return outStateHex;
}

static m_u8 **AESMixColumn(m_u8 **inStateHex, int rows, int cols){
    m_u8 **outStateHex = (m_u8 **) malloc(sizeof (m_u8 *) * rows);
    m_u8 **workMatrix = (m_u8 **) malloc(sizeof (m_u8 *) * 2);

    for(int i=0;i<rows;i++){
        outStateHex[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<2;i++){
        workMatrix[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<rows;i++){
        m_u8 c, d;

        for (int j = 0; j < 4; j++) {
            d = inStateHex[j][i];

            workMatrix[0][j] = d;
            
            c = ((d >> 7) & 1);
            
            workMatrix[1][j] = (d << 1);
            workMatrix[1][j] ^= c * 0x1B;
        }

        outStateHex[0][i] = workMatrix[1][0] ^ workMatrix[0][3] ^ workMatrix[0][2] ^ workMatrix[1][1] ^ workMatrix[0][1];
        outStateHex[1][i] = workMatrix[1][1] ^ workMatrix[0][0] ^ workMatrix[0][3] ^ workMatrix[1][2] ^ workMatrix[0][2];
        outStateHex[2][i] = workMatrix[1][2] ^ workMatrix[0][1] ^ workMatrix[0][0] ^ workMatrix[1][3] ^ workMatrix[0][3];
        outStateHex[3][i] = workMatrix[1][3] ^ workMatrix[0][2] ^ workMatrix[0][1] ^ workMatrix[1][0] ^ workMatrix[0][0];
    }

    free2DArray(workMatrix,2);
    free2DArray(inStateHex,rows);

    return outStateHex;
}

/*****************************************************************************/


static m_u8 **AESNibbleUnsub(m_u8 **inStateHex, int rows, int cols){
    m_u8 **outStateHex = (m_u8 **) malloc(sizeof (m_u8 *) * rows);

    for(int i=0;i<rows;i++){
        outStateHex[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            outStateHex[i][j] = aesInvBox(inStateHex[i][j]);
        }
    }

    free2DArray(inStateHex,rows);

    return outStateHex;
}

static m_u8 **AESUnshiftRow(m_u8 **inStateHex, int rows, int cols){
    m_u8 **outStateHex = (m_u8 **) malloc(sizeof (m_u8 *) * rows);

    for(int i=0;i<rows;i++){
        outStateHex[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            //i is the number of elements to unshift by
            // i.e., when i = 1, A[2] should become A[1]
            // and A[0] should become A[3]
            if(j-i >= 0){
                outStateHex[i][j] = inStateHex[i][j-i];
            } else {
                outStateHex[i][j] = inStateHex[i][j-i+4];
            }
        }
    }

    free2DArray(inStateHex,rows);

    return outStateHex;
}

static m_u8 **AESUnmixColumn(m_u8 **inStateHex, int rows, int cols){
    m_u8 **outStateHex = (m_u8 **) malloc(sizeof (m_u8 *) * rows);
    m_u8 *workMatrix = (m_u8 *) malloc(sizeof (m_u8) * cols);

    for(int i=0;i<cols;i++){
        outStateHex[i] = (m_u8 *) malloc(sizeof (m_u8) * cols);
    }

    for(int i=0;i<rows;i++){
        workMatrix[0] = (m_u8)mul14[inStateHex[0][i]] ^ mul11[inStateHex[1][i]] ^ mul13[inStateHex[2][i]] ^ mul9[inStateHex[3][i]];
        workMatrix[1] = (m_u8)mul9[inStateHex[0][i]] ^ mul14[inStateHex[1][i]] ^ mul11[inStateHex[2][i]] ^ mul13[inStateHex[3][i]];
        workMatrix[2] = (m_u8)mul13[inStateHex[0][i]] ^ mul9[inStateHex[1][i]] ^ mul14[inStateHex[2][i]] ^ mul11[inStateHex[3][i]];
        workMatrix[3] = (m_u8)mul11[inStateHex[0][i]] ^ mul13[inStateHex[1][i]] ^ mul9[inStateHex[2][i]] ^ mul14[inStateHex[3][i]];

        outStateHex[0][i] = workMatrix[0];
        outStateHex[1][i] = workMatrix[1];
        outStateHex[2][i] = workMatrix[2];
        outStateHex[3][i] = workMatrix[3];

        // workMatrix[0] = (m_u8)mul14[inStateHex[i][0]] ^ mul11[inStateHex[i][1]] ^ mul13[inStateHex[i][2]] ^ mul9[inStateHex[i][3]];
        // workMatrix[1] = (m_u8)mul9[inStateHex[i][0]] ^ mul14[inStateHex[i][1]] ^ mul11[inStateHex[i][2]] ^ mul13[inStateHex[i][3]];
        // workMatrix[2] = (m_u8)mul13[inStateHex[i][0]] ^ mul9[inStateHex[i][1]] ^ mul14[inStateHex[i][2]] ^ mul11[inStateHex[i][3]];
        // workMatrix[3] = (m_u8)mul11[inStateHex[i][0]] ^ mul13[inStateHex[i][1]] ^ mul9[inStateHex[i][2]] ^ mul14[inStateHex[i][3]];

        // outStateHex[i][0] = workMatrix[0];
        // outStateHex[i][1] = workMatrix[1];
        // outStateHex[i][2] = workMatrix[2];
        // outStateHex[i][3] = workMatrix[3];
    }

    free(workMatrix);
    free2DArray(inStateHex,rows);

    return outStateHex;
}

/*****************************************************************************/

/* Accepts an AES key as a one-dimensional array of integers and the plaintext as a one-dimensional array of integers; returns the ciphertext */
m_u8 *AESencrypt(m_u8 AESRoundKeys[4][44], m_u8 *pTextHex, int keyLen, int pTextLen){
    m_u8 **pTextMatrix = (m_u8 **) malloc(sizeof (m_u8 *) * 4);

    for(int i=0;i<4;i++){
        pTextMatrix[i] = (m_u8 *) malloc(sizeof (m_u8) * 4);
    }

    //Transpose 1-dimensional arrays into 4 x 4 matrices
    for(int i=0;i<keyLen;i++){
        pTextMatrix[i%4][i/4] = pTextHex[i];
    }

    int round = 1;
    m_u8 **roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);

    //First round
    pTextMatrix = AESStateXOR(pTextMatrix, roundKeyMatrix, 4, 4);

    //Rounds 2 through 10
    for(round=2;round<=10;round++){
        pTextMatrix = AESNibbleSub(pTextMatrix, 4, 4);
        pTextMatrix = AESShiftRow(pTextMatrix, 4, 4);
        pTextMatrix = AESMixColumn(pTextMatrix, 4, 4);

        roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);
        pTextMatrix = AESStateXOR(pTextMatrix, roundKeyMatrix, 4, 4);
    }

    //Last rounds
    round = 11;
    roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);

    pTextMatrix = AESNibbleSub(pTextMatrix, 4, 4);
    pTextMatrix = AESShiftRow(pTextMatrix, 4, 4);

    pTextMatrix = AESStateXOR(pTextMatrix, roundKeyMatrix, 4, 4);

    m_u8 *cTextHex = (m_u8 *) malloc(sizeof (m_u8) * 4);
    
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            cTextHex[(j*4)+i] = pTextMatrix[i][j];
        }
    }

    free2DArray(roundKeyMatrix,4);
    free2DArray(pTextMatrix,4);

    return cTextHex;
}

m_u8 *AESdecrypt(m_u8 AESRoundKeys[4][44], m_u8 *cTextHex, int keyLen, int cTextLen){
    m_u8 **cTextMatrix = (m_u8 **) malloc(sizeof (m_u8 *) * 4);

    for(int i=0;i<4;i++){
        cTextMatrix[i] = (m_u8 *) malloc(sizeof (m_u8) * 4);
    }

    //Transpose 1-dimensional arrays into 4 x 4 matrices
    for(int i=0;i<keyLen;i++){
        cTextMatrix[i%4][i/4] = cTextHex[i];
    }

    int round = 11;
    m_u8 **roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);    

    cTextMatrix = AESStateXOR(cTextMatrix, roundKeyMatrix, 4, 4);

    cTextMatrix = AESUnshiftRow(cTextMatrix, 4, 4);
    cTextMatrix = AESNibbleUnsub(cTextMatrix, 4, 4);

    //Rounds 2 through 10
    for(round=10;round>=2;round--){
        roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);
        cTextMatrix = AESStateXOR(cTextMatrix, roundKeyMatrix, 4, 4);

        cTextMatrix = AESUnmixColumn(cTextMatrix, 4, 4);
        cTextMatrix = AESUnshiftRow(cTextMatrix, 4, 4);
        cTextMatrix = AESNibbleUnsub(cTextMatrix, 4, 4);
    }

    round = 1;
    roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);

    //First round
    cTextMatrix = AESStateXOR(cTextMatrix, roundKeyMatrix, 4, 4);

    m_u8 *pTextHex = (m_u8 *) malloc(sizeof (m_u8) * 4);
    
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            pTextHex[(j*4)+i] = cTextMatrix[i][j];
        }
    }

    free2DArray(roundKeyMatrix,4);
    free2DArray(cTextMatrix,4);

    return pTextHex;
}

/*****************************************************************************/

void AESinit(maruCipherDesc *cipher, void *opaque, int flags){

}


void AESsetkey(void *opaque, m_u8 *key, int len, int flags) {
    aes_ctx *keyRounds = (aes_ctx *)opaque;
    m_u8 **keyMatrix = (m_u8 **) malloc(sizeof (m_u8 *) * 4);

    for(int i=0;i<4;i++){
        keyMatrix[i] = (m_u8 *) malloc(sizeof (m_u8) * 4);
    }

    //Transpose 1-dimensional arrays into 4 x 4 matrices
    for(int i=0;i<len;i++){
        keyMatrix[i%4][i/4] = key[i];
    }

    AESRoundKeysMatrix(keyMatrix,keyRounds->K);

}

void AESencryptCBC(void *opaque, m_u8 *iv, m_u8 *data, int len) {
    aes_ctx *keyRounds = (aes_ctx *)opaque;
    m_u8 *_data = malloc(sizeof (m_u8) * len);
    m_u8 *start = *(&_data);
    int keyLen = 16;
    int blockSize = 16;

    int n;
    m_u8 *tmp;
    memcpy(_data,data,sizeof (m_u8) * len);

    if (iv){
	    _data[0] ^= iv[0];
        _data[1] ^= iv[1];
        _data[2] ^= iv[2];
        _data[3] ^= iv[3];
        _data[4] ^= iv[4];
        _data[5] ^= iv[5];
        _data[6] ^= iv[6];
        _data[7] ^= iv[7];
        _data[8] ^= iv[8];
        _data[9] ^= iv[9];
        _data[10] ^= iv[10];
        _data[11] ^= iv[11]; 
        _data[12] ^= iv[12];
        _data[13] ^= iv[13];
        _data[14] ^= iv[14];
        _data[15] ^= iv[15];
    }

    //Call AES encrypt on the current contents of data
    tmp = AESencrypt(keyRounds->K,_data,keyLen,blockSize);

    //Copy back the result
    memcpy(_data,tmp,sizeof (m_u8) * blockSize);

    
    //Increment the pointer of data
    _data+=(sizeof (m_u8) * blockSize);
    //memset(_data,0,sizeof (m_u8) * 16);
    
    for (n=1; n<(len/16); n++)
	{
	    _data[0] ^= tmp[0];
        _data[1] ^= tmp[1];
        _data[2] ^= tmp[2];
        _data[3] ^= tmp[3];
        _data[4] ^= tmp[4];
        _data[5] ^= tmp[5];
        _data[6] ^= tmp[6];
        _data[7] ^= tmp[7];
        _data[8] ^= tmp[8];
        _data[9] ^= tmp[9];
        _data[10] ^= tmp[10];
        _data[11] ^= tmp[11]; 
        _data[12] ^= tmp[12];
        _data[13] ^= tmp[13];
        _data[14] ^= tmp[14];
        _data[15] ^= tmp[15];

	    tmp = AESencrypt(keyRounds->K,_data,keyLen,blockSize);
        memcpy(_data,tmp,sizeof (m_8) * blockSize);
        //*_data = tmp;
        _data+=blockSize;
	}

    memcpy(data,start,sizeof (m_u8) * len);
    free(tmp);
    //free(_data);
}

void AESencryptCBCTo(void *opaque, m_u8 *iv, m_u8 *data, m_u8 *to, int len){
    aes_ctx *keyRounds = (aes_ctx *)opaque;
    m_u8 *_data = malloc(sizeof (m_u8) * len);
    m_u8 *start = *(&_data);
    int keyLen = 16;
    int blockSize = 16;

    int n;
    m_u8 *tmp;
    memcpy(_data,data,sizeof (m_u8) * len);

    if (iv){
	    _data[0] ^= iv[0];
        _data[1] ^= iv[1];
        _data[2] ^= iv[2];
        _data[3] ^= iv[3];
        _data[4] ^= iv[4];
        _data[5] ^= iv[5];
        _data[6] ^= iv[6];
        _data[7] ^= iv[7];
        _data[8] ^= iv[8];
        _data[9] ^= iv[9];
        _data[10] ^= iv[10];
        _data[11] ^= iv[11]; 
        _data[12] ^= iv[12];
        _data[13] ^= iv[13];
        _data[14] ^= iv[14];
        _data[15] ^= iv[15];
    }

    //Call AES encrypt on the current contents of data
    tmp = AESencrypt(keyRounds->K,_data,keyLen,blockSize);

    //Copy back the result
    memcpy(_data,tmp,sizeof (m_u8) * blockSize);

    
    //Increment the pointer of data
    _data+=(sizeof (m_u8) * blockSize);
    //memset(_data,0,sizeof (m_u8) * 16);
    
    for (n=1; n<(len/16); n++)
	{
	    _data[0] ^= tmp[0];
        _data[1] ^= tmp[1];
        _data[2] ^= tmp[2];
        _data[3] ^= tmp[3];
        _data[4] ^= tmp[4];
        _data[5] ^= tmp[5];
        _data[6] ^= tmp[6];
        _data[7] ^= tmp[7];
        _data[8] ^= tmp[8];
        _data[9] ^= tmp[9];
        _data[10] ^= tmp[10];
        _data[11] ^= tmp[11]; 
        _data[12] ^= tmp[12];
        _data[13] ^= tmp[13];
        _data[14] ^= tmp[14];
        _data[15] ^= tmp[15];

	    tmp = AESencrypt(keyRounds->K,_data,keyLen,blockSize);
        memcpy(_data,tmp,sizeof (m_8) * blockSize);
        //*_data = tmp;
        _data+=blockSize;
	}

    memcpy(to,start,sizeof (m_u8) * len);
    free(tmp);
    //free(_data);
}

void AESdecryptCBC(void *opaque, m_u8 *iv, m_u8 *data, int len) {
    aes_ctx *keyRounds = (aes_ctx *)opaque;
    m_u8 *_data = malloc(sizeof (m_u8) * len);
    m_u8 *prev;
    int keyLen = 16;
    int blockSize = 16;

    int n;
    m_u8 *tmp;
    memcpy(_data,data,sizeof (m_u8) * len);

    //Start from the beginning of the last block
    _data += (sizeof (m_u8) * (len - blockSize));

    // Decrypt current block, XOR with previous block (still encrypted) to get plaintext
    for (n=(len/16); n>1; n--)
	{
        //Call AES decrypt on the current contents of data
        tmp = AESdecrypt(keyRounds->K,_data,keyLen,blockSize);

        //Save the current position of the data pointer
        prev = *(&_data);

        //Move data pointer backwards by one block
        _data -= (sizeof (m_u8) * blockSize);

        //XOR the current decrypted block with the encrypted contents
        //of the previous block
	    tmp[0] ^= _data[0];
        tmp[1] ^= _data[1];
        tmp[2] ^= _data[2];
        tmp[3] ^= _data[3];
        tmp[4] ^= _data[4];
        tmp[5] ^= _data[5];
        tmp[6] ^= _data[6];
        tmp[7] ^= _data[7];
        tmp[8] ^= _data[8];
        tmp[9] ^= _data[9];
        tmp[10] ^= _data[10];
        tmp[11] ^= _data[11]; 
        tmp[12] ^= _data[12];
        tmp[13] ^= _data[13];
        tmp[14] ^= _data[14];
        tmp[15] ^= _data[15];

        //Copy the current block back to data
        memcpy(prev,tmp,sizeof (m_8) * blockSize);
	}

    tmp = AESdecrypt(keyRounds->K,_data,keyLen,blockSize);
    memcpy(_data,tmp,sizeof (m_u8) * blockSize);

    if (iv){
	    _data[0] ^= iv[0];
        _data[1] ^= iv[1];
        _data[2] ^= iv[2];
        _data[3] ^= iv[3];
        _data[4] ^= iv[4];
        _data[5] ^= iv[5];
        _data[6] ^= iv[6];
        _data[7] ^= iv[7];
        _data[8] ^= iv[8];
        _data[9] ^= iv[9];
        _data[10] ^= iv[10];
        _data[11] ^= iv[11]; 
        _data[12] ^= iv[12];
        _data[13] ^= iv[13];
        _data[14] ^= iv[14];
        _data[15] ^= iv[15];
    }

    memcpy(data,_data,sizeof (m_u8) * len);

    free(tmp);
    //free(_data);
}

void AESdecryptCBCTo(void *opaque, m_u8 *iv, m_u8 *data, m_u8 *to, int len){
    aes_ctx *keyRounds = (aes_ctx *)opaque;
    m_u8 *_data = malloc(sizeof (m_u8) * len);
    m_u8 *prev;
    int keyLen = 16;
    int blockSize = 16;

    int n;
    m_u8 *tmp;
    memcpy(_data,data,sizeof (m_u8) * len);

    //Start from the beginning of the last block
    _data += (sizeof (m_u8) * (len - blockSize));

    // Decrypt current block, XOR with previous block (still encrypted) to get plaintext
    for (n=(len/16); n>1; n--)
	{
        //Call AES decrypt on the current contents of data
        tmp = AESdecrypt(keyRounds->K,_data,keyLen,blockSize);

        //Save the current position of the data pointer
        prev = *(&_data);

        //Move data pointer backwards by one block
        _data -= (sizeof (m_u8) * blockSize);

        //XOR the current decrypted block with the encrypted contents
        //of the previous block
	    tmp[0] ^= _data[0];
        tmp[1] ^= _data[1];
        tmp[2] ^= _data[2];
        tmp[3] ^= _data[3];
        tmp[4] ^= _data[4];
        tmp[5] ^= _data[5];
        tmp[6] ^= _data[6];
        tmp[7] ^= _data[7];
        tmp[8] ^= _data[8];
        tmp[9] ^= _data[9];
        tmp[10] ^= _data[10];
        tmp[11] ^= _data[11]; 
        tmp[12] ^= _data[12];
        tmp[13] ^= _data[13];
        tmp[14] ^= _data[14];
        tmp[15] ^= _data[15];

        //Copy the current block back to data
        memcpy(prev,tmp,sizeof (m_8) * blockSize);
	}

    tmp = AESdecrypt(keyRounds->K,_data,keyLen,blockSize);
    memcpy(_data,tmp,sizeof (m_u8) * blockSize);

    if (iv){
	    _data[0] ^= iv[0];
        _data[1] ^= iv[1];
        _data[2] ^= iv[2];
        _data[3] ^= iv[3];
        _data[4] ^= iv[4];
        _data[5] ^= iv[5];
        _data[6] ^= iv[6];
        _data[7] ^= iv[7];
        _data[8] ^= iv[8];
        _data[9] ^= iv[9];
        _data[10] ^= iv[10];
        _data[11] ^= iv[11]; 
        _data[12] ^= iv[12];
        _data[13] ^= iv[13];
        _data[14] ^= iv[14];
        _data[15] ^= iv[15];
    }

    memcpy(to,_data,sizeof (m_u8) * len);

    free(tmp);
    //free(_data);
}

// void AEScryptCBC(void *opaque, u_m_u8 *iv, u_m_u8 *data, u_m_u8 *to, int len, int flags){

// }