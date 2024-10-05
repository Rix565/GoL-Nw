#include "eadk.h"
#include <string.h>
#include <limits.h>

#define WIDTH 160
#define HEIGHT 120

#define PIXEL_SIZE 2

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

#define BYTES_NEEDED BITNSLOTS(WIDTH *HEIGHT)

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Game Of Life";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

char map_bitset[BYTES_NEEDED];
char counters[WIDTH * HEIGHT];
char counters2[WIDTH * HEIGHT];

#define set_at(cx, cy)                                                           \
  for (int xx = 0; xx < 3; xx++)                                                 \
  {                                                                              \
    for (int yy = 0; yy < 3; yy++)                                               \
    {                                                                            \
      if (!(xx == 1 && yy == 1))                                                 \
      {                                                                          \
        counters[(cx + xx - 1) % WIDTH + ((cy + yy - 1) % HEIGHT) * WIDTH] += 1; \
      }                                                                          \
    }                                                                            \
  }

#define clear_at(cx, cy)                                                         \
  for (int xx = 0; xx < 3; xx++)                                                 \
  {                                                                              \
    for (int yy = 0; yy < 3; yy++)                                               \
    {                                                                            \
      if (!(xx == 1 && yy == 1))                                                 \
      {                                                                          \
        counters[(cx + xx - 1) % WIDTH + ((cy + yy - 1) % HEIGHT) * WIDTH] -= 1; \
      }                                                                          \
    }                                                                            \
  }

int main(int argc, char *argv[])
{
  for (int x = 0; x < WIDTH; x++)
  {
    for (int y = 0; y < HEIGHT; y++)
    {
      BITCLEAR(map_bitset, x + y * WIDTH);
      counters[x + y * WIDTH] = 0;
      eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x * PIXEL_SIZE), (uint16_t)(y * PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_white);
    }
  }

  for (int x = 0; x < WIDTH; x++)
  {
    for (int y = 0; y < HEIGHT; y++)
    {
      if (eadk_random() > 1 << 31)
      {
        BITSET(map_bitset, x + y * WIDTH);
        set_at(x, y);
        eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x * PIXEL_SIZE), (uint16_t)(y * PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_black);
      }
    }
  }

  while (true)
  {
    memcpy(counters2, counters, WIDTH * HEIGHT);

    for (int x = 0; x < WIDTH; x++)
    {
      for (int y = 0; y < HEIGHT; y++)
      {
        int count = counters2[x + y * WIDTH];

        if ((count < 2) || (count > 3))
        {
          if (BITTEST(map_bitset, x + y * WIDTH))
          {
            clear_at(x, y);
          };
          BITCLEAR(map_bitset, x + y * WIDTH);
          eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x * PIXEL_SIZE), (uint16_t)(y * PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_white);
        }
        else if (count == 3)
        {
          if (!BITTEST(map_bitset, x + y * WIDTH))
          {
            set_at(x, y);
          }
          BITSET(map_bitset, x + y * WIDTH);
          eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x * PIXEL_SIZE), (uint16_t)(y * PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_black);
        }
      }
    }

    eadk_keyboard_state_t keyboardState = eadk_keyboard_scan();

    if (eadk_keyboard_key_down(keyboardState, eadk_key_home))
    {
      return 0;
    }

    //eadk_timing_msleep(20); // We want speed!
  }
  return 0;
}
