import java.util.Arrays;
import java.util.Scanner;
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
        if(args.length == 1){
            runTests();
        } else {
            Scanner s = new Scanner(System.in);
            String sysKey = s.next();
            String plaintText = s.next();

            AESCipher cipher = new AESCipher();

            int[] keyHex = cipher.str2hex(sysKey);
            int[] pTextHex = cipher.str2hex(plaintText);

            int[] cTextHex = cipher.AES(keyHex, pTextHex);
            String cText = cipher.hex2str(cTextHex);

            System.out.println(cText);
        }
    }

    public static void runTests(){
        AESCipher cipher = new AESCipher();
        String[] sysKeys = {
            "E8E9EAEBEDEEEFF0F2F3F4F5F7F8F9FA",
            "000000000000000000000000DEADBEEF",
            "0000000000000000DEADBEEF00000000",
            "00000000DEADBEEF0000000000000000",
            "DEADBEEF000000000000000000000000"
        };
        String[] plainTexts = {
            "014BAF2278A69D331D5180103643E99A",
            "0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F",
            "0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F",
            "0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F",
            "0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F",
        };
        String[] cipherTexts = {
            "6743C3D1519AB4F2CD9A78AB09A511BD",
            "E4192FDE43E21B47063D7591868210AC",
            "120AAFC46F29B20B8CB8C5D8E78B3800",
            "BA0E8DC16BB5A50C75FB1EA42CD8FE58",
            "39DBD43A9497C07187C5DCD2571E6DDD"
        };

        for(int i=0;i<sysKeys.length;i++){
            int[] keyHex = cipher.str2hex(sysKeys[i]);
            int[] pTextHex = cipher.str2hex(plainTexts[i]);
            
            System.out.println(String.format("---Test #%d---", i+1));
            System.out.println("Key:                    " + sysKeys[i]);
            System.out.println("Plaintext:              " + plainTexts[i]);

            int[] cTextHex = cipher.AES(keyHex, pTextHex);
            String cText = cipher.hex2str(cTextHex);

            System.out.println("Expected Ciphertext:    " + cipherTexts[i]);
            System.out.println("Actual Ciphertext:      " + cText);

            if(cText.equals(cipherTexts[i])){
                System.out.println("Test Result:            Pass");
            } else {
                System.out.println("Test Result:            Fail");
            }
        }

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