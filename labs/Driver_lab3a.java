/**
* file:     Driver_lab3b.java
* author:   John Craig
* course:   MSCS 630
* assignment: Lab 3
* due date: February 3, 2022
* version: 1.0
*
* This file contains the source code for the first
* lab assignment.
*/

/**
* Driver_lab3
*
* This class contains a program which converts one or more lines of text into integers.
*/
public class Driver_lab3b {
    //Declare constants for input
    static final int MAX_NUMBERS_FIRST_LINE = 2;

    /* Main method of the program; accepts lines of text as input and prints
       each line with the characters converted to integers */
    public static void main(String args[]) {
        int numLines = args.length;

        int[] modulusAndSize = parseFirstLine(0);
        int[][] A = parseRemainingLines(args, modulusAndSize[1]);

        int determinant = cofModDet(A, modulusAndSize[0]);

        System.out.println("%d", determinant);
    }

    /* Returns the modulus 'm' and matrix size of 'A' as a two-element integer
       array parsed from a string. Throws an error if the values are not found
       correctly */
    public static int[] parseFirstLine(String line) throws IllegalArgumentException {
        String[] splitLine = line.split(' ');
        int[] digits;

        try {
            if(splitLines.length != MAX_NUMBERS_FIRST_LINE){
                throw new IllegalArgumentException("The first line must contain exactly two numbers.");
            } else {
                digits = new int[2];
                
                digits[0] = Integer.parseInt(splitLine[0]);
                digits[1] = Integer.parseInt(splitLine[1]);
            }
        } catch (IllegalArgumentException e){
            System.exit(0);
        } catch (NumberFormatException e){
            System.exit(0);
        }

        return digits;
    }

    /* Accepts an array containing one line followed by 'size' lines, each containing 'size'
       numbers, and returns a matrix with the numbers parsed as integers */
    public static int[][] parseRemainingLines(String[] lines, int size) throws IllegalArgumentException {
        int[][] matrix = new int[size][size];

        try {
            if(lines.size - 1 != size){
                throw new IllegalArgumentException("There must be %d lines of input following the first.", size);
            } else {
                String[] splitLine;

                for(int i=1;i<size;i++){
                    splitLine = lines[i].split(' ');

                    if(splitLine.length != size){
                        throw new IllegalArgumentException("Each line must contain %d numbers.", size);
                    } else {
                        for(int j=0;j<size;j++){
                            matrix[i-1][j] = Integer.parseInt(splitLine[j]);
                        }
                    }
                }
            }
        } catch (IllegalArgumentException e){
            System.exit(0);
        } catch (NumberFormatException e){
            System.exit(0);
        }

        return matrix;
    }

"""
    Again, we could have been working (mod 9) throughout all intermediate calculations
    and would have obtained the same result.

        -Footnote on pg. 160, Stanoyevitch
"""

    /* Returns the determinant of the square matrix
       'A' under modulo 'm' */
    public static int cofModDet(int m, int[][] A){
        return matrixDet(A) % m;
    }

"""
 1. For n=1, 
        det(A) is A[0][0], since A=[a].

 2. For n=2, 
        det(A) = (A[0][0]*A[1][1]) - (A[0][1] * A[1][0]), where A is a 2-dimensional array of length 2 and depth 2

 3. For n>=3, 
        det(A) = (A[0][0] * det(A[0:][0:])) - (A[0][1] * det(A[0:][0,2:])) + 
                 (A[0][2] * det(A[0:][1,3:])) - ... + ((-1)^(n) * A[0][n-1] * det(A[0][:n-1])
"""

    /* Returns the determinant of the square matrix 'A' */
    public static int matrixDet(int[][] A){
        //Base cases
        if(A.length == 1)
        {
            return A[0][0];
        } else if(A.length == 2)
        {
            return (A[0][0] * A[1][1]) - (A[0][1] * A[1][0]);
        } else //Recursive case
        {
            int determinant = 0;

            for(int j=0;j<A.length;j++){
                determinant += A[0][j] * matrixDet(submatrix(A,0,j)) * (-1 * (i % 2));
            }
        }
    }

    /* Returns a submatrix of the matrix 'A' with the specified
       row and column removed */
    public static int[][] submatrix(int[][] A, int row, int col){
        int[][] A_p = new int[A.length - 1][A.length - 1];

        for(int i=0;i<A.length;i++){
            for(int j=0;j<A[0].length;j++){
                int k = i;
                int l = j;

                //Do not copy over the element if it is in the
                //specified row or column
                if(i != row && j != col){
                    //If the element is passed the specified row or column,
                    //adjust is index in the submatrix
                    if(i > row){
                        k--;
                    }

                    if(j > col){
                        l--;
                    }

                    //Fill in the submatrix
                    A_p[k][l] = A[i][j];
                }
            }
        }

        return A_p;
    }
}