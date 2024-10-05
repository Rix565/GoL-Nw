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

#define BYTES_NEEDED BITNSLOTS(WIDTH*HEIGHT)


const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Game Of Life";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

char front[BYTES_NEEDED];
char back[BYTES_NEEDED];


int main(int argc, char * argv[]) {
  for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        BITCLEAR(front, x+y*WIDTH);
        eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x*PIXEL_SIZE), (uint16_t)(y*PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_white);
      }
  }

  for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        if (eadk_random() > 1<<31) {
          BITSET(front, x+y*WIDTH);
          eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x*PIXEL_SIZE), (uint16_t)(y*PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_black);
        }
      }
  }

  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);
  
  while (true) {
    memcpy(back, front, BYTES_NEEDED);

    for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        int count = 0;

        for (int v = 0; v < 3; v++) {
          for (int w = 0; w < 3; w++) {
            int xx = (x+v-1) % WIDTH;
            int yy = (y+w-1) % HEIGHT;

            if (xx==x && yy==y) continue;
            
            if (BITTEST(back, xx+yy*WIDTH)) count++;
          }
        }

        if ((count < 2) || (count > 3)) {
          BITCLEAR(front, x+y*WIDTH);
          eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x*PIXEL_SIZE), (uint16_t)(y*PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_white);
        } else if (count == 3) {
          BITSET(front, x+y*WIDTH);
          eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x*PIXEL_SIZE), (uint16_t)(y*PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_black);
        }
      }
    }

    eadk_keyboard_state_t keyboardState = eadk_keyboard_scan();

    if (eadk_keyboard_key_down(keyboardState, eadk_key_home)) {
      return 0;
    }

    eadk_timing_msleep(20);
  }
  return 0;
}
