#include "eadk.h"
#include <string.h>

#define WIDTH 160
#define HEIGHT 120

#define PIXEL_SIZE 2


const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Game Of Life";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

bool front[WIDTH*HEIGHT];
bool back[WIDTH*HEIGHT];

int main(int argc, char * argv[]) {
  for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        front[x+y*WIDTH] = false;
        eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x*PIXEL_SIZE), (uint16_t)(y*PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_white);
      }
  }

  for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        if (eadk_random() > 1<<31) {
          front[x+y*WIDTH] = true;
          eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x*PIXEL_SIZE), (uint16_t)(y*PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_black);
        }
      }
  }

  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);
  
  while (true) {
    memcpy(back, front, WIDTH*HEIGHT);

    for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        int count = 0;

        for (int v = 0; v < 3; v++) {
          for (int w = 0; w < 3; w++) {
            int xx = (x+v-1) % WIDTH;
            int yy = (y+w-1) % HEIGHT;

            if (xx==x && yy==y) continue;
            
            if (back[xx+yy*WIDTH]) count++;
          }
        }

        if ((count < 2) || (count > 3)) {
          front[x+y*WIDTH] = false;
          eadk_display_push_rect_uniform((eadk_rect_t){(uint16_t)(x*PIXEL_SIZE), (uint16_t)(y*PIXEL_SIZE), (uint16_t)(PIXEL_SIZE), (uint16_t)(PIXEL_SIZE)}, eadk_color_white);
        } else if (count == 3) {
          front[x+y*WIDTH] = true;
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
