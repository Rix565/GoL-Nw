#include "eadk.h"
#include <string.h>
#include <limits.h>

// Map settings
#define WIDTH 160
#define HEIGHT 120

// BitSet tools
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

#define BYTES_NEEDED BITNSLOTS(WIDTH * HEIGHT)

// Draw a pixel with the given color at the given coordinates relatively to the camera
#define draw_pix_cam(cx, cy, color, ccam)                                                  \
  {                                                                                        \
    uint16_t rx = (cx - ccam.x) * ccam.zoom;                                               \
    uint16_t ry = (cy - ccam.y) * ccam.zoom;                                               \
    if (0 <= rx && rx < eadk_screen_rect.width && 0 <= ry && ry < eadk_screen_rect.height) \
      eadk_display_push_rect_uniform((eadk_rect_t){rx, ry, ccam.zoom, ccam.zoom}, color);  \
  }

// Numworks apps specific headers
const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Game Of Life";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

// Map and counters
char map_bitset[BYTES_NEEDED];
char counters[WIDTH * HEIGHT];
char counters2[WIDTH * HEIGHT];

// Camera
struct camera
{
  int x;
  int y;
  int z;

  int zoom;
};

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

#define redraw(ccam)                                  \
  for (uint16_t cx = 0; cx < WIDTH; cx++)             \
  {                                                   \
    for (uint16_t cy = 0; cy < HEIGHT; cy++)          \
    {                                                 \
      if (BITTEST(map_bitset, cx + cy * WIDTH))       \
        draw_pix_cam(cx, cy, eadk_color_black, ccam); \
    }                                                 \
  }

void clear_screen()
{
  eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)0, (uint16_t)0, (uint16_t)320, (uint16_t)240}, eadk_color_white);
}

int main(int argc, char *argv[])
{
  struct camera cam = {0, 0, 0, 2};

  // Clear the screen and the map
  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);
  for (uint16_t x = 0; x < WIDTH; x++)
  {
    for (uint16_t y = 0; y < HEIGHT; y++)
    {
      BITCLEAR(map_bitset, x + y * WIDTH);
      counters[x + y * WIDTH] = 0;
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
        draw_pix_cam(x, y, eadk_color_black, cam);
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
          draw_pix_cam(x, y, eadk_color_white, cam);
        }
        else if (count == 3) // The cell become alive
        {
          if (!BITTEST(map_bitset, x + y * WIDTH))
          {
            set_at(x, y);
          }
          BITSET(map_bitset, x + y * WIDTH);
          draw_pix_cam(x, y, eadk_color_black, cam);
        }
      }
    }

    // Handle the keyboard events
    eadk_keyboard_state_t keyboardState = eadk_keyboard_scan();

    if (eadk_keyboard_key_down(keyboardState, eadk_key_home))
    {
      return 0; // Quit
    }
    else if (eadk_keyboard_key_down(keyboardState, eadk_key_right))
    {
      cam.x += 1;
      clear_screen();
      redraw(cam);
    }
    else if (eadk_keyboard_key_down(keyboardState, eadk_key_left))
    {
      cam.x -= 1;
      clear_screen();
      redraw(cam);
    }
    else if (eadk_keyboard_key_down(keyboardState, eadk_key_up))
    {
      cam.y -= 1;
      clear_screen();
      redraw(cam);
    }
    else if (eadk_keyboard_key_down(keyboardState, eadk_key_down))
    {
      cam.y += 1;
      clear_screen();
      redraw(cam);
    }
    else if (eadk_keyboard_key_down(keyboardState, eadk_key_plus))
    {
      cam.zoom += 1;
      redraw(cam);
      eadk_timing_msleep(100);
    }
    else if (eadk_keyboard_key_down(keyboardState, eadk_key_minus))
    {
      cam.zoom -= 1;
      if (cam.zoom <= 0)
        cam.zoom = 1;
      redraw(cam);
      eadk_timing_msleep(100);
    }

    eadk_timing_msleep(20); // We want speed!
  }
  return 0;
}
