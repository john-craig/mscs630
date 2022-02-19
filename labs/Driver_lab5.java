import java.util.Arrays;
/**
* file:     Driver_lab5.java
* author:   John Craig
* course:   MSCS 630
* assignment: Lab 5
* due date: February 20, 2022
* version: 1.0
*
* This file contains the source code for the first
* lab assignment.
*/

/**
* Driver_lab5
*
*/
public class Driver_lab5 {
    //Declare constants for input
    static final int MAX_PLAINTEXT_LENGTH = 16;
    static final int MATRIX_SIZE = 4;

    /*  */
    public static void main(String args[]) {
        // Scanner s = new Scanner(System.in);
        // String sysKey = s.next();
        // String plaintText = s.next();

        // AESCipher cipher = new AESCipher();

        // int[] keyHex = cipher.str2hex(sysKey);
        // int[] pTextHex = cipher.str2hex(plaintText);

        // int[] cTextHex = cipher.AES(keyHex, pTextHex);
        // String cText = cipher.hex2str(cTextHex);

        // System.out.println(cText);
        runTests();
    }

    public static void runTests(){
        AESCipher cipher = new AESCipher();

        int[][] addKey1 = {
            {0x54, 0x4F, 0x4E, 0x20},
            {0x77, 0x6E, 0x69, 0x54},
            {0x6F, 0x65, 0x6E, 0x77},
            {0x20, 0x20, 0x65, 0x6F}
        };
        int[][] addKey2 = {
            {0x54, 0x73, 0x20, 0x67},
            {0x68, 0x20, 0x4B, 0x20},
            {0x61, 0x6D, 0x75, 0x46},
            {0x74, 0x79, 0x6E, 0x75}
        };

        int[][] nibSub = cipher.AESStateXOR(addKey1, addKey2);
        
        int[][] shiftRows = cipher.AESNibbleSub(nibSub);

        int[][] mixCol = cipher.AESShiftRow(shiftRows);

        int[][] last = cipher.AESMixColumn(mixCol);
        printHex(last);

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