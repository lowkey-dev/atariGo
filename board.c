#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Board {
    int rows, columns, turn;
    char** matrix;
    char fileName[71];
};


/*
 *construct a 2 dimension array of format [row][column] to store data about
 *current game
*/
int set_dimensions(struct Board* b, int rows, int columns) {
    b->rows = rows;
    b->columns = columns;
    return 0;
}


/*
 *initalize every element of the board to be blank (default symbol '.') and 
 *initalize the turn count to 0
 */
int build_board(struct Board* b) {
    b->matrix = malloc(sizeof(int*) * b->rows);
    for(int i = 0; i < b->rows; i++) {
        b->matrix[i] = malloc(sizeof(int) * b->columns);
    } //set every element to be an empty space
    for(int i = 0; i < b->rows; i++) {
        for(int j = 0; j < b->columns; j++) {
            b->matrix[i][j] = '.';
        }
    } //initalize the turn count to zero
    b->turn = 0;
    return 0;
}


/*
 *ask the board if the co-ordinates specified is occupied by a player piece.
 *Returns a non zero value if the space is ocupied
 */
int is_occupied(struct Board* b, int* cords) {
    if(b->matrix[cords[0]][cords[1]] == 'O') {
        return 1;
    } else if (b->matrix[cords[0]][cords[1]] == 'X') {
        return 2;
    }
    return 0;
}


/*
 *adds a player stone to the co-ordinates specified
 */
int place_stone(struct Board* b, int* cords, int player) {
    if(player == 0) {
        b->matrix[cords[0]][cords[1]] = 'O';
        b->turn = 1;
    }
    if(player == 1) {
        b->matrix[cords[0]][cords[1]] = 'X';
        b->turn = 0;
    }
    return 0;
}


/*
 *prints the board row by row adding the necessery formatting to leading
 *and trailing spacers before outputting the row to stdout
 */
int print_row(struct Board* b, char* rowToPrint) {
    //make sure the string terminates properly
    rowToPrint[b->columns] = '\0';
    for(int i = 0; i < b->rows; i++) {
        for(int j = 0; j < b->columns; j++) {
            int cords[] = {i, j};
            switch(is_occupied(b, cords)) {
                case 0:
                    rowToPrint[j] = '.';
                    break;
                case 1:
                    rowToPrint[j] = 'O';
                    break;
                case 2:
                    rowToPrint[j] = 'X';
                    break;
            }
        } //print the row and add a new line character
        fprintf(stdout, "%c", '|');
        fprintf(stdout, "%s", rowToPrint);
        fprintf(stdout, "%c\n", '|');
    }
    return 0;
}

/*
prints the current board by first printing the tp border of the board
then sequentially printing rows of the board, finally printing the trailing
border
*/
int print_board(struct Board* b) {
    char* firstRow = malloc(sizeof(char) * b->columns + 3);
    char* lastRow = malloc(sizeof(char) * b->columns + 3);
    //build leading and trailing rows of the board
    for(int i = 0; i < b->columns + 2; i++) {
        if(i == 0) {
            firstRow[i] = '/';
            lastRow[i] = '\\';
        } else if(i == b->columns + 1) {
            firstRow[i] = '\\';
            lastRow[i] = '/';
        } else {
            firstRow[i] = '-';
            lastRow[i] = '-';
        }
    }//print first row (border)
    firstRow[b->columns + 2] = '\0';
    lastRow[b->columns + 2] = '\0';
    fprintf(stdout, "%s\n", firstRow);
    char* rowToPrint = malloc(sizeof(char) * b->columns + 1);
    //then print game board
    print_row(b, rowToPrint);
    //finally print last row (border)
    fprintf(stdout, "%s\n", lastRow);
    free(firstRow);
    free(lastRow);
    free(rowToPrint);
    return 0;
}
