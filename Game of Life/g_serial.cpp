#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

int my_hash(char cell)
{
    // return 0 if it's a dead cell('.') or 1 if it's an alive cell('X')
    if (cell == '.')
        return 0;
    else
        return 1;
    return -1;
}

int neighbours_no(char **matrix, int i, int j)
{
    // find number of neighbours of a cell
    int nr_neighbours = my_hash(matrix[i - 1][j - 1]) + my_hash(matrix[i - 1][j]) + \
                        my_hash(matrix[i - 1][j + 1]) + my_hash(matrix[i][j - 1]) + \
                        my_hash(matrix[i][j + 1]) + my_hash(matrix[i + 1][j - 1]) + \
                        my_hash(matrix[i + 1][j]) + my_hash(matrix[i + 1][j + 1]);
    return nr_neighbours;
}

char** bordered_matrix(char** matrix, int rows, int columns)
{
    // set the values of the matrix margins

    // first and last row
    for (int j = 1; j <= columns; j++)
    {
        matrix[0][j] = matrix[rows][j];
        matrix[rows + 1][j] = matrix[1][j];
    }

    // first and last column
    for (int i = 1; i <= rows; i++)
    {
        matrix[i][0] = matrix[i][columns];
        matrix[i][columns + 1] = matrix[i][1];
    }

    // upper corners
    matrix[0][0] = matrix[rows][columns];
    matrix[0][columns + 1] = matrix[rows][1];

    // bottom corners
    matrix[rows + 1][0] = matrix[1][columns];
    matrix[rows + 1][columns + 1] = matrix[1][1];

    return matrix;
}

int main(int argc, char *argv[])
{

    if (argc != 4)   // argc should be 4 for correct execution
    {
        // print argv[0] assuming it is the program name
        cout<<"usage: "<< argv[0] <<" <filename>\n";
    }
    else
    {
        // input file, number of iterations and output file
        ifstream input_file (argv[1]);
        int nr_iterations = atoi(argv[2]), rows, columns;
        ofstream output_file(argv[3]);

        // matrix size
        string vars_dimensions;

        getline(input_file, vars_dimensions, '\n');
        stringstream vars_dim(vars_dimensions);

        // get number of rows and number of columns
        vars_dim >> rows >> columns;

        // create two matrices with margins
        // matrix for the current iteration
        char** b_matrix_curr_iter = new char*[rows + 2];
        for (int i = 0; i < rows + 2; ++i)
            b_matrix_curr_iter[i] = new char[columns + 2];

        // matrix for the next iteration
        char** b_matrix_next_iter = new char*[rows + 2];
        for (int i = 0; i < rows + 2; ++i)
            b_matrix_next_iter[i] = new char[columns + 2];

        // Always check to see if file opening succeeded
        if ( !input_file.is_open() )
            cout << "Could not open file\n";
        else
        {
            string token;

            for (int i = 0; i < rows; ++i)
            {

                // get the line and make the string a stream
                getline(input_file, token);
                stringstream ss(token);

                int j = 0;
                while(ss.good() && j < columns)
                {
                    // read cell by cell from line
                    // operator ">>" skips the white spaces
                    // initially, I put the same values in the matrix with margins but starting from (1,1) not (0,0)
                    // after reading, I border the matrix
                    ss >> b_matrix_curr_iter[i + 1][j + 1];

                    ++j;
                }
            }
        }

        // close the input file
        input_file.close();

        // border the matrix
        b_matrix_curr_iter = bordered_matrix(b_matrix_curr_iter, rows, columns);

        int nr_neighbours;

        for (int k = 0; k < nr_iterations; k++)
        {
            for (int i = 1; i <= rows; i++)
            {
                for (int j = 1; j <= columns; j++)
                {
                    // find the number of neighbours for the current cell
                    nr_neighbours = neighbours_no(b_matrix_curr_iter, i, j);

                    // decide if the cell will be alive or dead at the next iteration
                    if (b_matrix_curr_iter[i][j] == 'X')
                    {
                        if (nr_neighbours < 2)
                            b_matrix_next_iter[i][j] = '.';
                        else if (nr_neighbours > 3)
                            b_matrix_next_iter[i][j] = '.';
                        else
                            b_matrix_next_iter[i][j] = 'X';
                    }
                    else if (b_matrix_curr_iter[i][j] == '.')
                    {
                        if (nr_neighbours == 3)
                            b_matrix_next_iter[i][j] = 'X';
                        else
                            b_matrix_next_iter[i][j] = '.';
                    }
                }
            }

            // copy the next_iteration_matrix to the current_iteration_matrix
            for (int i = 1; i <= rows; i++)
            {
                for (int j = 1; j <= columns; j++)
                {
                    b_matrix_curr_iter[i][j] = b_matrix_next_iter[i][j];
                }
            }

            // border the new matrix
            b_matrix_curr_iter = bordered_matrix(b_matrix_curr_iter, rows, columns);
        }

        // write the final output matrix to the file
        for (int i = 1; i < rows + 1; i++)
        {
            for (int j = 1; j < columns + 1; j++)
            {
                output_file << b_matrix_curr_iter[i][j] << ' ';
            }
            if (i < rows)
            	output_file << "\n";
        }

        // close the output file
        output_file.close();

        // free the matrices
        for (int i = 0; i < rows + 2; i++)
        {
            delete [] b_matrix_curr_iter[i];
            delete [] b_matrix_next_iter[i];
        }
        delete [] b_matrix_curr_iter;
        delete [] b_matrix_next_iter;
    }

    return 0;
}
