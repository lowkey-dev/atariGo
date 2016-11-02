#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "board.h"
#include "move.h"
#include "liberties.h"


/*
Clear all of the data left over in stdin if the line read was longer than 70 
chars
*/
int clear_buffer(void) {
    int c;
    while((c = getchar()) != EOF && c != '\n');
    return 0;
}


/*
try and get a move (x and y cord) or save a file from the input read from 
stdin if the input is invalid, return an error flag to get_move. If the
input is valid, update the current move in the corresponding player
struct
*/
int verify_input(char* input, struct Player* p, struct Board* b) {
    int success = 0;
    if (input[0] == 'w') {
        strcpy(b->fileName, &input[1]);
        return 2;
    } else if(!isdigit(input[0])) {
        return 0; 
    } else {
        char* xStart;
        char* yStart;
        long x = -1, y = -1;
        if(isdigit(input[0])) {
            x = strtol(input, &xStart, 10);
        }
        if(!isspace(xStart[0])) {
            return 0;
        }
        if(isdigit(xStart[1])) {
            y = strtol(&xStart[1], &yStart, 10);
        } else {
            return 0;
        }
        if(isspace(yStart[0]) || isalpha(yStart[0])) {
            return 0;
        }
        if((x > -1) && (y > -1) && (x < b->rows) && (y < b->columns)) {
            p->currentMove[0] = x;
            p->currentMove[1] = y;
            success = 1;
        }
    }
    return success;
}


/*
 *print prompt to stdout and keep reading stdin until a valid move/save 
 *command is found. Once a string is read from stdin, it is processed by 
 *verify_input and checked for appropriate errors if an error occurs 
 *(ie EOF read) return the corresponding error code to calling function
*/
int get_move(struct Player* p, struct Board* b) {
    volatile int success = 0;
    while(!success) {
        char input[71];
        int read = 0;
        if(p->playerNumber == 0) {
            fprintf(stdout, "Player O> ");
        }
        if(p->playerNumber == 1) {
            fprintf(stdout, "Player X> ");
        }
        char* readMove = fgets(input, sizeof(input), stdin);
        //check to see if reading input from stdin was successful
        if (readMove != NULL) {
            size_t len = strlen(input);
            if(len > 0 && input[len - 1] == '\n') {
                input[--len] = '\0';
            }
            if(strlen(input) > 70) {
                clear_buffer();
            } else {
                read = 1;
            }
        }
        if(readMove == NULL) {
            return 6;
        } //attempt to read move from string entered
        if (read) {
            success = verify_input(input, p, b);
            if (!success) {
                read = 0;
            }
        }
        if(success == 2) {
            return 2;
        }
    }
    return 0;
}

/*
 *take args from argv and create a new game from the parameters specified
 *once players/board have been created, print a blank board ready to play
 */
int new_game(char** argv, struct Player* p1, struct Player* p2, 
        struct Board* b) {
    set_dimensions(b, atoi(argv[3]), atoi(argv[4]));
    build_board(b);
    setup_player(p1, b, *argv[1], 0);
    setup_player(p2, b, *argv[2], 1);
    print_board(b);
    return 0;
}


/*take values read from first line of load file and attempt
 * to restore game board. This function is a helper function to 
 * unpack_args to reduce line count. It checks if the board 
 * dimensions are consistent and then restores the board and
 * players from the data read
 */
int restore_game(char** argv, struct Board* b, struct Player* p1,
        struct Player* p2) {
    if(b->columns < 4 || b->columns > 1000 || b->rows < 4 ||
            b->columns > 1000) {
        return 5;
    }
    build_board(b);
    setup_player(p1, b, *argv[1], 0);
    setup_player(p2, b, *argv[2], 1);
    return 0;
}


/*
 *take a string of board parameters from a save file and place them in the
 *appropriate board/player variable based on position in string.
 *also check the char immedatly after finding a value to ensure that all
 *arguments are seperated by a single white space and the line contains 
 *no invalid chars
 *Long function due to 9 different arguments that ahve to be unpacked and
 *style convention prohibits efficient line use (such as having if 
 *statement body on the same line as the condition statement
*/
int unpack_args(char** argv, char* line, struct Player* p1, 
        struct Player* p2, struct Board* b) {
    int turn;
    char* start = line, * end;
    for(int i = 0; i < 9; i++) {
        switch(i) {
            case 0:
                b->rows = strtol(&start[0], &end, 10);                
                break;
            case 1:
                b->columns = strtol(&start[1], &end, 10);
                if(restore_game(argv, b, p1, p2)) {
                    return 5;
                }
                break;
            case 2:
                turn = strtol(&start[1], &end, 10);
                if(turn == 0 || turn == 1) {
                    b->turn = turn;
                    break;
                } else {
                    return 5;
                }
            case 3:
                p1->aiVariables.currentMove[0] = strtol(&start[1], &end, 10);
                break;
            case 4:
                p1->aiVariables.currentMove[1] = strtol(&start[1], &end, 10);
                break;
            case 5:
                p1->aiVariables.moveCount = strtol(&start[1], &end, 10);
                break;
            case 6:
                p2->aiVariables.currentMove[0] = strtol(&start[1], &end, 10);
                break;
            case 7:
                p2->aiVariables.currentMove[1] = strtol(&start[1], &end, 10);
                break;
            case 8:
                p2->aiVariables.moveCount = strtol(&start[1], &end, 10);
                break;            
        }
        if((!isspace(end[0]) || !isdigit(end[1])) && i < 8) {
            return 5;
        }
        if(i == 8 && end[0] != '\n') {
            return 5;
        }
        start = end;
    }
    return 0;
}


/*
 *read first line from save file and sequentially load variables into 
 *structure and player variables. If invalid characters are fund, return
 *an error
 */
int process_file_args(char** argv, char* line, struct Player* p1, 
        struct Player* p2, struct Board* b) {
    //check the first number is valid
    if(!isdigit(line[0])) {
        return 5;
    } //check the string is a valid length
    if(strlen(line) > 70) {
        return 5;
    } //attempt to restore game from values in string
    int error = unpack_args(argv, line, p1, p2, b);
    return error;
}


/*
 *load a line from a board into the baord struct, if lines are too short/long 
 *or invalid characters are found, return an error
 */
int process_file_board(char* line, int lineCount, struct Board* b) {
    //check each char is valid on the board, if a unexpected char or line
    //terminaton is read, throw an error
    for (int i = 0; i < b->columns; i++) {
        if(line[i] == '\n') {
            return 5;
        }
        if(line[i] != '.' && line[i] != 'O' && line[i] != 'X') {
            return 5;
        } //if the char is a valid piece place it on the board
        if(line[i] == 'O') {
            b->matrix[lineCount][i] = 'O';
        }
        if(line[i] == 'X') {
            b->matrix[lineCount][i] = 'X';
        }
    } //make sure line terminates with /n
    if(line[b->columns] != '\n') {
        return 5;
    }
    return 0;
}


/*
 *read and open file and take the arguments from the first line to restore
 *game variables. Use the rest of the file to load the location of stones
 *on the board.
 */
int read_file(FILE* file, struct Player* p1, struct Player* p2, 
        struct Board* b, char** argv) {
    char line[71] = "";
    //make sure line is valid
    if(fgets(line, sizeof(line), file) == NULL) {
        return 5;
    } //if the file args are invalid, return an error code
    if(process_file_args(argv, line, p1, p2, b)) {
        return 5;
    }
    int lineCount = 0, invalid;
    //restore the board line by line, if the file contains fewer lines
    //than expected or is longer than expected, return an error code
    for(int i = 0; i < b->rows; i++) {
        if(fgets(line, sizeof(line), file) == NULL) {
            return 5;
        }
        if(strlen(line) > 70) {
            return 5;
        }
        invalid = process_file_board(line, lineCount, b);
        if(invalid) {
            return 5;
        }
        lineCount++;
    }
    print_board(b);
    fclose(file);
    return 0;
}

/*
 *try to open a file from specified input, if invalid return an error. 
 *If the file opens successfully process the variabels in the file
 */
int load_game(char** argv, struct Player* p1, struct Player* p2, 
        struct Board* b) {
    FILE* file = fopen(argv[3], "r");
    if(file == NULL) {
        fclose(file);
        return 4;
    } else {
        return read_file(file, p1, p2, b, argv);
    }
}


/*
 *test the supplied file game initiation to ensure that the file is valid, 
 *throw an error if the file is invalid
 */
int test_file(char** fileName) {
    FILE* file = fopen(*fileName, "r");
    if(file == NULL) {
        return 4;
    }
    fclose(file);
    return 0;
}


/*
print an associated error message based on the errorCode flag
*/
int error_handle(int errorCode) {
    switch (errorCode) {
        case 1:
            fprintf(stderr, ("Usage: nogo p1type p2type [height width | "
                    "filename]\n"));
            return 1;
        case 2:
            fprintf(stderr, "Invalid player type\n");
            return 2;
        case 3:
            fprintf(stderr, "Invalid board dimension\n");
            return 3;
        case 4:
            fprintf(stderr, "Unable to open file\n");
            return 4;
        case 5:
            fprintf(stderr, "Incorrect file contents\n");
            return 5;
        case 6:
            fprintf(stderr, "End of input from user\n");
            return 6;
    }
    return 0;
}


/*
 *verify that arguments supplied by user at game initalization are valid and 
 *within expected range. Throw appropriate error if they are out of range
 *Note - long function due to several cases that need seperate staements
 and the style guide prevents else return statements on the same line
*/
int process_input(int argc, char** argv) {
    int isint = 1;
    if((argc < 4) || (argc > 5)) {
        return 1;
    }
    for(int i = 1; i < argc; i++) {
        switch(i) {
        //check args 1 and 2 are either computer or human
            case 1: 
            case 2:
	        if ((strcmp(argv[i], "h") && strcmp(argv[i], "c")) != 0) {
                    return 2;
                }
		break;
            //check if arg 3 is a board dimension (int) or file
	    case 3:
                for(char* p = *(argv + i); *p; p++) {
	            if(*p == '-') {
                        return 3;
                    }
                    if (!isdigit(*p)) {
                        isint = 0;
                        break;
		    }
                }
                if (!isint) {
                    int open = test_file(&argv[i]);
                    return open;
                }
                if (isint && (argc < 5)) {
                    return 3;
                }
                break;
        //check second board dimension is a valid int
            case 4:
                if(!isint) {
                    return 3;
                }
                for (char* p = *(argv + i); *p; p++) {
                    if (!isdigit(*p)) {
                        return 3;
                    }
                }
                if(atoi(argv[3]) < 4 || atoi(argv[3]) > 1000) {
                    return 3;
                }
                break;
        }
    }
    return 0;
}


/*
write all of the game variables (player/board variables) to first line of a
open file
*/
int save_state(FILE* file, struct Player* p1, struct Player* p2, 
        struct Board* b) {
    char* stateString = malloc(sizeof(char) * 71);
    int rows = b->rows;
    int columns = b->columns;
    int turn = b->turn;
    int p1m0 = p1->currentMove[0];
    int p1m1 = p1->currentMove[1];
    int p1move = p1->aiVariables.moveCount;
    int p2m0 = p2->currentMove[0];
    int p2m1 = p2->currentMove[1];
    int p2move = p2->aiVariables.moveCount;
    sprintf(stateString, "%d %d %d %d %d %d %d %d %d\n", rows, columns, turn, 
            p1m0, p1m1, p1move, p2m0, p2m1, p2move);
    fputs(stateString, file);
    free(stateString);
    return 0;
}

/*
write the current board configuration to an open file assigning each row of 
a board to its own line, terminated by a line return
*/
int save_board(FILE* file, struct Board* b) {
    char* rowToSave = malloc(sizeof(char) * b->columns + 1);
    rowToSave[b->columns] = '\n';
    for (int i = 0; i < b->rows; i++) {
        for (int j = 0; j < b->columns; j++) {
            rowToSave[j] = b->matrix[i][j];
        }
        fputs(rowToSave, file);
    }
    free(rowToSave);
    return 0;
}


/*
take an open file and save the curretn game variables to the first line and
the board configuration to subsequent lines. If the file is invalid, return
an error
*/
int save_game(char* fileName, struct Player* p1, struct Player* p2, 
        struct Board* b) {
    FILE* file = fopen(fileName, "w");
    if(file == NULL) {
        return 4;
    }
    if(p1->playerType == 'c') {
        next_computer_move(p1, b);
    }
    if(p2->playerType == 'c') {
        next_computer_move(p2, b);
    }
    if(p1->playerType == 'h') {
        p1->currentMove[0] = p1->aiVariables.currentMove[0];
        p1->currentMove[1] = p1->aiVariables.currentMove[1];
    }
    if(p2->playerType == 'h') {
        p2->currentMove[0] = p2->aiVariables.currentMove[0];
        p2->currentMove[1] = p2->aiVariables.currentMove[1];
    }
    save_state(file, p1, p2, b);
    save_board(file, b);
    fclose(file);
    return 0;
} 


/*
 *print end of game message
 */
int winner(struct Player* p) {
    if(p->playerNumber == 0) {
        fprintf(stdout, "Player O wins\n");
    }
    if(p->playerNumber == 1) {
        fprintf(stdout, "Player X wins\n");
    }
    return 0;
}


/*
 *for each player, get a move (eitehr from stdin for users or the ai move
 *generator for computers) and attempt to place a stone on the board. If an
 *error occurs or a special command (EOL or save) pass the error back to
 *calling function
 */
int generate_move(struct Board* b, struct Player* p) {
    if(p->playerType == 'c') {
        while(computer_move(&p->aiVariables, b), !make_move(b, p));
        if(p->playerNumber == 0) {
            printf("Player O: %d %d\n", 
                    p->aiVariables.currentMove[0], 
                    p->aiVariables.currentMove[1]);
        }
        if(p->playerNumber == 1) {
            printf("Player X: %d %d\n", 
                    p->aiVariables.currentMove[0], 
                    p->aiVariables.currentMove[1]);
        }
        print_board(b);
    }
    if(p->playerType == 'h') {
        do { //keep getting moves from stdin until it a free space is found
            int legalMove = get_move(p, b);
            if(legalMove == 6) {
                return 6;
            } //2 == save file flag
            if(legalMove == 2) {
                return 2;
            }
        } while(!make_move(b, p));
        print_board(b);
    }
    return 0;
}


/*
 *players alternate turn until a winner is found or an error flag is generated
 *If a error flag is generated, send it back to main to print an appropriate
 *error message
 */
int play_game(struct Board* b, struct Player* p1, struct Player* p2) {
    int validMove;
    do {
        if(b->turn == 0) {
            validMove = generate_move(b, p1);
            if(!has_liberties(p2, b)) {
                return winner(p1);
            } //if a winner is detected print winenr message
            if(!has_liberties(p1, b)) {
                return winner(p2);
            }
            if(validMove == 2) {
                save_game(b->fileName, p1, p2, b);
                continue;
            }
            b->turn = 1;
        } else if(b->turn == 1) {
            validMove = generate_move(b, p2);
            if(validMove == 2) {
                save_game(b->fileName, p1, p2, b);
                continue;
            } //if a winner is detected print winner message
            if(!has_liberties(p1, b)) {
                return winner(p2);
            }
            if(!has_liberties(p2, b)) {
                return winner(p1);
            }
            b->turn = 0;
        } // 6 == unexpected EOF char
        if (validMove == 6) {
            return 6;
        }
    } while (1);
}


int main(int argc, char** argv) {
    //check arguments are valid
    int error = process_input(argc, argv);
    if(error) {
        return error_handle(error);
    } //initalize player and board variables
    struct Player p1;
    struct Player p2;
    struct Board b;
    //load/create game based on args supplied
    if (isdigit(argv[3][0])) {
        new_game(argv, &p1, &p2, &b);
    }
    if (isalpha(argv[3][0])) {
        error = load_game(argv, &p1, &p2, &b);
        if(error) {
            return error_handle(error);
        }

    } //play game until a winner is found
    int game = play_game(&b, &p1, &p2);
    //if error flag thrown return error
    if(game) {
        return error_handle(game);
    }
    return 0;
}
