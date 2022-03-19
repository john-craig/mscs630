/**
* file:     Driver_lab1.java
* author:   John Craig
* course:   MSCS 630
* assignment: Lab 1
* due date: January 23, 2022
* version: 1.0
*
* This file contains the source code for the first
* lab assignment.
*/

/**
* Driver_lab1
*
* This class contains a program which converts one or more lines of text into integers.
*/
public class Driver_lab1 {
    //Declare constants for input
    static final int MAX_LETTERS = 45;
    static final int MAX_WORDS = 500;
    static final int MAX_LINES = 50;

    /* Main method of the program; accepts lines of text as input and prints
       each line with the characters converted to integers */
    public static void main(String args[]) throws IllegalArgumentException {
        int numLines = args.length;

        try {
            if(numLines > MAX_LINES){
            //Input is too long
                throw new IllegalArgumentException("Input greater than 50 lines. Must be between 1 and 50 lines.");
            } else if(numLines == 0) {
                throw new IllegalArgumentException("No lines were input. Must be between 1 and 50 lines.");
            } else {
                printCipher(args);
            }
         } catch(IllegalArgumentException e){
             System.exit(0);
         }
        

    }

    /* No output. Prints the cipher corresponding
       to each line of its input */
    public static void printCipher(String lines[]){
        //Declare working variables
        int[] ciphertext;
        String output;

        //Iterate over lines
        for(int i=0;i<lines.length;i++){
            ciphertext = str2int(lines[i]);
            output = "";

            for(int j=0;j<ciphertext.length;j++){
                output = output + Integer.toString(ciphertext[j]) + " ";
            }

            System.out.println(output);
        }
    }


    /* Returns the input string converted to an array of
       integers, such that a-z and A-Z are 0-25, and ' ' is 26 */
    public static int[] str2int(String plaintext) throws IllegalArgumentException {
        //Determine the number of characters in the string
        int numChars = plaintext.length();

        //Allocate ciphertext array
        int[] ciphertext = new int[numChars];

        //Declare working variables to track letters and words
        int curLetter = 0;
        int curWord = 0;
        boolean illegalChar = false;

        //Perform conversion
        for(int i=0;i<numChars && curLetter <= MAX_LETTERS && curWord <= MAX_WORDS && !illegalChar;i++){
            int ch = (int)plaintext.charAt(i);

            if(ch == 32){
                //Space
                ciphertext[i] = 26;
                curLetter = 0;
                curWord++;
            } else if(ch >= 65 && ch <= 90){
                //Uppercase letter
                ciphertext[i] = ch - 65;
                curLetter++;
            } else if(ch >= 97 && ch <= 122){
                //Lowercase letter
                ciphertext[i] = ch - 97;
                curLetter++;
            } else {
                //Illegal character
                illegalChar = true;
            }
        }

        //Validate the line.
        try {
            if(curLetter > MAX_LETTERS){
                //Too many letters in a word
                throw new IllegalArgumentException("A word container too many letters. Must be between 1 and 45 letters.");
            } else if (curLetter == 0){
                //No letters in a word
                throw new IllegalArgumentException("A word container no letters. Must be between 1 and 45 letters.");
            }

            if(curWord > MAX_WORDS){
                //Too many words in a line
                throw new IllegalArgumentException("A line contained too many words. Must be between 1 and 500 words.");
            } else if (curWord == 0){
                throw new IllegalArgumentException("A line contained too few words. Must be between 1 and 500 words.");
            }

            if(illegalChar){
                //There was an illegal character in input
                throw new IllegalArgumentException("Input contained an illegal character. Must be only a-z, A-Z, and ' '.");
            }
        } catch(IllegalArgumentException e){
            System.exit(0);
        }

        return ciphertext;
    }
}