#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    KEY_NONE,
    KEY_CHAR,
    KEY_ENTER,
    KEY_BACKSPACE
} key_type_t;

typedef struct {
    key_type_t type;
    char character;
} key_event_t;

uint8_t keyboard_get_scancode(void);
uint8_t keyboard_status(void);
bool keyboard_has_input(void);

bool keyboard_poll(key_event_t *event);

#endif
