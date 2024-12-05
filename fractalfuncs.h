#define conx -0.74         // Real constant, horizontal axis (x)
#define cony 0.1             // Imaginary constant, vertial axis (y)
#define Maxx 2.0             // Rightmost Real point of plane to be displayed
#define Minx -2.0            // Leftmost Real point
#define  Maxy 1.0             // Uppermost Imaginary point
#define  Miny -1.0            // Lowermost Imaginary point
#define initer 50          // # of times to repeat function


typedef struct Pixel
{
    float r,g,b;
} PIXEL;

struct IMG {
    PIXEL * pixels;
    int cols,rows;
};

void putpixel(struct IMG * img,int x, int y, int color);

void julia(struct IMG *, int xpt, int ypt, int col);

void saveimg(struct IMG * img, char * fname);