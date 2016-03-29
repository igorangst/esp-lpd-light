#include "color.h"

Color color_interpolate(Color a, Color b, uint16 pos)
{
  Color c;
  c.r = ((128 - pos)*a.r + pos*b.r) >> 7;
  c.g = ((128 - pos)*a.g + pos*b.g) >> 7;
  c.b = ((128 - pos)*a.b + pos*b.b) >> 7;
  return c;
}

Color color_interpolate_list(Color* cs, uint16 n, uint16 pos)
{
  uint16 i = (pos >> 7) % n;
  uint16 j = (i + 1) % n;
  return color_interpolate(cs[i], cs[j], pos & 0x7f);
}

Color dim(Color c, uint16 fac)
{
  Color dimmed = c;
  dimmed.r = ((uint16)c.r * fac) >> 7;
  dimmed.g = ((uint16)c.g * fac) >> 7;
  dimmed.b = ((uint16)c.b * fac) >> 7;
  return dimmed;
}
