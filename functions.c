#include <stdio.h>     // the standard c library
#include "functions.h" // reference the header file with function declarations
#include <stdlib.h>    // provides memory allocation functions
#include <string.h>    // provides string based functions

// NOTE: The code for the addition and subtraction function is identitical to the only two minor sign changes are needed and are reflected by comments saying "CHANGE FROM ADDITION:"
//       The comments are also for the most part the same as the addition function

void ReadMMtoCSR(const char *filename, CSRMatrix *matrix)
{

    FILE *file = fopen(filename, "r"); // open the provided filename in read mode
    if (file == NULL)                  // here the code checks if the file has been successfully opened
    {
        fprintf(stderr, "Error: Failed to open %s\n file", filename);
        exit(EXIT_FAILURE); // terminates program similar with error status similar to "return 1;"

        /* If the file wasn't opened I chode to use the "fprintf, stderr, EXIT_FAILURE" format which we learnt in
        lecture as a more professional way to print custom errors and exit the program*/
    }

    char line[1000]; // initial a character array of size 1000 to act as a buffer for reading each line

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] != '%')
        {
            break; // Skip comments
        }
        /* the function reads a line of size represented by (sizeof(line)) from the "file" and saves it to the temporary
        buffer from before called "line." The if statemement checks if the first character of the line is '%' and skips it
        because since it is a comment it must be ignored in terms of the data collection process */

        // This allows us to skip the lines with comments and now be able to read the lines containing data
    }

    sscanf(line, "%d %d %d", &matrix->num_rows, &matrix->num_cols, &matrix->num_non_zeros);
    // sscanf is a function used to read input from a string in this case the string is the line containing the matrix dimensions and number of entries
    // the function also allows for the input to be saved into specified variables, in this case variables of the struct
    // these values are saved to the variables of each respective variable by passing the address of the matrix struct and pointing to the desired variable

    // Next we need to allocate memory for the three CSR arrays
    matrix->csr_data = (double *)malloc(matrix->num_non_zeros * sizeof(double)); // allocate memory for values array
    // Check to make sure memory allocations have occurred correctly, terminate program if failure
    if (matrix->csr_data == NULL) // check memory allocation for data array
    {
        fprintf(stderr, "Memory allocation failed for csr_data\n");
        fclose(file);       // close file
        exit(EXIT_FAILURE); // terminate program
    }

    matrix->col_ind = (int *)malloc(matrix->num_non_zeros * sizeof(int)); // allocate memory for column indices
    if (matrix->col_ind == NULL)                                          // check memory allocation for column indices array
    {
        fprintf(stderr, "Memory allocation failed for col_ind\n");
        free(matrix->csr_data); // free memory for data array
        fclose(file);
        exit(EXIT_FAILURE);
    }

    matrix->row_ptr = (int *)calloc(matrix->num_rows + 1, sizeof(int)); // allocate memory for row pointers array
    if (matrix->row_ptr == NULL)                                        // check memory allocation for row pointer array
    {
        fprintf(stderr, "Memory allocation failed for row_ptr\n");
        free(matrix->csr_data); // free memory for data array
        free(matrix->col_ind);  // free memory for column indices array
        fclose(file);
        exit(EXIT_FAILURE);
    }

    int row;      // initialized row variable to store the row index of a non-zero element from the .mtx file
    int column;   // initialized column variable to store the column index of a non-zero element from the .mtx file
    double value; // initialized value variable to hold the value of a non-zero element from the .mtx file
    while (fscanf(file, "%d %d %lf", &row, &column, &value) == 3)
    // reads a line from the file and stores the number into thr row, col, and value variables
    // the "==3" checks whether exactly 3 values (row index, column index, and value) were read
    {
        row--;
        // Matrix market begins indexing from 1 and the C programming language begins indexing from 0 so to resolve this issue, row is decremented by 1 to use 0-based indexing
        matrix->row_ptr[row + 1]++;
        // increments the element at row + 1 in the row_ptr array.
    }

    for (int i = 1; i <= matrix->num_rows; i++) // loop through all rows with incrementing starting at 1 because the first element (0th index) is already set to be 0
    {
        matrix->row_ptr[i] += matrix->row_ptr[i - 1]; // add the previous element to the current element in row_ptr array to accumulate and set up the array to hold hold the starting index of row i in the data and column indices arrays
    }

    // We have now processed the row_ptr array and captured the matrix dimensions however we have reached the end of the file and now need to reread the file
    // We need to reread the file and make another pass in order to process and store the values and column indices in the csr_data and col_ind arrays
    // the fseek function effectively allows us to reset the "file" pointer back to the beginning of the file, SEEK_SET represents the beginning of the file and 0 represents that we want dont want to offset  from the beginning of the file
    fseek(file, 0, SEEK_SET);

    // this is the same block we had earlier in the function, we are once again just skipping any lines of the file which contain comments
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] != '%')
            break;
    }

    /* Here we declare and intialize a temporay integer  array called temp_row_ptr which basically is is used to help keep track of the
    indices in the CSR format while reading data from the .mtx file. The memset function basically copies the data from the row_ptr array and assigns it
    to the temp_row_ptr array. A detailed explanation of the memset function is provided in the addition and subtraction functions. By doing this we can now
    safely increment the values of the temp_row_ptr array without endangering the integrity of the original row_ptr array. This helps with filling the column indices
    and values arrays, it's usage is seen farther down */
    int *temporary_row_ptr = (int *)malloc((matrix->num_rows + 1) * sizeof(int));
    memcpy(temporary_row_ptr, matrix->row_ptr, (matrix->num_rows + 1) * sizeof(int));

    // while loop goes line by line reading 3 values and assigning them to the row, col, and value variables
    // the "== 3" ensures the loop will stop once there is no more lines of data to be read
    while (fscanf(file, "%d %d %lf", &row, &column, &value) == 3)
    {
        row--; // once again, as previosuly explained we need to convert to 0 based indexing for both our rows and columns
        column--;
        int index = temporary_row_ptr[row]++; // temp_row_ptr[row] gives the current index in the csr_data and col_ind arrays for the row
                                              // it is post incremented so that after obtaining the index the next non-zero element for the row will be placed in the next position
        matrix->csr_data[index] = value;      // assigns the value read from the file to the csr_data array at the cooresponding index
        matrix->col_ind[index] = column;      // assigns the column index read from the file to the col_ind array at the cooresponding index
    }

    free(temporary_row_ptr); // we need to make sure to free up temporary memory for the temporary array we initialized as it is no longer needed
    fclose(file);            // make sure to close the file when done
}

CSRMatrix addition(const CSRMatrix *A, const CSRMatrix *B)
{
    // Check to see if dimensions are comaptible, both matrices must have the same number of rows and columns
    if (A->num_rows != B->num_rows || A->num_cols != B->num_cols)
    {
        fprintf(stderr, "Error: Incompatible Dimensions, please try again.\n");
        exit(EXIT_FAILURE);
    }

    CSRMatrix C;                                            // initialize a new CSR matrix to hold the results of the computation
    C.num_rows = A->num_rows;                               // should have the same number of rows as both A and B
    C.num_cols = A->num_cols;                               // should have the same number of rows as both A and B
    C.row_ptr = (int *)calloc(C.num_rows + 1, sizeof(int)); // initialize the row_ptr array to adhere to the CSR format of number of rows + 1
    if (C.row_ptr == NULL)                                  // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for csr_data.\n");
        exit(EXIT_FAILURE);
    }

    // initialize some variables which hold temporary data prior to computation
    int temp_non_zeros = A->num_non_zeros + B->num_non_zeros; // this is a temporary amount of non-zero entries for C which is essentially the maximum amount it could have

    C.csr_data = (double *)malloc(temp_non_zeros * sizeof(double)); // allocate memory for the csr_data array in the case of the maximum amount of non-zero entries
    if (C.csr_data == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for csr_data.\n");
        free(C.row_ptr); // Free previously allocated memory for row pointer array
        exit(EXIT_FAILURE);
    }

    C.col_ind = (int *)malloc(temp_non_zeros * sizeof(int)); // allocate memory for the col_ind array in the case of the maximum amount of non-zero entries
    if (C.col_ind == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for col_ind.\n");
        free(C.row_ptr);  // Free previously allocated memory for row pointer array
        free(C.csr_data); // Free previously allocated memory for values array
        exit(EXIT_FAILURE);
    }

    // temprary variables for helping with tracking in computation
    int *column_marker = (int *)malloc(C.num_cols * sizeof(int)); // initialized an array called "column_marker" to keep track of columns that have been processed, explained below with "memset"
    if (column_marker == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for column_marker.\n");
        free(C.row_ptr);  // Free previously allocated memory for row pointer array
        free(C.csr_data); // Free previously allocated memory for values array
        free(C.col_ind);  // Free previously allocated memory for column indixes array
        exit(EXIT_FAILURE);
    }

    memset(column_marker, -1, C.num_cols * sizeof(int));
    /* The memset function takes a pointer to a memory block (in this case column marker), an integer (in this case -1) and
    the size of bytes (in this case the size of integer times the number of columns). What it does is it fills the memory block with
    the passed value which in this case is -1. So what it does in this context is that it sets each byte of the memory block pointed to by
    column_marker to -1. The reason we do this is by initializing to this -1 value which is not a valid index, during computation
    we can mark and track the columns to see if they have been processed or not. For example if "column_marker[col] != -1" then the column
    was processed and the value can be updated. If column_marker[col] == -1 it means the column has not been processed and must
    be accounted for and added to the resultant matrix. */

    int num_non_zeros_C = 0; // initalized the number of non-zero entries for the resultant matrix to be C, this will be the actual variable used at the end opposed to the temporary one

    for (int i = 0; i < C.num_rows; i++) // iterate through the number of rows in the matriced
    {
        C.row_ptr[i] = num_non_zeros_C; // set the row pointer for resultant matirx C to the current num_non_zeros_C value to mark the start of non-zero elements in C.

        // nested loop to process the non-zero elements of matrix A
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) // iterate through the non-zero elements in the current row of matrix A
        {
            int col_index = A->col_ind[j];                // retrieves and assigns the column index of the current non-zero element to "col_index"
            C.csr_data[num_non_zeros_C] = A->csr_data[j]; // copy the non-zero value from A to C
            C.col_ind[num_non_zeros_C] = col_index;       // sets the column index in C to the column index cooresponding to the current non-zero entry
            column_marker[col_index] = num_non_zeros_C;   // mark the cooresponding column to reflect that it has been processed by setting it equal to num_non_zeros_C
            num_non_zeros_C++;                            // increment the non zero counter by 1
        }

        // nested loop to process the non-zero elements of matrix B
        for (int j = B->row_ptr[i]; j < B->row_ptr[i + 1]; j++)
        {
            int col_index = B->col_ind[j];      // retrieves and assigns the column index of the current non-zero element to "col_index"
            if (column_marker[col_index] != -1) // checks whether the column has already been process by matrix A through referencing its column marker
            {
                C.csr_data[column_marker[col_index]] += B->csr_data[j];
                // if the column has already been marked/processed then it adds the cooreponding value in matrix B from the already assigned value in matrix C
                // this is effectively performing addition on cooresponding entries
            }
            else // if the column has not been processed/marked the following block is entered
            {
                C.csr_data[num_non_zeros_C] = B->csr_data[j]; // assigns the value of the cooresponding entry in matrix B to matrix C (same as doing adding the entry in B to 0)
                C.col_ind[num_non_zeros_C] = col_index;       // sets the column index in C to the column index cooresponding to the current non-zero entry
                column_marker[col_index] = num_non_zeros_C;   // mark the cooresponding column to reflect that it has been processed by setting it equal to num_non_zeros_C
                num_non_zeros_C++;                            // increment the non zero counter by 1
            }
        }

        /* Now something very important that needs to be done is the column markers for each column need to be reset to -1.
        The reason for this is that when the computation is performed for the next row they need to be reset as the same columns
        may need to accessed or not and we need to know whether the columns have been processed or not for each cooresponding row
        iteration.*/

        // reset column markers for matrix A
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) // loops from the starting index of non-zero elements to the ending index of non-zero elements
        {
            int col_index = A->col_ind[j]; // retrieves the column index of the current non-zero element
            column_marker[col_index] = -1; // resets the column marker for the cooresponding column_index back to -1
        }

        // reset column markers for matrix B
        for (int j = B->row_ptr[i]; j < B->row_ptr[i + 1]; j++) // loops from the starting index of non-zero elements to the ending index of non-zero elements
        {
            int col_index = B->col_ind[j]; // retrieves the column index of the current non-zero element
            column_marker[col_index] = -1; // resets the column marker for the cooresponding column_index back to -1
        }
    }

    C.row_ptr[C.num_rows] = num_non_zeros_C; // finalize the row_ptr array by setting its last element to the number of non-zero elements to adhere to CSR format

    /* An issue I was coming across when testing the functions was that my results were printing some 0's in the values array which is incorrect as it is only
    supposed to include non-zero elements. It also led to there being an extra number of elements in the number of non-zero elements counter. To fix this I decided a solution would be
    that after completing the computation, revise and update the arrays and variables to remove these 0's and display the correct output */

    // To start I declared and allocated memory for new arrays which will represent the filtered versions of the values, column indices, and row pointer arrays
    double *filtered_csr_data = (double *)malloc(num_non_zeros_C * sizeof(double));
    if (filtered_csr_data == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_csr_data.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        exit(EXIT_FAILURE);
    }

    int *filtered_col_ind = (int *)malloc(num_non_zeros_C * sizeof(int));
    if (filtered_col_ind == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_col_ind.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        free(filtered_csr_data);
        exit(EXIT_FAILURE);
    }

    int *filtered_row_ptr = (int *)calloc(C.num_rows + 1, sizeof(int));
    if (filtered_col_ind == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_row_ptr.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        free(filtered_csr_data);
        free(filtered_col_ind);
        exit(EXIT_FAILURE);
    }

    int filtered_num_non_zeros = 0;      // temporarily set the filtered number of non zero elements to 0
    for (int i = 0; i < C.num_rows; i++) // iterate over the rows in C
    {
        filtered_row_ptr[i] = filtered_num_non_zeros; // set the starting index for the filtered row pointers array, similar logic as earlier in the code

        for (int j = C.row_ptr[i]; j < C.row_ptr[i + 1]; j++) // iterate over the non-zero elements in the cooresponding row
        {
            if (C.csr_data[j] != 0) // check if the value of the current element is not equal to zero
            {
                // if it is a nonzero element
                filtered_csr_data[filtered_num_non_zeros] = C.csr_data[j]; // copy it to the filtered data array at the index cooresponding to the filtered non-zero elements counter
                filtered_col_ind[filtered_num_non_zeros] = C.col_ind[j];   // copy the cooresponding column index to the filtered colum indices array at the same index
                filtered_num_non_zeros++;                                  // increment counter after each element
            }
        }
    }

    filtered_row_ptr[C.num_rows] = filtered_num_non_zeros; // finalize the row pinter array by assigning the last element to the number of non-zero elements

    // Now we need to update the values, row pointer, and column indices arrays to the filtered versions but before that we need to free their previous memory
    free(C.csr_data);
    free(C.col_ind);
    free(C.row_ptr);

    C.num_non_zeros = filtered_num_non_zeros; // update number of non zero elements counter
    C.csr_data = filtered_csr_data;           // update data array
    C.col_ind = filtered_col_ind;             // update column indices array
    C.row_ptr = filtered_row_ptr;             // update row pointers array

    // If everything is allocated successfully we still need to make sure to free up any temporary memory which is no longer needed once all the computation is completed
    free(column_marker);

    return C; // returns the resultant matrix C
}

CSRMatrix subtraction(const CSRMatrix *A, const CSRMatrix *B)
{
    // Check to see if dimensions are comaptible, both matrices must have the same number of rows and columns
    if (A->num_rows != B->num_rows || A->num_cols != B->num_cols)
    {
        fprintf(stderr, "Error: Incompatible Dimensions, please try again.\n");
        exit(EXIT_FAILURE);
    }

    CSRMatrix C;                                            // initialize a new CSR matrix to hold the results of the computation
    C.num_rows = A->num_rows;                               // should have the same number of rows as both A and B
    C.num_cols = A->num_cols;                               // should have the same number of rows as both A and B
    C.row_ptr = (int *)calloc(C.num_rows + 1, sizeof(int)); // initialize the row_ptr array to adhere to the CSR format of number of rows + 1
    if (C.row_ptr == NULL)                                  // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for csr_data.\n");
        exit(EXIT_FAILURE);
    }

    // initialize some variables which hold temporary data prior to computation
    int temp_non_zeros = A->num_non_zeros + B->num_non_zeros; // this is a temporary amount of non-zero entries for C which is essentially the maximum amount it could have

    C.csr_data = (double *)malloc(temp_non_zeros * sizeof(double)); // allocate memory for the csr_data array in the case of the maximum amount of non-zero entries
    if (C.csr_data == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for csr_data.\n");
        free(C.row_ptr); // Free previously allocated memory for row pointer array
        exit(EXIT_FAILURE);
    }

    C.col_ind = (int *)malloc(temp_non_zeros * sizeof(int)); // allocate memory for the col_ind array in the case of the maximum amount of non-zero entries
    if (C.col_ind == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for col_ind.\n");
        free(C.row_ptr);  // Free previously allocated memory for row pointer array
        free(C.csr_data); // Free previously allocated memory for values array
        exit(EXIT_FAILURE);
    }

    // temprary variables for helping with tracking in computation
    int *column_marker = (int *)malloc(C.num_cols * sizeof(int)); // initialized an array called "column_marker" to keep track of columns that have been processed, explained below with "memset"
    if (column_marker == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for column_marker.\n");
        free(C.row_ptr);  // Free previously allocated memory for row pointer array
        free(C.csr_data); // Free previously allocated memory for values array
        free(C.col_ind);  // Free previously allocated memory for column indixes array
        exit(EXIT_FAILURE);
    }

    memset(column_marker, -1, C.num_cols * sizeof(int));
    /* The memset function takes a pointer to a memory block (in this case column marker), an integer (in this case -1) and
    the size of bytes (in this case the size of integer times the number of columns). What it does is it fills the memory block with
    the passed value which in this case is -1. So what it does in this context is that it sets each byte of the memory block pointed to by
    column_marker to -1. The reason we do this is by initializing to this -1 value which is not a valid index, during computation
    we can mark and track the columns to see if they have been processed or not. For example if "column_marker[col] != -1" then the column
    was processed and the value can be updated. If column_marker[col] == -1 it means the column has not been processed and must
    be accounted for and added to the resultant matrix. */

    int num_non_zeros_C = 0; // initalized the number of non-zero entries for the resultant matrix to be C, this will be the actual variable used at the end opposed to the temporary one

    for (int i = 0; i < C.num_rows; i++) // iterate through the number of rows in the matriced
    {
        C.row_ptr[i] = num_non_zeros_C; // set the row pointer for resultant matirx C to the current num_non_zeros_C value to mark the start of non-zero elements in C.

        // nested loop to process the non-zero elements of matrix A
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) // iterate through the non-zero elements in the current row of matrix A
        {
            int col_index = A->col_ind[j];                // retrieves and assigns the column index of the current non-zero element to "col_index"
            C.csr_data[num_non_zeros_C] = A->csr_data[j]; // copy the non-zero value from A to C
            C.col_ind[num_non_zeros_C] = col_index;       // sets the column index in C to the column index cooresponding to the current non-zero entry
            column_marker[col_index] = num_non_zeros_C;   // mark the cooresponding column to reflect that it has been processed by setting it equal to num_non_zeros_C
            num_non_zeros_C++;                            // increment the non zero counter by 1
        }

        // nested loop to process the non-zero elements of matrix B
        for (int j = B->row_ptr[i]; j < B->row_ptr[i + 1]; j++)
        {
            int col_index = B->col_ind[j];      // retrieves and assigns the column index of the current non-zero element to "col_index"
            if (column_marker[col_index] != -1) // checks whether the column has already been process by matrix A through referencing its column marker
            {
                // CHANGE FROM ADDITION
                C.csr_data[column_marker[col_index]] -= B->csr_data[j];
                // if the column has already been marked/processed then it subtracts the cooreponding value in matrix B from the already assigned value in matrix C
                // this is effectively performing subtraction on cooresponding entries
            }
            else // if the column has not been processed/marked the following block is entered
            {
                // CHANGE FROM ADDITION:
                C.csr_data[num_non_zeros_C] = -(B->csr_data[j]); // assigns the negative value of the cooresponding entry in matrix B to matrix C (same as doing subtractng the entry in B from 0)
                C.col_ind[num_non_zeros_C] = col_index;          // sets the column index in C to the column index cooresponding to the current non-zero entry
                column_marker[col_index] = num_non_zeros_C;      // mark the cooresponding column to reflect that it has been processed by setting it equal to num_non_zeros_C
                num_non_zeros_C++;                               // increment the non zero counter by 1
            }
        }

        /* Now something very important that needs to be done is the column markers for each column need to be reset to -1.
        The reason for this is that when the computation is performed for the next row they need to be reset as the same columns
        may need to accessed or not and we need to know whether the columns have been processed or not for each cooresponding row
        iteration.*/

        // reset column markers for matrix A
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) // loops from the starting index of non-zero elements to the ending index of non-zero elements
        {
            int col_index = A->col_ind[j]; // retrieves the column index of the current non-zero element
            column_marker[col_index] = -1; // resets the column marker for the cooresponding column_index back to -1
        }

        // reset column markers for matrix B
        for (int j = B->row_ptr[i]; j < B->row_ptr[i + 1]; j++) // loops from the starting index of non-zero elements to the ending index of non-zero elements
        {
            int col_index = B->col_ind[j]; // retrieves the column index of the current non-zero element
            column_marker[col_index] = -1; // resets the column marker for the cooresponding column_index back to -1
        }
    }

    C.row_ptr[C.num_rows] = num_non_zeros_C; // finalize the row_ptr array by setting its last element to the number of non-zero elements to adhere to CSR format

    /* An issue I was coming across when testing the functions was that my results were printing some 0's in the values array which is incorrect as it is only
    supposed to include non-zero elements. It also led to there being an extra number of elements in the number of non-zero elements counter. To fix this I decided a solution would be
    that after completing the computation, revise and update the arrays and variables to remove these 0's and display the correct output */

    // To start I declared and allocated memory for new arrays which will represent the filtered versions of the values, column indices, and row pointer arrays
    double *filtered_csr_data = (double *)malloc(num_non_zeros_C * sizeof(double));
    if (filtered_csr_data == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_csr_data.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        exit(EXIT_FAILURE);
    }

    int *filtered_col_ind = (int *)malloc(num_non_zeros_C * sizeof(int));
    if (filtered_col_ind == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_col_ind.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        free(filtered_csr_data);
        exit(EXIT_FAILURE);
    }

    int *filtered_row_ptr = (int *)calloc(C.num_rows + 1, sizeof(int));
    if (filtered_col_ind == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_row_ptr.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        free(filtered_csr_data);
        free(filtered_col_ind);
        exit(EXIT_FAILURE);
    }

    int filtered_num_non_zeros = 0;      // temporarily set the filtered number of non zero elements to 0
    for (int i = 0; i < C.num_rows; i++) // iterate over the rows in C
    {
        filtered_row_ptr[i] = filtered_num_non_zeros; // set the starting index for the filtered row pointers array, similar logic as earlier in the code

        for (int j = C.row_ptr[i]; j < C.row_ptr[i + 1]; j++) // iterate over the non-zero elements in the cooresponding row
        {
            if (C.csr_data[j] != 0) // check if the value of the current element is not equal to zero
            {
                // if it is a nonzero element
                filtered_csr_data[filtered_num_non_zeros] = C.csr_data[j]; // copy it to the filtered data array at the index cooresponding to the filtered non-zero elements counter
                filtered_col_ind[filtered_num_non_zeros] = C.col_ind[j];   // copy the cooresponding column index to the filtered colum indices array at the same index
                filtered_num_non_zeros++;                                  // increment counter after each element
            }
        }
    }

    filtered_row_ptr[C.num_rows] = filtered_num_non_zeros; // finalize the row pinter array by assigning the last element to the number of non-zero elements

    // Now we need to update the values, row pointer, and column indices arrays to the filtered versions but before that we need to free their previous memory
    free(C.csr_data);
    free(C.col_ind);
    free(C.row_ptr);

    C.csr_data = filtered_csr_data;           // update data array
    C.col_ind = filtered_col_ind;             // update column indices array
    C.row_ptr = filtered_row_ptr;             // update row pointers array
    C.num_non_zeros = filtered_num_non_zeros; // update number of non zero elements counter

    // If everything is allocated successfully we still need to make sure to free up any temporary memory which is no longer needed once all the computation is completed
    free(column_marker);

    return C; // returns the resultant matrix C
}

CSRMatrix multiplication(const CSRMatrix *A, const CSRMatrix *B)
{
    // prior to any computation we need to see if the passed matrices have compatible dimensions for multiplication
    // If A is N1xM1 and B is N2xM2 then M1 must equal N2 and the resultant matrix C will have dimensions N1xM2
    if (A->num_cols != B->num_rows)
    {
        fprintf(stderr, "Error: Incompatible dimensions, please try again.\n");
        exit(EXIT_FAILURE); // terminate program and prompt the error message if dimensions are incompatible
    }

    CSRMatrix C;                                            // initialize the resultant matrix C which will contain the completed computation of the matrix multiplication
    C.num_rows = A->num_rows;                               // C takes the row dimension of A
    C.num_cols = B->num_cols;                               // C takes the col dimension of B
    C.row_ptr = (int *)calloc(C.num_rows + 1, sizeof(int)); // allocate memory for the row pointer array using calloc and adhere to CSR format so it's length is the number of rows + 1
    if (C.row_ptr == NULL)                                  // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for row_ptr.\n");
        exit(EXIT_FAILURE);
    }

    int max_non_zeros = 100000000;
    // set a temporary rough bound for the maximum amount of non-zero elements that can be in C, I chose a huge number because some of the large matrices have very large amounts of non-zero elements
    C.csr_data = (double *)malloc(max_non_zeros * sizeof(double)); // allocate memory in the data array to the size of the amount of the temporary max non-zero elements. These are temporary measures to ensure we have enough memory, the correct amount is reallocated at the end.
    if (C.csr_data == NULL)                                        // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for csr_data.\n");
        free(C.row_ptr); // free previously allocated memory for the row pointers array
        exit(EXIT_FAILURE);
    }

    // similarly allocate a temporary amount of memory for the column indices array according to the temorary max non-zero elements
    C.col_ind = (int *)malloc(max_non_zeros * sizeof(int));
    if (C.col_ind == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for col_ind.\n");
        free(C.row_ptr);  // free previously allocated memory for the row pointers array
        free(C.csr_data); // free previously allocated memory for the values/data array
        exit(EXIT_FAILURE);
    }

    // initalized column marker array
    /* The column marker array serves the same purpose as the ones in the addition and subtraction functions. The logic is explained
    in detail in those functions but essentially the column marker array helps tracks which columns have been processed and is essential
    for completing the computation correctly*/
    int *column_marker = (int *)malloc(C.num_cols * sizeof(int));
    if (column_marker == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for column_marker.\n");
        free(C.row_ptr);  // free previously allocated memory for the row pointers array
        free(C.csr_data); // free previously allocated memory for the values/data array
        free(C.col_ind);  // free previously allocated memory for the column indices array
        exit(EXIT_FAILURE);
    }

    memset(column_marker, -1, C.num_cols * sizeof(int));
    // set all the bytes of the memory block pointed to by column_marker to -1, full detailed explanation is included in addition and subtraction functions
    // -1 is not a valid index number and represents the column has yet to be processed

    int num_non_zeros_C = 0;              // initalized the number of non-zero entries for the resultant matrix to be C, this will be the actual variable used at the end opposed to the temporary one
    for (int i = 0; i < A->num_rows; i++) // iterate through each row of A
    {
        C.row_ptr[i] = num_non_zeros_C; // initialize the row pointer for the current row in resultant matrix C

        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) // iterate over the non-zero elements in the current row of matrix A
        {
            int a_col_index = A->col_ind[j]; // initialized variable a_col_index to hold column index of the current non-zero element in A
            double a_val = A->csr_data[j];   // initialized variable a_val to hold the value of the current non-zero element in A

            for (int k = B->row_ptr[a_col_index]; k < B->row_ptr[a_col_index + 1]; k++) // iterate over the non-zero elements in B cooresponding to the column index of the current non-zero element in A
            {
                int b_col_index = B->col_ind[k]; // initialized variable b_col_index to hold column index of the current non-zero element in b
                double b_val = B->csr_data[k];   // initialized variable b_val to hold the value of the current non-zero element in b

                if (column_marker[b_col_index] < C.row_ptr[i]) // conditional check of the column marker so see if the current column b has been processed for resultant matrix C
                {
                    column_marker[b_col_index] = num_non_zeros_C; // mark the column as processed by setting it equal to the current number of non-zero elements
                    C.col_ind[num_non_zeros_C] = b_col_index;     // sets the column index in C to the column index cooresponding to the current non-zero entry in B
                    C.csr_data[num_non_zeros_C] = a_val * b_val;  // stores the value of the products of the A and B values in the values array for C
                    num_non_zeros_C++;
                }
                else
                {
                    C.csr_data[column_marker[b_col_index]] += a_val * b_val; // if the column was already processed then accumulate the products in the cooresponding position in C
                }
            }
        }

        // Just like we did in the addition and subtrcation functions we need to reset the column markers after each row iteration to ensure there is no carryover and fresh column markers can be accessed for the next iteration
        // a more detailed explanation is available in the addition and subtraction functions
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) // iterate through the non-zero elements in the current row of A
        {
            int a_col_index = A->col_ind[j]; // retrieve the column index of the non-zero element in the ith row of A

            for (int k = B->row_ptr[a_col_index]; k < B->row_ptr[a_col_index + 1]; k++) // loop through the cooresponding columns in the a_col_indexth row of B
            {
                int b_col_index = B->col_ind[k]; // retrieves the column index of the current non-zero elements in B
                column_marker[b_col_index] = -1; // resets the column marker at the cooresponding index to -1
            }
        }
    }

    C.row_ptr[C.num_rows] = num_non_zeros_C; // finalize the row_ptr array by setting its last element to the number of non-zero elements to adhere to CSR format

    /* An issue I was coming across when testing the functions was that my results were printing some 0's in the values array which is incorrect as it is only
    supposed to include non-zero elements. It also led to there being an extra number of elements in the number of non-zero elements counter. To fix this I decided a solution would be
    that after completing the computation, revise and update the arrays and variables to remove these 0's and display the correct output */

    // To start I declared and allocated memory for new arrays which will represent the filtered versions of the values, column indices, and row pointer arrays
    double *filtered_csr_data = (double *)malloc(num_non_zeros_C * sizeof(double));
    if (filtered_csr_data == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_csr_data.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        exit(EXIT_FAILURE);
    }

    int *filtered_col_ind = (int *)malloc(num_non_zeros_C * sizeof(int));
    if (filtered_col_ind == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_col_ind.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        free(filtered_csr_data);
        exit(EXIT_FAILURE);
    }

    int *filtered_row_ptr = (int *)calloc(C.num_rows + 1, sizeof(int));
    if (filtered_col_ind == NULL) // memory allocation failure check
    {
        fprintf(stderr, "Error: Memory allocation failed for filtered_row_ptr.\n");
        free(C.row_ptr);
        free(C.csr_data);
        free(C.col_ind);
        free(column_marker);
        free(filtered_csr_data);
        free(filtered_col_ind);
        exit(EXIT_FAILURE);
    }

    int filtered_num_non_zeros = 0;      // temporarily set the filtered number of non zero elements to 0
    for (int i = 0; i < C.num_rows; i++) // iterate over the rows in C
    {
        filtered_row_ptr[i] = filtered_num_non_zeros; // set the starting index for the filtered row pointers array, similar logic as earlier in the code

        for (int j = C.row_ptr[i]; j < C.row_ptr[i + 1]; j++) // iterate over the non-zero elements in the cooresponding row
        {
            if (C.csr_data[j] != 0) // check if the value of the current element is not equal to zero
            {
                // if it is a nonzero element
                filtered_csr_data[filtered_num_non_zeros] = C.csr_data[j]; // copy it to the filtered data array at the index cooresponding to the filtered non-zero elements counter
                filtered_col_ind[filtered_num_non_zeros] = C.col_ind[j];   // copy the cooresponding column index to the filtered colum indices array at the same index
                filtered_num_non_zeros++;                                  // increment counter after each element
            }
        }
    }

    filtered_row_ptr[C.num_rows] = filtered_num_non_zeros; // finalize the row pinter array by assigning the last element to the number of non-zero elements

    // Now we need to update the values, row pointer, and column indices arrays to the filtered versions but before that we need to free their previous memory
    free(C.csr_data);
    free(C.col_ind);
    free(C.row_ptr);

    C.csr_data = filtered_csr_data;           // update data array
    C.col_ind = filtered_col_ind;             // update column indices array
    C.row_ptr = filtered_row_ptr;             // update row pointers array
    C.num_non_zeros = filtered_num_non_zeros; // update number of non zero elements counter

    // If everything is allocated successfully we still need to make sure to free up any temporary memory which is no longer needed once all the computation is completed
    free(column_marker);

    return C; // returns the resultant matrix C where C = A * B
}

CSRMatrix transpose(const CSRMatrix *A)
{
    CSRMatrix A_transpose;                        // initialize a new CSR matix A transpose to represent the transposed version of matrix A
    A_transpose.num_rows = A->num_cols;           // When taking the transpose the number of rows in A^T becomes the number of columns in A
    A_transpose.num_cols = A->num_rows;           // When taking the transpose the number of columns in A^T becomes the number of rows in A
    A_transpose.num_non_zeros = A->num_non_zeros; // When taking the transpose the number of non-zero elements is the same as before

    // Next let's allocate memory for the row pointer, column indices and values arrays while also checking for memory allocation failure each time
    // If memory allocation fails, its important to free up any previously allocated memory
    A_transpose.row_ptr = (int *)calloc(A_transpose.num_rows + 1, sizeof(int)); // size adheres to CSR format
    if (A_transpose.row_ptr == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for row_ptr.\n");
        exit(EXIT_FAILURE);
    }

    A_transpose.col_ind = (int *)malloc(A_transpose.num_non_zeros * sizeof(int)); // size is dependen on the number of non-zero entries
    if (A_transpose.col_ind == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for col_ind.\n");
        free(A_transpose.row_ptr); // We previously allocated memory for the row pointer array so we should free it if the column indices memory allocation fails
        exit(EXIT_FAILURE);
    }

    A_transpose.csr_data = (double *)malloc(A_transpose.num_non_zeros * sizeof(double)); // once again size depends on the number of non-zero entries
    if (A_transpose.csr_data == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for csr_data.\n");
        free(A_transpose.row_ptr); // free up previously allocated row pointer array memory
        free(A_transpose.col_ind); // free up previously allocated column indices array memory
        exit(EXIT_FAILURE);
    }

    int *row_counts = (int *)calloc(A_transpose.num_rows, sizeof(int));
    /* Here a temporary integer array called row_counts is initialized and allocated memory for. This array holds the count
    of non-zero entries for each row in the transposed matrix. */

    if (row_counts == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for row_counts.\n");
        free(A_transpose.row_ptr);  // free up previously allocated row pointer array memory
        free(A_transpose.col_ind);  // free up previously allocated column indices array memory
        free(A_transpose.csr_data); // free up previously allocated values array memory
        exit(EXIT_FAILURE);
    }

    /* Since the transposed matrix swaps rows and columns, we loop through all the non-zero entries and increment
    the cooresponding index in row_counts based off the column index of the non-zero element. This is important as it will
    help with setting up the row pointer array which is done farther down in the code */
    for (int i = 0; i < A->num_non_zeros; i++)
    {
        row_counts[A->col_ind[i]]++;
    }

    // Set up the row_ptr array for the transposed matrix
    for (int i = 1; i <= A_transpose.num_rows; i++) // iterate over the number of rows, starting from 1 because the first element of row_ptr is always 0
    {
        A_transpose.row_ptr[i] = A_transpose.row_ptr[i - 1] + row_counts[i - 1];
        /* For the ith row this line calculates the row_ptr value at i which signifies the starting index of the row in the column indices
        and values arrays. It does this by setting row_ptr[i] equal to the cumulative sum fo the previous value in row_ptr and the count of non-zero
        entries in the previous row */
    }

    /* Initialized and alocated memory for another temporary array called current position. This array is very important as it
    helps in filling out the column indices and values arrays of A^T. What it does is keep track of the position in A^T and then incremented to
    the next available position as we iterate through all the entries. It ensures that the transpose is correctly computed and it's usage is seen farther below. */
    int *current_position = (int *)calloc(A_transpose.num_rows, sizeof(int));
    if (current_position == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for current_position.\n");
        free(A_transpose.row_ptr);  // free up previously allocated row pointer array memory
        free(A_transpose.col_ind);  // free up previously allocated column indices array memory
        free(A_transpose.csr_data); // free up previously allocated values array memory
        free(row_counts);           // free up previously allocated row counts array memory
        exit(EXIT_FAILURE);
    }

    // To complete the transpose operation the below code fills the values and column indices arrays
    for (int row = 0; row < A->num_rows; row++) // iterate through each row of A
    {
        for (int j = A->row_ptr[row]; j < A->row_ptr[row + 1]; j++)
        // iterate through each non-zero element in A by starting at the starting index of non-zero elements in the current row and ending at ending index of non-zero elements in the current row
        {
            int col_index = A->col_ind[j];                                               // initialized variable "col_index" and assigned it column index of the current non-zero element in A
            int dest_pos = A_transpose.row_ptr[col_index] + current_position[col_index]; // initialized variable dest_pos to calculate where the current non-zero element should be placed in A^T
            // A_transpose.row_ptr[col_index] indicates the first element of each column in A^T then we add current_position[col_index] which starts at 0 and is incremented for each element added to the column
            // this ensures we do not overwrite previous elements in the column
            A_transpose.col_ind[dest_pos] = row;             // assign the row index of A to the column indices of A^T at the destination position index for the current non-zero element
            A_transpose.csr_data[dest_pos] = A->csr_data[j]; // assign the current non-zero element from the values array of A to the values array of A^T at the destination position index
            current_position[col_index]++;                   // increment the current_position variable for the current column in A^T
        }
    }

    // Make sure to free up memory by freeing allocated memory for temporary arrays
    free(row_counts);
    free(current_position);

    return A_transpose; // return the transposed matrix A^T
}

void printMatrix(const CSRMatrix *matrix)
{
    printf("Number of non-zeros: %d\n", matrix->num_non_zeros); // print the number of non-zero elements
    printf("Row Pointer: ");                                    // print the row pointers
    for (int i = 0; i <= matrix->num_rows; i++)
    {
        printf("%d ", matrix->row_ptr[i]);
    }
    printf("\n");

    printf("Column Index: "); // print the column indices
    for (int i = 0; i < matrix->num_non_zeros; i++)
    {
        printf("%d ", matrix->col_ind[i]);
    }
    printf("\n");

    printf("Values: "); // print the values of the
    for (int i = 0; i < matrix->num_non_zeros; i++)
    {
        printf("%0.4lf ", matrix->csr_data[i]);
    }
    printf("\n");
}

void freeMatrix(CSRMatrix *matrix)
{
    if (matrix->csr_data != NULL) // Free the allocated memory for csr_data if it isn't empty
    {
        free(matrix->csr_data);
        matrix->csr_data = NULL;
    }
    if (matrix->col_ind != NULL) // Free the allocated memory for csr_ind if it isn't empty
    {
        free(matrix->col_ind);
        matrix->col_ind = NULL;
    }
    if (matrix->row_ptr != NULL) // Free the allocated memory for row_ptr if it isn't empty
    {
        free(matrix->row_ptr);
        matrix->row_ptr = NULL;
    }

    // Safe case: set the other fields to default values
    matrix->num_non_zeros = 0;
    matrix->num_rows = 0;
    matrix->num_cols = 0;
}