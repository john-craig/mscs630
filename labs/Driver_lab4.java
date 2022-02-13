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
public class Driver_lab4 {
    //Declare constants for input
    static final int MAX_PLAINTEXT_LENGTH = 16;
    static final int MATRIX_SIZE = 4;

    /* Main method of the program; accepts lines of text as input and prints
       each line with the characters converted to integers */
    public static void main(String args[]) {
        char padding = args[0].charAt(0);
        String plaintext = args[1];
        int[][] hexMatrix = {};
        
        //Determine the number of iterations based on the length of the plaintext
        int iterations = (plaintext.length()/MAX_PLAINTEXT_LENGTH) + (plaintext.length() % MAX_PLAINTEXT_LENGTH) == 0 ? 0 : 1;

        //Iterate over the plaintext
        for(int i=0; i< iterations; i++){
            //Get the hexmatric of the substring
            hexMatrix = getHexMatP(
                padding,
                //Determine whether the second index of the substring should end at the next sixteen-byte offset
                //or at the end of the plaintext string
                plaintext.substring(
                    i*MAX_PLAINTEXT_LENGTH,
                    (i+1 * MAX_PLAINTEXT_LENGTH <= plaintext.length()-1) ? i+1 * MAX_PLAINTEXT_LENGTH : plaintext.length()-1
                )
            );

            prntHexMat(hexMatrix);
        }
    }


    public static void prntHexMat(int[][] hexMatrix){
        for(int i=0;i<MATRIX_SIZE;i++){
            System.out.println(
                String.format("%x", hexMatrix[i][0]) + " " + 
                String.format("%x", hexMatrix[i][1]) + " " +
                String.format("%x", hexMatrix[i][2]) + " " + 
                String.format("%x", hexMatrix[i][3])
            );
        }
    }

    public static int[][] getHexMatP(char s, String p) throws IllegalArgumentException {
        int[][] hexMatrix = {};

        try {
            if(p.length() > MAX_PLAINTEXT_LENGTH){
                throw new IllegalArgumentException("Plaintext strings must be less than or equal to sixteen bytes.");
            } else {
                hexMatrix = new int[MATRIX_SIZE][MATRIX_SIZE];
                int curPos;

                for(int j=0;j<MATRIX_SIZE;j++){
                    for(int i=0;i<MATRIX_SIZE;i++){
                        curPos = (MATRIX_SIZE * i) + j;

                        if(curPos > p.length() - 1){
                            hexMatrix[j][i] = s;
                        } else {
                            hexMatrix[j][i] = p.charAt(curPos);
                        }
                    }
                }
            }
        } catch (IllegalArgumentException e){
            System.exit(0);
        } 

        return hexMatrix;
    }
}