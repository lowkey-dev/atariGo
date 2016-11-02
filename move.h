#ifndef MOVE_H_
#define MOVE_H_

struct AiMoveVariables {
    int factor;
    int moveCount;
    int board;
    int newMove;
    int row;
    int column;
    int currentMove[2];
    int player;
};

struct Player {
    char playerType;
    int playerNumber;
    int currentMove[2];
    struct AiMoveVariables aiVariables;
};

int next_computer_move(struct Player* p, struct Board* b);

int computer_move(struct AiMoveVariables* aiVariables, struct Board* b);

int setup_computer_move_gen(struct AiMoveVariables* aiVariables, 
        int row, int player);

int make_move(struct Board* b, struct Player* p);

int setup_player(struct Player* p, struct Board* b, char playerType, 
        int playerNumber);

#endif // MOVE_H_
