#ifndef BOARD_H_
#define BOARD_H_

struct Board {
    int rows, columns, turn;
    char** matrix;
    char fileName[71];
};

int set_dimensions(struct Board* b, int rows, int columns);

int build_board(struct Board* b);

int is_occupied(struct Board* b, int* cords);

int place_stone(struct Board* b, int* cords, int player);

int print_row(struct Board* b, char* rowToPrint);

int print_board(struct Board* b);

#endif // BOARD_H_
