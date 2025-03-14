#include "graphics.h"

Framebuffer image;

void setFb(Framebuffer fb) {
    image = fb;
}

void plot(int x, int y, int color) {
  plotStruct(x, y, color, image);
}

void rectangle(int x, int y, int width, int height, int color) {
  rectangleStruct(x, y, width, height, color, image);
}

void plotStruct(int x, int y, int color, Framebuffer fb) {
  if (x >= 0 && x < fb.width && y >= 0 && y < fb.height) {
    fb.data[y*fb.width + x] = color;
  }
}

void rectangleStruct(int x, int y, int width, int height, int color, Framebuffer fb) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      plotStruct(x + j, y + i, color, fb);
    }
  }
}