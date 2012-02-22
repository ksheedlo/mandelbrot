/* Ken Sheedlo
 * Simple Mandelbrot renderer. */

#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<math.h>
#include<complex.h>
#include<pthread.h>
#include<alloca.h>

#define PNG_CAPABLE 1

#ifdef PNG_CAPABLE
#include<cv.h>
#include<highgui.h>
#endif

#define MAX_ITERATIONS  1000
#define SET_MEMBER      MAX_ITERATIONS + 1
#define MAX_REAL        1.0
#define MIN_REAL        (-2.0)
#define MIN_IMAG        (-1.2)
#define MAX_IMAG        1.2

#define BLACK   0x0
#define WHITE   0xFFFFFF
#define NCOLORS_RED     8192
#define RCOLOR_DENSITY  (NCOLORS_RED / ((double)MAX_ITERATIONS + 1))
#define RED_CUTOFF      245

#define NCOLORS_GCY     8192
#define GCCOLOR_DENSITY (NCOLORS_GCY / ((double)MAX_ITERATIONS + 1))
#define GRN_CUTOFF      245
#define CYAN_CUTOFF     580
#define WHITE_CUTOFF    1024

#define NCOLORS_BGY     8192
#define BGY_DENSITY     (NCOLORS_BGY / ((double)MAX_ITERATIONS + 1))
#define BGY_OFFSET      280
#define BGY_GRN_OFFSET  300
#define BGY_YLW_OFFSET  320
#define BGY_TOP         8000

#define W_SET   1
#define H_SET   2
#define P_SET   4
#define R_SET   8
#define I_SET   16

#define RGBA_R  0
#define RGBA_G  1
#define RGBA_B  2
#define RGBA_A  3

#define OUTPUT_PNG  0
#define OUTPUT_MBT  1

#define MBT_MINTHREADS  1
#define MBT_MAXTHREADS  64
#define MBT_DEFTHREADS  2

#define _MIN(a,b)       ((a) < (b) ? (a) : (b))
#define _MAX(a,b)       ((a) < (b) ? (b) : (a))

typedef struct {
    double x, y, width, height;
} rectd_t;

typedef union {
    uint8_t rgba[4];
    int32_t color;
} color_u;

typedef struct {
    int32_t *pixbuf;
    rectd_t *rect;
    int32_t (*colorfunc)(int32_t, double);
    size_t width;
    size_t height;
    int32_t ylow, yhigh;
} renderarg;

int32_t mandelbrot_series(double complex x, double *result);

void render(int32_t *pixbuf, size_t width, size_t height, rectd_t *rect,
    int32_t (*colorfunc)(int32_t, double));

void *render_worker(void *threadarg);

int32_t color_bw(int32_t iterations, double z);

int32_t color_red(int32_t iterations, double z);

#ifdef PNG_CAPABLE
void write_png(int32_t *pixbuf, const char *filename, size_t width, size_t height);
#endif

void rgba_to_bgr(void *rgba, void *bgr, size_t bytes);

void write_mbt(int32_t *pixbuf, const char *filename, size_t width, size_t height);

void rgba_to_rgb(void *rgba, void *rgb, size_t bytes);

