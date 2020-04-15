#include "game.h"

const byte LED_PIN_DATA = 12;
const byte LED_PIN_CLOCK = 10;
const byte LED_PIN_CS = 11;

const byte LED_OP_DECODEMODE = 9;
const byte LED_OP_INTENSITY = 10;
const byte LED_OP_SCANLIMIT = 11;
const byte LED_OP_SHUTDOWN = 12;
const byte LED_OP_DISPLAYTEST = 15;

const byte BTN_PIN_DY = A0;
const byte BTN_PIN_DX = A1;
const int BTN_SIGNAL_MID_LEVEL = 518;
const int BTN_SIGNAL_THRESHOLD = BTN_SIGNAL_MID_LEVEL / 2;

const int LOOP_DELAY = 25;
const int GAME_LOOP_DELAY = 250;

const int RENDER_PANELS = 4;

byte led_buffer[RENDER_PANELS][2];

int game_loop_filter = 0;

Game game;
UserInput user_input;

void setup() {
    randomSeed(analogRead(A0));
  
    game_init(&game);
    game_prepare_level(&game);

    user_input.next_move = DIRECTION_RIGHT;

    render_init();
    render_field();
}

void loop() {
    input_read();

    game_loop_filter++;
    if (game_loop_filter >= GAME_LOOP_DELAY / LOOP_DELAY) {
        game_loop_filter = 0;
    
        STEP_RESULT result = game_handle_next_step(&game, &user_input);
        if (result == STEP_RESULT_SUCCESS) {
            render_field();
        } else {
            game_prepare_level(&game);
            user_input.next_move = DIRECTION_RIGHT;
            render_field();
        }
    }

    delay(LOOP_DELAY);
}

void input_read() {
    int dy = analogRead(BTN_PIN_DY) - BTN_SIGNAL_MID_LEVEL;
    int dx = analogRead(BTN_PIN_DX) - BTN_SIGNAL_MID_LEVEL;
    
    int abs_dy = dy < 0 ? -dy : dy;
    int abs_dx = dx < 0 ? -dx : dx;

    if (abs_dy > abs_dx) {
        if (abs_dy > BTN_SIGNAL_THRESHOLD) {
            user_input.next_move = dy < 0 ? DIRECTION_UP : DIRECTION_DOWN;
        }
    } else {
        if (abs_dx > BTN_SIGNAL_THRESHOLD) {
            user_input.next_move = dx < 0 ? DIRECTION_LEFT : DIRECTION_RIGHT;
        }
    }
}

void render_init() {
    pinMode(LED_PIN_DATA, OUTPUT);
    pinMode(LED_PIN_CLOCK, OUTPUT);
    pinMode(LED_PIN_CS, OUTPUT);
    digitalWrite(LED_PIN_CS, HIGH);

    render_led_send_command(LED_OP_DISPLAYTEST, 0);
    render_led_send_command(LED_OP_SCANLIMIT, 7);
    render_led_send_command(LED_OP_DECODEMODE, 0);
    render_led_send_command(LED_OP_SHUTDOWN, 1);
    render_led_send_command(LED_OP_INTENSITY, 8);
}

void render_field() {
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH;) {
            byte data = 0, mask = 1 << (FIELD_HEIGHT - 1), d = RENDER_PANELS - x / FIELD_HEIGHT - 1;
            for (; mask > 0; x++) {
                if (!GAME_CELL_IS_TYPE(game.field.cells[x][y], CELL_TYPE_EMPTY))
                    data |= mask;
                mask >>= 1;
            }
            
            led_buffer[d][0] = FIELD_HEIGHT - y;
            led_buffer[d][1] = data;
        }
        render_led_send_buffer();
    }
}

void render_led_send_command(byte opcode, byte data) {
    for (int i = 0; i < RENDER_PANELS; i++) {
        led_buffer[i][0] = opcode;
        led_buffer[i][1] = data;
    }
    render_led_send_buffer();   
}

void render_led_send_buffer() {
    digitalWrite(LED_PIN_CS, LOW);
    
    for (int i = RENDER_PANELS - 1; i >= 0; i--) {
        shiftOut(LED_PIN_DATA, LED_PIN_CLOCK, MSBFIRST, led_buffer[i][0]);
        shiftOut(LED_PIN_DATA, LED_PIN_CLOCK, MSBFIRST, led_buffer[i][1]);
    }
    
    digitalWrite(LED_PIN_CS, HIGH);
}
