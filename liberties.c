#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "move.h"

/*define structures to store the locations of strings once the first element
 * has been found
 *Also need to define the entry and exit points of a linked structure
 *to make searchign for liberties along a strring simple
 */
struct StringNode {
    int cords[2];
    struct StringNode* next;
};

struct String {
    struct StringNode* head;
    struct StringNode* tail;
};

typedef struct String String;

/*
 *initalize the struct to store strings of stones
 */
void init_string(String* s) {
    s->head = 0;
    s->tail = 0;
}

/*
 *remove all of the current stones from the que
 */
void free_string(String* s) {
    struct StringNode* t = s->head;
    while(t != 0) {
        struct StringNode* temp = t;
	t = t->next;
	free(temp);
    }
}

/*
 *add a new piece to the end of the linked que and update the trailing
 * pointer
 */
void build_string(String* s, int* cords) {
    //If it is the first element of the que need to initalize the que
    if(s->head == 0) {
        s->tail = s->head = malloc(sizeof(struct StringNode));
	s->head->next = 0;
    } else {
        struct StringNode* n = malloc(sizeof(struct StringNode));
	n->next = 0;
	s->tail->next = n;
	s->tail = n;
    }
    //update the last element to contain the cords of the stone to be
    //added to the end of the que
    s->tail->cords[0] = cords[0];
    s->tail->cords[1] = cords[1];
}

/*
 *return the current leading element of the string, if it finds the end of
 * the string, ensure that the leading/trailing pointers are updated
 */
int get_string(String* s, int* cords) {
    //If the string is empty return error cords
    if(s->head == 0) {
	cords[0] = -1;
	cords[1] = -1;
	return 1;
    }
    //extract cords from que
    cords[0] = s->head->cords[0];
    cords[1] = s->head->cords[1];
    //remove the element from the que and update pointers
    struct StringNode* temp = s->head;
    s->head = s->head->next;
    free(temp);
    if(s->head == 0) {
	s->tail = 0;
    }
    return 0;
}

/*
 *take the location of a current piece and search for connected stone of the 
 *same type. If stones are found and haven't already beeen added to the 
 *string, add them to the linked que and repeat the search for the new piece
 */
void search_string(String* s, struct Player* p, struct Board* b, 
        int* cords, int** checked) {
    build_string(s, cords);
    checked[cords[0]][cords[1]] = 1;
    int* newcords = malloc(sizeof(int) * 2);
    for(int i = 0; i < 4; i++) {
    	switch(i) {
    	    case 0:
                newcords[0] = cords[0] + 1;
                newcords[1] = cords[1];
            	break;
            case 1:
            	newcords[0] = cords[0] - 1;
            	newcords[1] = cords[1];
            	break;
            case 2:
            	newcords[0] = cords[0];
            	newcords[1] = cords[1] + 1;
            	break;
            case 3:
            	newcords[0] = cords[0];
            	newcords[1] = cords[1] - 1;       	
            	break;
    	} //if the cords are off the board move to the next location
    	if(newcords[0] < 0 || newcords[0] >= b->rows || newcords[1] < 0 || 
                newcords[1] >= b->columns) {
            continue;
        } //if the cords contain a stone from the same player recursively
        //call sring_search to build the string
    	if((is_occupied(b, newcords) == p->playerNumber + 1) && 
                !checked[newcords[0]][newcords[1]]) {
            search_string(s, p, b, newcords, checked);
        }
    	checked[newcords[0]][newcords[1]] = 1;
    }
    free(newcords);
}

/*
 *check the current string (linked que) for liberties. If one piece is found 
 * to have a liberty return 0, otherwise retrun 1 (indicates no liberties 
 * found)
 */
int check_string_liberties(String* s, struct Board* b) {
    int* cords = malloc(sizeof(int) * 2);
    int* newcords = malloc(sizeof(int) * 2);
    //Loop over all elements of the linked que, if a liberty i found return
    //zero
    while (!get_string(s, cords)) {
    	for(int i = 0; i < 4; i++) {
            switch(i) {
                case 0:
    	    	    newcords[0] = cords[0] + 1;
    	    	    newcords[1] = cords[1];
    	            break;
    	        case 1:
                    newcords[0] = cords[0] - 1;
    	            newcords[1] = cords[1];
    	            break;
    	        case 2:
    	            newcords[0] = cords[0];
    	            newcords[1] = cords[1] + 1;
    	            break;
    	        case 3:
    	            newcords[0] = cords[0];
    	            newcords[1] = cords[1] - 1;          	
    	            break;
            }
            if(newcords[0] < 0 || newcords[0] >= b->rows || 
                    newcords[1] < 0 || newcords[1] >= b->columns) {
                continue;
            }
            if(!is_occupied(b, newcords)) {
                free(newcords);                      
                return 0;
            }
        }
    }
    free(cords);
    free(newcords);
    return 1;
}


/*
 *deallocate the matrix used to remember which locations have been checked
 */
void clean_up(int** checked, struct Board* b) {
    for(int i = 0; i < b->rows; i++) {
        free(checked[i]);
    }
    free(checked);
}


/*
 *create a board to record which spaces have been checked
 *initalize the board as all zeroes and flip indicies to 
 *1 as a location is checked
 */
int** check_board(struct Board* b) {
    int** checked = malloc(sizeof(int*) * b->rows);
    for(int i = 0; i < b->rows; i++) {
        checked[i] = malloc(sizeof(int) * b->columns);
    }
    for(int i = 0; i < b->rows; i++) {
        for(int j = 0; j < b->columns; j++) {
            checked[i][j] = 0;
        }
    }
    return checked;
}

/*
 *create a blank board, search the current game board for the desired piece
 *(either player 1 or player 2), updating the blank matrix to prevent 
 *duplicate searches of the same location. If a string is found earch for 
 *conencted pieces then check the entire string for liberties. If no
 *liberties are found return 1, otherwise keep searching the board until
 *all locations have been checked
 */
int has_liberties(struct Player* p, struct Board* b) {
    int liberties = 0;
    int cords[2];
    int** checked = check_board(b);
    for(int i = 0; i < b->rows; i++) {
        cords[0] = i;
        for(int j = 0; j < b->columns; j++) {
            cords[1] = j;
            String s;
            init_string(&s);
            //check to see if the cord is occupied and has not been 
            //checked before. If it is new and the correct piece
            //try to build a string.  Then search fr liberties attached
            //to the string
            if((is_occupied(b, cords) == p->playerNumber + 1) &&
                    !checked[i][j]) {
                search_string(&s, p, b, cords, checked);
                liberties = check_string_liberties(&s, b);
            }
            checked[i][j] = 1;
            //if a liberty is not found (has_liberties returns 1)
            //return a zero (no liberties)
            if(liberties) {
                clean_up(checked, b);
                free_string(&s);
                return 0;
            }
            free_string(&s);
        }
    }
    clean_up(checked, b);
    //if all strings found have at least one liberty, return 1
    return 1;
}
