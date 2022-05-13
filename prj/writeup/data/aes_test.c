#include "aes.h"
#include "../src/maru/maru.h"
#include <stdlib.h>
#include <stdio.h>

m_u8 *str2hex(m_u8 *str, int len){
    m_u8 *hex = (m_u8 *)malloc(sizeof (m_u8) * (len/2));

    for(int i=0;i<len;i++){
        m_u8 c;

        switch(str[i]){
            case 'A':
                c = 10;
                break;
            case 'B':
                c = 11;
                break;
            case 'C':
                c = 12;
                break;
            case 'D':
                c = 13;
                break;
            case 'E':
                c = 14;
                break;
            case 'F':
                c = 15;
                break;
            case '0':
                c = 0;
                break;
            case '1':
                c = 1;
                break;
            case '2':
                c = 2;
                break;
            case '3':
                c = 3;
                break;
            case '4':
                c = 4;
                break;
            case '5':
                c = 5;
                break;
            case '6':
                c = 6;
                break;
            case '7':
                c = 7;
                break;
            case '8':
                c = 8;
                break;
            case '9':
                c = 9;
                break;
        }

        //printf("%c",str[i]);

        if(i%2 == 0){
            hex[i/2] = c * 16;
        } else {
            hex[i/2] = (hex[i/2] + c);
        }

    }

    return hex;
}

m_u8 *hex2str(m_u8 *hex, int len){
    m_u8 *str = (m_u8 *)malloc(sizeof (m_u8) * ((len*2)+1));

    for(int i=0;i<len;i++){
        m_u8 c1 = hex[i] / 16;
        m_u8 c2 = hex[i] % 16;

        switch(c1){
            case 0:
                str[(i*2)] = '0';
                break;
            case 1:
                str[(i*2)] = '1';
                break;
            case 2:
                str[(i*2)] = '2';
                break;
            case 3:
                str[(i*2)] = '3';
                break;
            case 4:
                str[(i*2)] = '4';
                break;
            case 5:
                str[(i*2)] = '5';
                break;
            case 6:
                str[(i*2)] = '6';
                break;
            case 7:
                str[(i*2)] = '7';
                break;
            case 8:
                str[(i*2)] = '8';
                break;
            case 9:
                str[(i*2)] = '9';
                break;
            case 10:
                str[(i*2)] = 'A';
                break;
            case 11:
                str[(i*2)] = 'B';
                break;
            case 12:
                str[(i*2)] = 'C';
                break;
            case 13:
                str[(i*2)] = 'D';
                break;
            case 14:
                str[(i*2)] = 'E';
                break;
            case 15:
                str[(i*2)] = 'F';
                break;
        }

        switch(c2){
            case 0:
                str[(i*2)+1] = '0';
                break;
            case 1:
                str[(i*2)+1] = '1';
                break;
            case 2:
                str[(i*2)+1] = '2';
                break;
            case 3:
                str[(i*2)+1] = '3';
                break;
            case 4:
                str[(i*2)+1] = '4';
                break;
            case 5:
                str[(i*2)+1] = '5';
                break;
            case 6:
                str[(i*2)+1] = '6';
                break;
            case 7:
                str[(i*2)+1] = '7';
                break;
            case 8:
                str[(i*2)+1] = '8';
                break;
            case 9:
                str[(i*2)+1] = '9';
                break;
            case 10:
                str[(i*2)+1] = 'A';
                break;
            case 11:
                str[(i*2)+1] = 'B';
                break;
            case 12:
                str[(i*2)+1] = 'C';
                break;
            case 13:
                str[(i*2)+1] = 'D';
                break;
            case 14:
                str[(i*2)+1] = 'E';
                break;
            case 15:
                str[(i*2)+1] = 'F';
                break;
        }

        // printf("%c",str[(i*2)]);
        // printf("%c",str[(i*2)+1]);
    }

    //printf("%d\n",(len*2));
    //str[len*2] = '\0';

    return str;
}

int
main(int argc, m_u8 **argv)
{
    m_u8 *iv     = "DEADBEEFDEADBEEFDEADBEEFDEADBEEF";
    m_u8 *sysKey = "E8E9EAEBEDEEEFF0F2F3F4F5F7F8F9FA";
    m_u8 *pText  = "014BAF2278A69D331D5180103643E99A014BAF2278A69D331D5180103643E99A";
    // m_u8 *cText  = "6743C3D1519AB4F2CD9A78AB09A511BD";
    
    m_u8 *sysKeyHex = str2hex(sysKey,32);
    m_u8 *pTextHex = str2hex(pText,64);
    m_u8 *ivHex    = str2hex(iv,32);
    // m_u8 *cTextHex = str2hex(cText,32);
    // m_u8 *oText;
    
    
    // printf("Starting Plaintext:\t\t%s\n",pText);
    // printf("Starting Ciphertext:\t\t%s\n",cText);

    // cTextHex = AESencrypt(sysKeyHex,pTextHex,16,16);
    // oText    = hex2str(cTextHex,16);
    // printf("Encrypted Ciphertext:\t\t%s\n",oText);

    // pTextHex = AESdecrypt(sysKeyHex,cTextHex,16,16);
    // oText    = hex2str(pTextHex,16);
    // printf("Decrypted Plaintext:\t\t%s\n", oText);

    maruCipherDesc *cipher;
    void *opaque = malloc(sizeof(aes_ctx));;


    AESinit(cipher,opaque,0);
    AESsetkey(opaque,sysKeyHex,16,0);

    printf("%s\n", hex2str(pTextHex,32));

    AESencryptCBC(opaque,ivHex,pTextHex,32);

    printf("%s\n", hex2str(pTextHex,32));

    AESdecryptCBC(opaque,ivHex,pTextHex,32);

    printf("%s\n", hex2str(pTextHex,32));

    free(sysKeyHex);
    free(pTextHex);
    free(ivHex);

    return 0;
}