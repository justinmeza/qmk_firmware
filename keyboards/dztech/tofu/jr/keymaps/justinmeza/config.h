#pragma once

// LEDs off by default: solid-color effect at zero brightness. Layer-flash and
// QK_BOOT indicators set raw LED colors, so they still display over this.
#undef RGB_MATRIX_DEFAULT_MODE
#undef RGB_MATRIX_DEFAULT_HUE
#undef RGB_MATRIX_DEFAULT_SAT
#undef RGB_MATRIX_DEFAULT_VAL
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_COLOR
#define RGB_MATRIX_DEFAULT_HUE 0
#define RGB_MATRIX_DEFAULT_SAT 0
#define RGB_MATRIX_DEFAULT_VAL 0
