import java.util.Arrays;  

public class AESCipher {

    public static int[][] AESStateXOR(int[][] sHex, int[][] keyHex){
        int[][] outStateHex = new int[sHex.length][sHex[0].length];

        for(int i=0;i<sHex.length;i++){
            for(int j=0;j<sHex[0].length;j++){
                outStateHex[i][j] = sHex[i][j] ^ keyHex[i][j];
            }
        }

        return outStateHex;
    }

    public static int[][] AESNibbleSub(int[][] inStateHex){
        int[][] outStateHex = new int[4][4];

        for(int i=0;i<inStateHex.length;i++){
            for(int j=0;j<inStateHex.length;j++){
                outStateHex[i][j] = aesBox(inStateHex[i][j]);
            }
        }

        return outStateHex;
    }

    public static int[][] AESShiftRow(int[][] inStateHex){
        int[][] outStateHex = new int[4][4];
        int offset;

        for(int i=0;i<inStateHex.length;i++){
            for(int j=0;j<inStateHex.length;j++){
                if(j+i >= 4){
                    outStateHex[i][j] = inStateHex[i][j+i-4];
                } else {
                    outStateHex[i][j] = inStateHex[i][j+i];
                }
            }
        }

        return outStateHex;
    }

    public static int[][] AESMixColumn(int[][] inStateHex){
        int[][] outStateHex = new int[4][4];
        byte[][] workMatrix = new byte[2][4];

        for(int i=0;i<inStateHex.length;i++){
            byte c, d;
    
            for (int j = 0; j < 4; j++) {
                d = (byte)inStateHex[j][i];

                workMatrix[0][j] = d;
                
                c = (byte)((byte)(d >> 7) & 1);
                
                workMatrix[1][j] = (byte)(d << 1);
                workMatrix[1][j] ^= c * 0x1B;
            }

            outStateHex[0][i] = Byte.toUnsignedInt((byte)(workMatrix[1][0] ^ (byte)(workMatrix[0][3] ^ (byte)(workMatrix[0][2] ^ (byte)(workMatrix[1][1] ^ workMatrix[0][1])))));
            outStateHex[1][i] = Byte.toUnsignedInt((byte)(workMatrix[1][1] ^ (byte)(workMatrix[0][0] ^ (byte)(workMatrix[0][3] ^ (byte)(workMatrix[1][2] ^ workMatrix[0][2])))));
            outStateHex[2][i] = Byte.toUnsignedInt((byte)(workMatrix[1][2] ^ (byte)(workMatrix[0][1] ^ (byte)(workMatrix[0][0] ^ (byte)(workMatrix[1][3] ^ workMatrix[0][3])))));
            outStateHex[3][i] = Byte.toUnsignedInt((byte)(workMatrix[1][3] ^ (byte)(workMatrix[0][2] ^ (byte)(workMatrix[0][1] ^ (byte)(workMatrix[1][0] ^ workMatrix[0][0])))));
        }

        return outStateHex;
    }

    public static int[] AES(int[] keyHex, int[] pTextHex){
        int[][] keyMatrix = new int[4][4];
        int[][] cTextMatrix = new int[4][4];

        //Transpose 1-dimensional arrays into 4 x 4 matrices
        for(int i=0;i<keyHex.length;i++){
            //keyMatrix[i/4][i%4] = keyHex[i];
            //cTextMatrix[i/4][i%4] = pTextHex[i];
            keyMatrix[i%4][i/4] = keyHex[i];
            cTextMatrix[i%4][i/4] = pTextHex[i];
        }

        //Generate the RoundKeys Array
        int[][] AESRoundKeys = AESRoundKeysMatrix(keyMatrix);

        int round = 1;
        int[][] roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);

        // printHex(roundKeyMatrix);
        // System.out.println("---");
        printHex(AESRoundKeys);

        //First round
        cTextMatrix = AESStateXOR(cTextMatrix, roundKeyMatrix);

        //Rounds 2 through 10
        for(round=2;round<=10;round++){
            cTextMatrix = AESNibbleSub(cTextMatrix);
            cTextMatrix = AESShiftRow(cTextMatrix);
            cTextMatrix = AESMixColumn(cTextMatrix);

            roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);
            // printHex(roundKeyMatrix);
            // System.out.println("---");
            cTextMatrix = AESStateXOR(cTextMatrix, roundKeyMatrix);
        }

        //Last round
        round = 11;
        roundKeyMatrix = AESGetRoundKey(AESRoundKeys, round);

        cTextMatrix = AESNibbleSub(cTextMatrix);
        cTextMatrix = AESShiftRow(cTextMatrix);

        cTextMatrix = AESStateXOR(cTextMatrix, roundKeyMatrix);

        int[] cTextHex = new int[16];
        
        for(int i=0;i<cTextMatrix.length;i++){
            for(int j=0;j<cTextMatrix[0].length;j++){
                cTextHex[(j*4)+i] = cTextMatrix[i][j];
            }
        }

        return cTextHex;
    }

    public static String[] aesRounderKeys(String keyHex){
        String[] roundKeysHex = new String[11];

        int[] keyRow = new int[4];
        int[][] keyMatrix = new int[4][4];
        int[][] workMatrix = new int[44][4];

        for(int i=0;i<keyMatrix.length;i++){
            keyMatrix[i] = str2hex(keyHex.substring(i*8,(i*8)+8));
            keyRow = str2hex(keyHex.substring(i*8,(i*8)+8));

            keyMatrix[0][i] = keyRow[0];
            keyMatrix[1][i] = keyRow[1];
            keyMatrix[2][i] = keyRow[2];
            keyMatrix[3][i] = keyRow[3];
        }
        
        workMatrix = AESRoundKeysMatrix(keyMatrix);

        for(int i=0;i<workMatrix.length;i++){
            if(i%4==0){
                roundKeysHex[i/4] = "";
            }

            roundKeysHex[i/4] += hex2str(workMatrix[i]);
        }

        return roundKeysHex;
    }

    // Returns a 4 x 4 matrix for the current round, where 1 <= round <= 11
    public static int[][] AESGetRoundKey(int[][] AESRoundKeysMatrix, int round){
        int[][] keyMatrix = new int[4][4];
        int startIndex = (round - 1) * 4;

        for(int i=0;i<4;i++){
            for(int j=0;j<4;j++){
                keyMatrix[i][j] = AESRoundKeysMatrix[i][j+startIndex];
            }
        }

        return keyMatrix;
    }

    public static int[][] AESRoundKeysMatrix(int[][] keyMatrix){
        int[][] workMatrix = new int[4][44];

        //Round zero
        for(int k=0;k<4;k++){
            workMatrix[k][0] = keyMatrix[k][0];
            workMatrix[k][1] = keyMatrix[k][1];
            workMatrix[k][2] = keyMatrix[k][2];
            workMatrix[k][3] = keyMatrix[k][3];
        }

        int[] tempMatrix = new int[4];
        int round;

        //Remaining rounds
        for(int j=4;j<44;j++){
            if(j % 4 != 0){
                workMatrix[0][j] = workMatrix[0][j-4] ^ workMatrix[0][j-1];
                workMatrix[1][j] = workMatrix[1][j-4] ^ workMatrix[1][j-1];
                workMatrix[2][j] = workMatrix[2][j-4] ^ workMatrix[2][j-1];
                workMatrix[3][j] = workMatrix[3][j-4] ^ workMatrix[3][j-1];
            } else {
                round = j / 4;

                //Shift previous matrix column by one
                tempMatrix[0] = workMatrix[1][j-1];
                tempMatrix[1] = workMatrix[2][j-1];
                tempMatrix[2] = workMatrix[3][j-1];
                tempMatrix[3] = workMatrix[0][j-1];

                //Apply sBox transformation and round constant modulation
                tempMatrix[0] = aesBox(tempMatrix[0]) ^ aesRcon(round);
                tempMatrix[1] = aesBox(tempMatrix[1]); //^ aesRcon(round);
                tempMatrix[2] = aesBox(tempMatrix[2]); //^ aesRcon(round);
                tempMatrix[3] = aesBox(tempMatrix[3]); //^ aesRcon(round);

                workMatrix[0][j] = tempMatrix[0] ^ workMatrix[0][j-4];
                workMatrix[1][j] = tempMatrix[1] ^ workMatrix[1][j-4];
                workMatrix[2][j] = tempMatrix[2] ^ workMatrix[2][j-4];
                workMatrix[3][j] = tempMatrix[3] ^ workMatrix[3][j-4];
            }
        }

        return workMatrix;
    }

    // public static int[][] AESRoundKeysMatrix(int[][] keyMatrix){
    //     int[][] workMatrix = new int[44][4];

    //     //Round zero
    //     for(int k=0;k<4;k++){
    //         workMatrix[k][0] = keyMatrix[k][0];
    //         workMatrix[k][1] = keyMatrix[k][1];
    //         workMatrix[k][2] = keyMatrix[k][2];
    //         workMatrix[k][3] = keyMatrix[k][3];
    //     }

    //     int[] tempMatrix = new int[4];
    //     int round;

    //     //Remaining rounds
    //     for(int j=4;j<44;j++){
    //         if(j % 4 != 0){
    //             workMatrix[j][0] = workMatrix[j-4][0] ^ workMatrix[j-1][0];
    //             workMatrix[j][1] = workMatrix[j-4][1] ^ workMatrix[j-1][1];
    //             workMatrix[j][2] = workMatrix[j-4][2] ^ workMatrix[j-1][2];
    //             workMatrix[j][3] = workMatrix[j-4][3] ^ workMatrix[j-1][3];
    //         } else {
    //             round = j / 4;

    //             //Shift previous matrix column by one
    //             tempMatrix[0] = workMatrix[j-1][1];
    //             tempMatrix[1] = workMatrix[j-1][2];
    //             tempMatrix[2] = workMatrix[j-1][3];
    //             tempMatrix[3] = workMatrix[j-1][0];

    //             //Apply sBox transformation and round constant modulation
    //             tempMatrix[0] = aesBox(tempMatrix[0]) ^ aesRcon(round);
    //             tempMatrix[1] = aesBox(tempMatrix[1]); //^ aesRcon(round);
    //             tempMatrix[2] = aesBox(tempMatrix[2]); //^ aesRcon(round);
    //             tempMatrix[3] = aesBox(tempMatrix[3]); //^ aesRcon(round);

    //             workMatrix[j][0] = tempMatrix[0] ^ workMatrix[j-4][0];
    //             workMatrix[j][1] = tempMatrix[1] ^ workMatrix[j-4][1];
    //             workMatrix[j][2] = tempMatrix[2] ^ workMatrix[j-4][2];
    //             workMatrix[j][3] = tempMatrix[3] ^ workMatrix[j-4][3];
    //         }
    //     }

    //     return workMatrix;
    // }

    public static int[] str2hex(String str){
        int[] hex = new int[str.length() / 2];

        for(int i=0;i<str.length();i++){
            if(i%2 == 0){
                char c1 = str.charAt(i);
                char c2 = str.charAt(i+1);

                if((int)c1 >= 48 && (int)c1 <= 57){
                    hex[i/2] = ((int)c1 - 48) * 16;
                } else {
                    hex[i/2] = ((int)c1 - 55) * 16;
                }


                if((int)c2 >= 48 && (int)c2 <= 57){
                    hex[i/2] += ((int)c2 - 48);
                } else {
                    hex[i/2] += ((int)c2 - 55);
                }
            }
        }

        return hex;
    }

    public static String hex2str(int[] hex){
        String str = "";

        for(int i=0;i<hex.length;i++){
            str += String.format("%X", hex[i]);
        }

        return str;
    }

    public static int aesBox(int inHex){
        int row = inHex / 16;
        int col = inHex % 16;

        int[][] sBox = {
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

    public static int aesRcon(int round){
        int[] rConstants = {
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

    public static void printHex(int[][] hexMat){
        for(int i=0;i<hexMat.length;i++){
            for (int j=0;j<hexMat[0].length;j++){
                System.out.print(String.format("%X ", hexMat[i][j]));
            }
            System.out.println("");
        }
    }
}