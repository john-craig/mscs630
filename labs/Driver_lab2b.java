/**
* file:     Driver_lab2.java
* author:   John Craig
* course:   MSCS 630
* assignment: Lab 2
* due date: January 29, 2022
* verison: 1.0
*
* This file contains the source code for the first
* lab assignment.
*/

/**
* Driver_lab2
*
* This class contains a program which converts one or more lines of text into integers.
*/
public class Driver_lab2b {
    //Declare constants for input
    static final int MAX_NUMBERS = 2;
    static final int MAX_LINES = -1;

    /* Main method of the program; accepts lines of text as input and prints
       each line with the characters converted to integers */
    public static void main(String args[]) {
        int numLines = args.length;

        //No max number of lines to check

        printResults(args);
    }

    /* No output. Prints the greatest common denominator
       of each pair of numbers on each line. */
    public static void printResults(String lines[]){
        //Declare working variables
        long[] digits;
        long denominator = "";

        //Iterate over lines
        for(int i=0;i<lines.length;i++){
            //Convert line to longs
            digits = parseLine(lines[i]);
            //Determine greatest common denominator and coefficients
            results = euclidAlgExt(digits[0], digits[1]);

            //Print output
            System.out.println(
                results[0].toString() + " "
                results[1].toString() + " "
                results[2].toString()
            );
        }
    }


    /* Returns an array of two longs parsed from a string containing
       space-separated integers */
    public static long[] parseLine(line) throws IllegalArgumentException {
        String splitLine = line.split(' ');
        long[] digits;

        try {
            if(splitLines.length != MAX_NUMBERS){
                throw new IllegalArgumentException("Each line must contain exactly two numbers.");
            } else {
                digits = new int[2];
                
                digits[0] = Long.parseLong(splitLine[0]);
                digits[1] = Long.parseLong(splitLine[1]);
            }
        } catch (IllegalArgumentException e){
            System.exit(0);
        } catch (NumberFormatException e){
            System.exit(0);
        }
    }

    /* Returns the greatest common denominator and the coefficients
       necessary to produce this denominator from two digits */
    public static long[] eudclidAlgExt(long a, long b){
        long[] results = new long[3];

        if(b == 0){
            results[0] = a;
            results[1] = 1;
            results[2] = 0;

            return results;
        }

        results = euclidAlgExt(b, a%b);

        results[1] = results[2];
        results[2] = (results[1] - ((a / b) * results[2]));

    return results;

    }

    /* Returns the greatest common denominator of the two digits,
       using Euclid's algorithm */
    public static long euclidAlg(long a, long b){
        //Ensure 'a' is greater than 'b'
        if(a < b){
            long tmp = b;
            b = a;
            a = tmp;
        }

        if(b == 0){
            return a;
        } else {
            return euclidAlg(b, (a % b));
        }
    }
}