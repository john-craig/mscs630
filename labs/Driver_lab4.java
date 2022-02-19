import java.io.*;
import java.util.Scanner;

/**
* file:     Driver_lab4.java
* author:   John Craig
* course:   MSCS 630
* assignment: Lab 4
* due date: February 20, 2022
* version: 1.0
*
* This file contains the source code for the first
* lab assignment.
*/

/**
* Driver_lab4
*
*/

//5468617473206D79204B756E67204675

public class Driver_lab4 {
    //Declare constants for input
    static final int MAX_PLAINTEXT_LENGTH = 16;
    static final int MATRIX_SIZE = 4;

    /* Main method of the program; accepts lines of text as input and prints
       each line with the characters converted to integers */
    public static void main(String args[]) {
        //Scanner s = new Scanner(System.in);
        //String sysKey = s.next();
        String sysKey = "5468617473206D79204B756E67204675";

        AESCipher cipher = new AESCipher();
        String[] roundKeys = cipher.aesRounderKeys(sysKey);

        for(int i=0;i<roundKeys.length;i++){
            System.out.println(roundKeys[i]);
        }
    }

}