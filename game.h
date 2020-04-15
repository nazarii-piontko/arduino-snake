#ifndef GAME_H
#define GAME_H

#include "Arduino.h"

const int FIELD_WIDTH = 32;
const int FIELD_HEIGHT = 8;

/**
 * Next snake direction or direction of snake move
 */
typedef enum {
    CELL_TYPE_EMPTY,
    CELL_TYPE_SNAKE,
    CELL_TYPE_FOOD,
    CELL_TYPE_BLOCK
} CELL_TYPE;

/**
 * Next snake direction or direction of snake move
 */
typedef enum {
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_UP,
    DIRECTION_DOWN
} DIRECTION;

typedef enum {
    STEP_RESULT_SUCCESS,
    STEP_RESULT_FAIL
 } STEP_RESULT;

typedef struct Point {
    /**
     * X-coordinate: [0..FIELD_WIDTH].
     */
    byte x;
    /**
     * Y-coordinate: [0..FIELD_HEIGHT].
     */
    byte y;
} Point;

#define POINT(x, y) ((Point) {(byte) x, (byte) y})

#define POINT_IS_EQUAL(p1, p2) (p1.x == p2.x && p1.y == p2.y)

typedef struct Snake {
    Point first_point;
    Point last_point;;
} Snake;

/**
 * Cell data will be encoded in one byte by the following bit schema:
 * 2 bits - type of the cell, one of CELL_TYPE value.
 * Next bits depends on type of cell.
 * - Snake cell:
 *  3 bits - next snake cell direction, one of DIRECTION value.
 *  3 bits - next snake cell direction, one of DIRECTION value.
 */
typedef byte Cell;

#define GAME_CELL_MAKE_EMPTY ((Cell) CELL_TYPE_EMPTY)
#define GAME_CELL_MAKE_SNAKE(next_direction) ((Cell) (CELL_TYPE_SNAKE | ((next_direction) << 2)))
#define GAME_CELL_MAKE_FOOD ((Cell) CELL_TYPE_FOOD)
#define GAME_CELL_MAKE_BLOCK ((Cell) CELL_TYPE_BLOCK)

#define GAME_CELL_GET_TYPE(cell) ((CELL_TYPE) ((cell) & 0x03))
#define GAME_CELL_IS_TYPE(cell, type) (GAME_CELL_GET_TYPE(cell) == (type))

#define GAME_CELL_GET_SNAKE_NEXT(cell) ((DIRECTION) (((cell) >> 2) & 0x03))

typedef struct Field {
    Cell cells[FIELD_WIDTH][FIELD_HEIGHT];
} Field;

typedef struct Game {
    Field field;
    Snake snake;
    Point food_location;
    short score;
} Game;

typedef struct UserInput {
    DIRECTION next_move;
} UserInput;

void game_init(Game *game);
void game_prepare_level(Game *game);
STEP_RESULT game_handle_next_step(Game *game, UserInput *input);

#endif
