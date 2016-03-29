#ifndef COLOR_H
#define COLOR_H

#include "lpd8806.h"

#define COLOR_RED     (const Color){0x7f, 0x00, 0x00}
#define COLOR_GREEN   (const Color){0x00, 0x7f, 0x00}
#define COLOR_BLUE    (const Color){0x00, 0x00, 0x7f}
#define COLOR_YELLOW  (const Color){0x7f, 0x7f, 0x00}
#define COLOR_PURPLE  (const Color){0x7f, 0x00, 0x7f}
#define COLOR_CYAN    (const Color){0x00, 0x7f, 0x7f}
#define COLOR_BLACK   (const Color){0x00, 0x00, 0x00}
#define COLOR_WHITE   (const Color){0x7f, 0x7f, 0x7f}

/* Returns a color interpolated between colors a and b,
 * at position pos, where pos is in [0, 128]. */
Color color_interpolate(Color a, Color b, uint16 pos);

/* Returns a color interpolated between colors in a list,
 * where for a list of length n, pos should be in [0, n*128].
 * pos=0 corresponds to the first color, while pos in 
 * [128*(n-1),128*n] will wrap around to the first color again. */
Color color_interpolate_list(Color* cs, uint16 n, uint16 pos);

Color dim(Color c, uint16 fac);

#endif
