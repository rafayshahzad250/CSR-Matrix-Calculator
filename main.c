// <Your CODE: include library(s)
#include <stdio.h> // standard C library
#include <stdlib.h> // provides functions like atoi(), EXIT_FAILURE, and EXIT_SUCCESS
#include <string.h> // provides string functions like strcmp()
#include "functions.h" // reference header file for function declarations
#include <time.h> // time library needed for cpu time calculations

int main(int argc, char *argv[]) 
{
	// <<Your CODE: Handle the inputs here>

	if (argc < 2 || argc > 5) // check whether a valid amount of arguments have been passed, at least 1 argument are needed as the fewest arguments that can be passed are: "./main" and "file"
	// more than 4 parameters cannot be passed either meaning argc cant be greater than 5
	{
        fprintf(stderr, "Error/Incorrect number of arguments: Please try again with the following format: ./main <file1.mtx> <file2.mtx> <operation> <print option>\n");
        exit(EXIT_FAILURE); // terminate program
    }

	if (argc == 3) // this is an invalid number of arguments that needs to be checked for which is inbetween argc == 2 and argc == 5
	{
		fprintf(stderr, "Error/Incorrect number of arguments: Please try again with the following format: ./main <file1.mtx> <file2.mtx> <operation> <print>\n");
        exit(EXIT_FAILURE); // terminate program 
	}

	const char *filename_1 = argv[1]; // file 1 is the first argument
	CSRMatrix A; // initalize matrix A
	ReadMMtoCSR(filename_1, &A); // read the file and assign it to matix A


	if (argc == 2) // if only the file name is passed print the matrix
	{
		printMatrix(&A); // print matrix A
		printf("\n"); // empty line for spacing
		exit(EXIT_SUCCESS); // indicates the program has completed and terminates, equivalent to "return 0"
	}
	
	// <Your CODE: The rest of your code goes here>

	// code to call the required functions
	if (argc == 4 && (strcmp(argv[2], "transpose") == 0)) // check if the transpose operation should be performed 
	// checks for the correct argument count 
	// strcmp compars the second argument and checks whehter it's equal to "transpose", if yes it returns 0
	{
		// initlialize cpu time to check how long computation takes
		clock_t start_time, end_time;
		double cpu_time_used;
		start_time = clock();

        CSRMatrix A_transpose = transpose(&A); // compute the transpose of A and assign it to the CSRMatrix AT

		// end cpu timer 
		end_time = clock();
		cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

        // Check if print argument is provided
        if (atoi(argv[3]) == 1) // check if the user wants the matriced to be printed
		{
            printf("Matrix A:\n"); 
            printMatrix(&A); // print matrix A
			printf("\n");
            printf("Transpose of A:\n"); 
            printMatrix(&A_transpose); // print matrix AT
			printf("\n");
			printf("CPU time: %f seconds\n", cpu_time_used); // print the cpu time for the operation as we need this to compare the multiplication function with the python implementation
			printf("\n");
        }
		else if (atoi(argv[3]) == 0) // if print option is 0, only pirnt the cpu time
		{
			printf("CPU time: %f seconds\n", cpu_time_used); // print the cpu time for the operation as we need this to compare the multiplication function with the python implementation
			printf("\n");
		}
		
        freeMatrix(&A); // free allocated memory for matix A
        freeMatrix(&A_transpose); // free allocated memory for matix AT
        exit(EXIT_SUCCESS); // indicates the program has completed and terminates, equivalent to "return 0"
    } 
	else if (argc == 5) // handles cases where addition, subtraction or mutliplication need to be performed
					    // checks whether the correct number of arguments have been passed for the other operations
	{
		const char *filename_2 = argv[2]; // file 2 is the second file
		CSRMatrix B; // initialize matrix B
		ReadMMtoCSR(filename_2, &B); // read file 2 and assign it matrix B
		CSRMatrix C; // initialize resultant matrix C

		const char *operation = argv[3]; // assigns the operation pointer to the 3rd passed argument which is the desired opertion

		// initlialize cpu time to check how long computation takes
		clock_t start_time, end_time;
		double cpu_time_used;
		start_time = clock();

			if (strcmp(operation, "addition") == 0) // checks if the operation to be performed is addition
			{
				C = addition(&A, &B); // performs additon and assigns it to the resultant matrix C
			} 
			else if (strcmp(operation, "subtraction") == 0) // checks if the operation to be performed is subtraction
			{
				C = subtraction(&A, &B); // performs subtraction and assigns it to the resultant matrix C
			} 
			else if (strcmp(operation, "multiplication") == 0) // checks if the operation to be performed is multiplication
			{
				C = multiplication(&A, &B); // performs multiplication and assigns it to the resultant matrix C
			} 
			else // safe case for if a typo or something occured and prints the following error
			{
				fprintf(stderr, "Unsupported operation. Please use one of the following: addition, subtraction, multiplication, transpose.\n");
				freeMatrix(&A);
				freeMatrix(&B);
				exit(EXIT_FAILURE); // terminate program
			}

			// end cpu timer 
			end_time = clock();
			cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

			// Lastly we need to check if the print option was indicated to be 1 and print the matrices if yes, otherwise just print cpu time.
			if (atoi(argv[4]) == 1) 
			{
				printf("Matrix A:\n");
				printMatrix(&A);
				printf("\n");
				printf("Matrix B:\n");
				printMatrix(&B);
				printf("\n");
				printf("Resultant Matrix C:\n");
				printMatrix(&C);
				printf("\n");
				printf("CPU time: %f seconds\n", cpu_time_used); // print the cpu time for the operation as we need this to compare the multiplication function with the python implementation
				printf("\n");
			}
			else if (atoi(argv[4]) == 0) // if no print option is given we omit printing matrices and print just the cpu time
			{
				printf("CPU time: %f seconds\n", cpu_time_used); 
				printf("\n");
			}
			

			// We need to make sure to free the allocated memory for matrices A,B, and C
			freeMatrix(&A);
			freeMatrix(&B);
			freeMatrix(&C);

			exit(EXIT_SUCCESS);
		
	} 
	else
	{
		fprintf(stderr, "Error/Incorrect number of arguments: Please try again with the following format: ./main <file1.mtx> <file2.mtx> <operation> <print option>\n");
        exit(EXIT_FAILURE); // terminate program 
	}

}
