#include <time.h>
#include <stdlib.h>
#include <emscripten.h>

#include "../game.h"

const int LOOP_DELAY = 25;
const int GAME_LOOP_DELAY = 250;

const int GAME_FIELD_CELL_SIZE = 10;

const int CANVAS_WIDTH = FIELD_WIDTH * GAME_FIELD_CELL_SIZE;
const int CANVAS_HEIGHT = FIELD_HEIGHT * GAME_FIELD_CELL_SIZE;

int canvas[CANVAS_WIDTH * CANVAS_HEIGHT];

Game game;
UserInput user_input;

int game_speed = 5;
int game_speed_loop_filter = 0;

int make_color(int r, int g, int b) {
   int color = (255 << 24) 
               | (b << 16) 
               | (g << 8) 
               | r;
   return color;
}

#ifdef __cplusplus
extern "C" {
#endif

int* EMSCRIPTEN_KEEPALIVE init() {
    srand(time(NULL));
  
    game_init(&game);
    game_prepare_level(&game);

    user_input.next_move = DIRECTION_RIGHT;

    return canvas;
}

void EMSCRIPTEN_KEEPALIVE set_input(int keycode) {
   user_input.next_move = (DIRECTION) keycode;
}

bool EMSCRIPTEN_KEEPALIVE execute_game_loop_iteration() {
   game_speed_loop_filter++;

   if (game_speed_loop_filter >= game_speed) {
      game_speed_loop_filter = 0;

      STEP_RESULT result = game_handle_next_step(&game, &user_input);

      if (result != STEP_RESULT_SUCCESS) {
         game_prepare_level(&game);
         user_input.next_move = DIRECTION_RIGHT;
      }

      return true;
   } else {
      return false;
   }
}

void EMSCRIPTEN_KEEPALIVE render() {
   for (int x = 0; x < FIELD_WIDTH; x++) {
        for (int y = 0; y < FIELD_WIDTH; y++) {
            int color;

            switch (GAME_CELL_GET_TYPE(game.field.cells[x][y])) {
               case CELL_TYPE_BLOCK:
                  color = make_color(0, 0, 0);
                  break;
               case CELL_TYPE_EMPTY:
                  color = make_color(255, 255, 255);
                  break;
               case CELL_TYPE_FOOD:
                  color = make_color(255, 0, 0);
                  break;
               case CELL_TYPE_SNAKE:
                  color = make_color(0, 0, 255);
                  break;
            }

            int firstIndex = ((y * GAME_FIELD_CELL_SIZE) * (FIELD_WIDTH * GAME_FIELD_CELL_SIZE)) + (x * GAME_FIELD_CELL_SIZE);

            if (canvas[firstIndex] == color) {
               continue;
            }
            
            for (int i = 0; i < GAME_FIELD_CELL_SIZE; ++i) {
               for (int j = 0; j < GAME_FIELD_CELL_SIZE; ++j) {
                  int index = ((y * GAME_FIELD_CELL_SIZE + j) * (FIELD_WIDTH * GAME_FIELD_CELL_SIZE)) + (x * GAME_FIELD_CELL_SIZE + i);
                  canvas[index] = color;
               }
            }
        }
    }
}

int EMSCRIPTEN_KEEPALIVE get_canvas_width() {
   return CANVAS_WIDTH;
}

int EMSCRIPTEN_KEEPALIVE get_canvas_height() {
   return CANVAS_HEIGHT;
}

void EMSCRIPTEN_KEEPALIVE increase_speed() {
   if (game_speed > 1) {
      game_speed--;
   }
}

void EMSCRIPTEN_KEEPALIVE decrease_speed() {
   game_speed++;
}

#ifdef __cplusplus
}
#endif