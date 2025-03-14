typedef struct {
    int width;
    int height;
    int* data;
} Framebuffer;

void setFb(Framebuffer fb);
void plot(int x, int y, int color);
void rectangle(int x, int y, int width, int height, int color);
void plotStruct(int x, int y, int color, Framebuffer fb);
void rectangleStruct(int x, int y, int width, int height, int color, Framebuffer fb);