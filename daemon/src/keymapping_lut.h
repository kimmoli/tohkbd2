#ifndef KEYMAPPING_LUT_H
#define KEYMAPPING_LUT_H

#include <linux/input.h>
#include "keymapping.h"

/* Keycodes without modifier */

static const int lut_plain_scandic[] =
{
    0xA1, KEY_TAB, 0,
    0xA3, KEY_1, 0,
    0xA4, KEY_2, 0,
    0xA5, KEY_3, 0,
    0xA6, KEY_4, 0,
    0xA7, KEY_5, 0,
    0xA8, KEY_6, 0,
    0xA9, KEY_7, 0,
    0xAA, KEY_8, 0,
    0xAB, KEY_9, 0,
    0xAC, KEY_0, 0,
    0xAD, KEY_MINUS, 0,
    0xAE, KEY_EQUAL, 0,
    0xAF, KEY_BACKSPACE, 0,
    0xB1, KEY_DELETE, 0,
    0xB2, KEY_UP, 0,
    0xB3, KEY_INSERT, 0,
    0xB4, KEY_Q, 0,
    0xB5, KEY_W, 0,
    0xB6, KEY_E, 0,
    0xB7, KEY_R, 0,
    0xB8, KEY_T, 0,
    0xB9, KEY_Y, 0,
    0xBA, KEY_U, 0,
    0xBB, KEY_I, 0,
    0xBC, KEY_O, 0,
    0xBD, KEY_P, 0,
    0xBE, KEY_KPPLUS, 0,
    0xC1, KEY_LEFT, 0,
    0xC3, KEY_RIGHT, 0,
    0xC4, KEY_A, 0,
    0xC5, KEY_S, 0,
    0xC6, KEY_D, 0,
    0xC7, KEY_F, 0,
    0xC8, KEY_G, 0,
    0xC9, KEY_H, 0,
    0xCA, KEY_J, 0,
    0xCB, KEY_K, 0,
    0xCC, KEY_L, 0,
    0xCD, KEY_SEMICOLON, 0,
    0xCE, KEY_APOSTROPHE, 0,
    0xD1, KEY_HOME, 0,
    0xD2, KEY_DOWN, 0,
    0xD3, KEY_END, 0,
    0xD4, KEY_Z, 0,
    0xD5, KEY_X, 0,
    0xD6, KEY_C, 0,
    0xD7, KEY_V, 0,
    0xD8, KEY_B, 0,
    0xD9, KEY_N, 0,
    0xDA, KEY_M, 0,
    0xDB, KEY_SLASH, FORCE_SHIFT,
    0xDC, KEY_1, FORCE_SHIFT,
    0xDD, KEY_COMMA, 0,
    0xDE, KEY_DOT, 0,
    0xEC, KEY_2, FORCE_SHIFT,
    0xEF, KEY_ENTER, 0,
    0xE9, KEY_SPACE, 0,
    0,0,0
};

static const int lut_plain_qwertz[] =
{
    0xA1, KEY_TAB, 0,
    0xA3, KEY_1, 0,
    0xA4, KEY_2, 0,
    0xA5, KEY_3, 0,
    0xA6, KEY_4, 0,
    0xA7, KEY_5, 0,
    0xA8, KEY_6, 0,
    0xA9, KEY_7, 0,
    0xAA, KEY_8, 0,
    0xAB, KEY_9, 0,
    0xAC, KEY_0, 0,
    0xAD, KEY_MINUS, 0,
    0xAE, KEY_EQUAL, 0,
    0xAF, KEY_BACKSPACE, 0,
    0xB1, KEY_DELETE, 0,
    0xB2, KEY_UP, 0,
    0xB3, KEY_INSERT, 0,
    0xB4, KEY_Q, 0,
    0xB5, KEY_W, 0,
    0xB6, KEY_E, 0,
    0xB7, KEY_R, 0,
    0xB8, KEY_T, 0,
    0xB9, KEY_Z, 0,
    0xBA, KEY_U, 0,
    0xBB, KEY_I, 0,
    0xBC, KEY_O, 0,
    0xBD, KEY_P, 0,
    0xBE, KEY_KPPLUS, 0,
    0xC1, KEY_LEFT, 0,
    0xC3, KEY_RIGHT, 0,
    0xC4, KEY_A, 0,
    0xC5, KEY_S, 0,
    0xC6, KEY_D, 0,
    0xC7, KEY_F, 0,
    0xC8, KEY_G, 0,
    0xC9, KEY_H, 0,
    0xCA, KEY_J, 0,
    0xCB, KEY_K, 0,
    0xCC, KEY_L, 0,
    0xCD, KEY_SEMICOLON, 0,
    0xCE, KEY_APOSTROPHE, 0,
    0xD1, KEY_HOME, 0,
    0xD2, KEY_DOWN, 0,
    0xD3, KEY_END, 0,
    0xD4, KEY_Y, 0,
    0xD5, KEY_X, 0,
    0xD6, KEY_C, 0,
    0xD7, KEY_V, 0,
    0xD8, KEY_B, 0,
    0xD9, KEY_N, 0,
    0xDA, KEY_M, 0,
    0xDB, KEY_SLASH, FORCE_SHIFT,
    0xDC, KEY_1, FORCE_SHIFT,
    0xDD, KEY_COMMA, 0,
    0xDE, KEY_DOT, 0,
    0xEC, KEY_2, FORCE_SHIFT,
    0xEF, KEY_ENTER, 0,
    0xE9, KEY_SPACE, 0,
    0,0,0
};

static const int lut_plain_azerty[] =
{
    0xA1, KEY_TAB, 0,
    0xA3, KEY_1, 0,
    0xA4, KEY_2, 0,
    0xA5, KEY_3, 0,
    0xA6, KEY_4, 0,
    0xA7, KEY_5, 0,
    0xA8, KEY_6, 0,
    0xA9, KEY_7, 0,
    0xAA, KEY_8, 0,
    0xAB, KEY_9, 0,
    0xAC, KEY_0, 0,
    0xAD, KEY_MINUS, 0,
    0xAE, KEY_EQUAL, 0,
    0xAF, KEY_BACKSPACE, 0,
    0xB1, KEY_DELETE, 0,
    0xB2, KEY_UP, 0,
    0xB3, KEY_INSERT, 0,
    0xB4, KEY_A, 0,
    0xB5, KEY_Z, 0,
    0xB6, KEY_E, 0,
    0xB7, KEY_R, 0,
    0xB8, KEY_T, 0,
    0xB9, KEY_Y, 0,
    0xBA, KEY_U, 0,
    0xBB, KEY_I, 0,
    0xBC, KEY_O, 0,
    0xBD, KEY_P, 0,
    0xBE, KEY_KPPLUS, 0,
    0xC1, KEY_LEFT, 0,
    0xC3, KEY_RIGHT, 0,
    0xC4, KEY_Q, 0,
    0xC5, KEY_S, 0,
    0xC6, KEY_D, 0,
    0xC7, KEY_F, 0,
    0xC8, KEY_G, 0,
    0xC9, KEY_H, 0,
    0xCA, KEY_J, 0,
    0xCB, KEY_K, 0,
    0xCC, KEY_L, 0,
    0xCD, KEY_M, 0,
    0xCE, KEY_APOSTROPHE, 0,
    0xD1, KEY_HOME, 0,
    0xD2, KEY_DOWN, 0,
    0xD3, KEY_END, 0,
    0xD4, KEY_W, 0,
    0xD5, KEY_X, 0,
    0xD6, KEY_C, 0,
    0xD7, KEY_V, 0,
    0xD8, KEY_B, 0,
    0xD9, KEY_N, 0,
    //0xDA, KEY_M, 0,
    0xDB, KEY_SLASH, FORCE_SHIFT,
    0xDC, KEY_1, FORCE_SHIFT,
    0xDD, KEY_COMMA, 0,
    0xDE, KEY_DOT, 0,
    0xEC, KEY_2, FORCE_SHIFT,
    0xEF, KEY_ENTER, 0,
    0xE9, KEY_SPACE, 0,
    0,0,0
};



/* Keycodes with SYM Modifier */

static const int lut_sym[] =
{
    0xA1, KEY_ESC, 0,
    0xA3, KEY_F1, 0,
    0xA4, KEY_F2, 0,
    0xA5, KEY_F3, 0,
    0xA6, KEY_F4, 0,
    0xA7, KEY_F5, 0,
    0xA8, KEY_F6, 0,
    0xA9, KEY_F7, 0,
    0xAA, KEY_F8, 0,
    0xAB, KEY_F9, 0,
    0xAC, KEY_F10, 0,
    0xAD, KEY_F11, 0,
    0xAE, KEY_F12, 0,
    //0xAF, KEY_BACKSPACE, 0,
    0xB1, KEY_TOH_SELFIE, 0,            /* Del */
    0xB2, KEY_PAGEUP, 0,                /* Up */
    0xB3, KEY_TOH_SCREENSHOT, 0,        /* Ins */
    0xB4, KEY_GRAVE, FORCE_SHIFT,       /* Q ~ */
    0xB5, KEY_3, FORCE_SHIFT,           /* W # */
    0xB6, KEY_4, FORCE_SHIFT,           /* E $ */
    0xB7, KEY_5, FORCE_SHIFT,           /* R % */
    0xB8, KEY_6, FORCE_SHIFT,           /* T ^ */
    0xB9, KEY_7, FORCE_SHIFT,           /* Y & */
    0xBA, KEY_8, FORCE_SHIFT,           /* U * */
    0xBB, KEY_LEFTBRACE, FORCE_SHIFT,   /* I { */
    0xBC, KEY_RIGHTBRACE, FORCE_SHIFT,  /* O } */
    0xBD, KEY_MINUS, FORCE_SHIFT,       /* P _ */
    0xBE, KEY_EQUAL, FORCE_SHIFT,       /* Å + */
    0xC1, KEY_VOLUMEDOWN, 0,            /* Left */
    0xC3, KEY_VOLUMEUP, 0,              /* Right*/
    0xC4, KEY_SEMICOLON, 0,             /* A ; */
    0xC5, KEY_APOSTROPHE, 0,            /* S ' */
    //0xC6, KEY_D, 0,                   /* D £ */
    0xC7, KEY_GRAVE, 0,                 /* F ` */
    //0xC8, KEY_G, 0,                   /* G ´ */
    //0xC9, KEY_H, 0,
    //0xCA, KEY_J, 0,
    0xCB, KEY_LEFTBRACE, 0,             /* K [ */
    0xCC, KEY_RIGHTBRACE, 0,            /* L ] */
    0xCD, KEY_9, FORCE_SHIFT,           /* Ö ( */
    0xCE, KEY_0, FORCE_SHIFT,           /* Ä ) */
    0xD1, KEY_TOH_BACKLIGHT, 0,         /* Home */
    0xD2, KEY_PAGEDOWN, 0,              /* Down */
    //0xD3, KEY_END, 0,
    0xD4, KEY_SEMICOLON, FORCE_SHIFT,   /* Z : */
    0xD5, KEY_APOSTROPHE, FORCE_SHIFT,  /* X " */
    //0xD6, KEY_C, 0, /* C € */
    0xD7, KEY_SLASH, 0, /* V / */
    0xD8, KEY_BACKSLASH, FORCE_SHIFT,   /* B | */
    0xD9, KEY_BACKSLASH, 0,             /* N \ */
    //0xDA, KEY_M, 0,                   /* M µ */
    0xDB, KEY_SLASH, FORCE_SHIFT,       /* ? */
    0xDC, KEY_1, FORCE_SHIFT,           /* ! */
    0xDD, KEY_102ND, 0,                 /* , < */
    0xDE, KEY_102ND, FORCE_SHIFT,       /* . > */
    0xEC, KEY_TOH_NEWEMAIL, 0,          /* @ */
    //0xEF, KEY_ENTER, 0,
    //0xE9, KEY_SPACE, 0,
    0,0,0
};

#endif // KEYMAPPING_LUT_H
