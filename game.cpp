#include "game.h"

#if defined(__EMSCRIPTEN__)

long random(long max) {
    return random() % max;
}

#endif

void move_snake(Game *game, Point next_point);
void increase_snake(Game *game, Point next_point);
bool is_next_snake_point_allowed(Game *game, Point point);
Point get_next_point(Point point, DIRECTION direction);
DIRECTION get_opposite_direction(DIRECTION direction);
void set_snake_direction(Game *game, DIRECTION direction);
void generate_food(Game *game);
void generate_blocks(Game *game);
void init_snake(Game *game);
void memzero(uint8_t *mem, size_t size);

void game_init(Game *game) {
    memzero((uint8_t*) game, sizeof(Game));
}

void game_prepare_level(Game *game) {
    memzero((uint8_t*) &(game->field), sizeof(Field));
    generate_blocks(game);
    init_snake(game);
    generate_food(game);
    game->score = 0;
}

STEP_RESULT game_handle_next_step(Game *game, UserInput *input) {
    set_snake_direction(game, input->next_move);

    Point current_snake_point = game->snake.first_point;
    Point next_snake_point = get_next_point(current_snake_point, 
                                            GAME_CELL_GET_SNAKE_NEXT(game->field.cells[current_snake_point.x][current_snake_point.y]));
    
    if (!is_next_snake_point_allowed(game, next_snake_point))
        return STEP_RESULT_FAIL;

    if (GAME_CELL_IS_TYPE(game->field.cells[next_snake_point.x][next_snake_point.y], CELL_TYPE_FOOD)) {
        game->score++;
        increase_snake(game, next_snake_point);
        generate_food(game);
    } else
        move_snake(game, next_snake_point);

    return STEP_RESULT_SUCCESS;
}

void move_snake(Game *game, Point next_point) {
    Point fp = game->snake.first_point;
    DIRECTION fd = GAME_CELL_GET_SNAKE_NEXT(game->field.cells[fp.x][fp.y]);
    Cell next_cell = GAME_CELL_MAKE_SNAKE(fd);

    Point lp = game->snake.last_point;
    DIRECTION ld = GAME_CELL_GET_SNAKE_NEXT(game->field.cells[lp.x][lp.y]);

    game->field.cells[lp.x][lp.y] = CELL_TYPE_EMPTY;
    game->field.cells[next_point.x][next_point.y] = next_cell;

    game->snake.first_point = next_point;
    game->snake.last_point = get_next_point(lp, ld);
}

void increase_snake(Game *game, Point next_point) {
    Point fp = game->snake.first_point;
    DIRECTION fd = GAME_CELL_GET_SNAKE_NEXT(game->field.cells[fp.x][fp.y]);
    Cell next_cell = GAME_CELL_MAKE_SNAKE(fd);

    game->field.cells[next_point.x][next_point.y] = next_cell;
    game->snake.first_point = next_point;
}

bool is_next_snake_point_allowed(Game *game, Point point) {
    Cell cell = game->field.cells[point.x][point.y];
    CELL_TYPE type = GAME_CELL_GET_TYPE(cell);

    if (GAME_CELL_IS_TYPE(cell, CELL_TYPE_EMPTY))
        return true;

    if (GAME_CELL_IS_TYPE(cell, CELL_TYPE_FOOD))
        return true;

    if (GAME_CELL_IS_TYPE(cell, CELL_TYPE_SNAKE)
        && POINT_IS_EQUAL(point, game->snake.last_point))
        return true;

    return false;
}

Point get_next_point(Point point, DIRECTION direction) {
    switch (direction) {
        case DIRECTION_LEFT:
            return POINT((uint8_t) (((uint8_t) (point.x - 1)) % FIELD_WIDTH), point.y);
        case DIRECTION_RIGHT:
            return POINT((uint8_t) (((uint8_t) (point.x + 1)) % FIELD_WIDTH), point.y);
        case DIRECTION_UP:
            return POINT(point.x, (uint8_t) (((uint8_t) (point.y - 1)) % FIELD_HEIGHT));
        case DIRECTION_DOWN:
            return POINT(point.x, (uint8_t) (((uint8_t) (point.y + 1)) % FIELD_HEIGHT));
    }

    return point;
}

DIRECTION get_opposite_direction(DIRECTION direction) {
    switch (direction) {
        case DIRECTION_LEFT:
            return DIRECTION_RIGHT;
        case DIRECTION_RIGHT:
            return DIRECTION_LEFT;
        case DIRECTION_UP:
            return DIRECTION_DOWN;
        case DIRECTION_DOWN:
            return DIRECTION_UP;
    }
    return DIRECTION_RIGHT;
}

void set_snake_direction(Game *game, DIRECTION direction) {
    Point p = game->snake.first_point;
    Cell c = game->field.cells[p.x][p.y];
    DIRECTION curr_direction = GAME_CELL_GET_SNAKE_NEXT(c);

    if (curr_direction == direction 
        || get_opposite_direction(curr_direction) == direction)
        return;

    game->field.cells[p.x][p.y] = (c & 0xf3) | (direction << 2);
}

/**
 * Generate food by following algoright (non optimal):
 * - generate random value (RND) in range from 0 to field WxH;
 * - iterate thought all cells until we found free cell under index RND, we skip occupied cells and do not count them.
 */
void generate_food(Game *game) {
    int rnd = (int) random(FIELD_WIDTH * FIELD_HEIGHT - game->score - 2);

    while (true) {
        for (int i = 0; i < FIELD_WIDTH; ++i) {
            for (int j = 0; j < FIELD_HEIGHT; ++j) {
                if (GAME_CELL_IS_TYPE(game->field.cells[i][j], CELL_TYPE_EMPTY)) {
                    if (--rnd <= 0) {
                        game->field.cells[i][j] = GAME_CELL_MAKE_FOOD;
                        return;
                    }
                }
            }
        }
    }
}

void generate_blocks(Game *game) {
    const long RND_MAX = 0x7FFFFFFFl;
    const long RND_BIN_THRESHOLD = RND_MAX / 2;

    if (random(RND_MAX) < RND_BIN_THRESHOLD) {
        /* No borders at all */
        return;
    }

    if (random(RND_MAX) > RND_BIN_THRESHOLD) {
        /* Left border */
        for (int i = 0; i < FIELD_HEIGHT; i++)
            game->field.cells[0][i] = GAME_CELL_MAKE_BLOCK;
    }

    if (random(RND_MAX) > RND_BIN_THRESHOLD) {
        /* Right border */
        for (int i = 0; i < FIELD_HEIGHT; i++)
            game->field.cells[FIELD_WIDTH - 1][i] = GAME_CELL_MAKE_BLOCK;
    }

    if (random(RND_MAX) > RND_BIN_THRESHOLD) {
        /* Top border */
        for (int i = 0; i < FIELD_WIDTH; i++)
            game->field.cells[i][0] = GAME_CELL_MAKE_BLOCK;
    }

    if (random(RND_MAX) > RND_BIN_THRESHOLD) {
        /* Bottom border */
        for (int i = 0; i < FIELD_WIDTH; i++)
            game->field.cells[i][FIELD_HEIGHT - 1] = GAME_CELL_MAKE_BLOCK;
    }
}

/**
 * Generate two cell snake in the field center.
 */
void init_snake(Game *game) {
    const int P1_X = FIELD_WIDTH / 2;
    const int P1_Y = FIELD_HEIGHT / 2;

    const int P2_X = FIELD_WIDTH / 2 - 1;
    const int P2_Y = FIELD_HEIGHT / 2;

    game->field.cells[P1_X][P1_Y] = GAME_CELL_MAKE_SNAKE(DIRECTION_RIGHT);
    game->field.cells[P2_X][P2_Y] = GAME_CELL_MAKE_SNAKE(DIRECTION_RIGHT);

    game->snake.first_point.x = P1_X;
    game->snake.first_point.y = P1_Y;

    game->snake.last_point.x = P2_X;
    game->snake.last_point.y = P2_Y;
}

/**
 * Fill memory block by zeros.
 */
void memzero(uint8_t *mem, size_t size) {
    while (size-- > 0)
        *mem++ = 0;
}
