#include "eadk.h"
#include <string.h>
#include <limits.h>

// Map settings
#define WIDTH 160
#define HEIGHT 120

#define PIXEL_SIZE 2

// BitSet tools
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

#define BYTES_NEEDED BITNSLOTS(WIDTH *HEIGHT)

// Draw a pixel with the given color at the given coordinates
#define draw_pix(cx, cy, color) eadk_display_push_rect_uniform((eadk_rect_t){cx * PIXEL_SIZE, cy * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE}, color);

// Numworks apps specific headers
const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Game Of Life";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

// Map and counters
char map_bitset[BYTES_NEEDED];
char counters[WIDTH * HEIGHT];
char counters2[WIDTH * HEIGHT];

// Function to increment counters
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

// Function to decrement counters
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
  // Clear the screen and the map
  for (uint16_t x = 0; x < WIDTH; x++)
  {
    for (uint16_t y = 0; y < HEIGHT; y++)
    {
      BITCLEAR(map_bitset, x + y * WIDTH);
      counters[x + y * WIDTH] = 0;
      draw_pix(x, y, eadk_color_white);
    }
  }

  // Randomize the cells
  for (uint16_t x = 0; x < WIDTH; x++)
  {
    for (uint16_t y = 0; y < HEIGHT; y++)
    {
      if (eadk_random() > 1 << 31)
      {
        BITSET(map_bitset, x + y * WIDTH);
        set_at(x, y);
        draw_pix(x, y, eadk_color_black);
      }
    }
  }

  // Main loop
  while (true)
  {
    // We need another array to avoid interferences between generations
    memcpy(counters2, counters, WIDTH * HEIGHT);

    for (uint16_t x = 0; x < WIDTH; x++)
    {
      for (uint16_t y = 0; y < HEIGHT; y++)
      {
        int count = counters2[x + y * WIDTH];

        if ((count < 2) || (count > 3)) // The cell dies
        {
          if (BITTEST(map_bitset, x + y * WIDTH))
          {
            clear_at(x, y);
          };
          BITCLEAR(map_bitset, x + y * WIDTH);
          draw_pix(x, y, eadk_color_white);
        }
        else if (count == 3) // The cell become alive
        {
          if (!BITTEST(map_bitset, x + y * WIDTH))
          {
            set_at(x, y);
          }
          BITSET(map_bitset, x + y * WIDTH);
          draw_pix(x, y, eadk_color_black);
        }
      }
    }

    // Handle the keyboard events
    eadk_keyboard_state_t keyboardState = eadk_keyboard_scan();

    if (eadk_keyboard_key_down(keyboardState, eadk_key_home))
    {
      return 0; // Quit
    }

    // eadk_timing_msleep(20); // We want speed!
  }
  return 0;
}
