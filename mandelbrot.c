/* Ken Sheedlo
 * Simple Mandelbrot renderer implementation. */

#include "mandelbrot.h"

int32_t mandelbrot_series(double complex x, double *result) {
  double complex z = x;
  for (int i = 0; i < MAX_ITERATIONS; i++) {
    z = (z * z) + x;
    double abs_z = creal(z * conj(z));
    if (abs_z > 4.0) {
      *result = abs_z;
      return (int32_t)(i+1);
    }
  }
  return SET_MEMBER;
}

void render(int32_t *pixbuf,
            size_t width,
            size_t height,
            rectd_t *rect,
            int32_t (*colorfunc)(int32_t, double)) {
  double rmax = rect->x + rect->width;
  double imax = rect->y + rect->height;
  double rmin = rect->x;
  double real_factor = (rmax - rect->x) / (width - 1);
  double imag_factor = (imax - rect->y) / (height - 1);

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      double result = 0;
      double complex c = (rmin + (x * real_factor)) + ((imax - (y * imag_factor)) * I);
      int iterations = mandelbrot_series(c, &result);
      pixbuf[y*width + x] = colorfunc(iterations, result);
    }
  }
}

int32_t color_bw(int32_t iterations, double z) {
  if (iterations == SET_MEMBER) {
    return BLACK;
  }
  return WHITE;
}

int32_t color_red(int32_t iterations, double z) {
  if (iterations == SET_MEMBER) {
    return BLACK;
  }
  int32_t r, g, b;
  double v = iterations - (log(log(z) / log(4)) / log(2));
  int32_t color = (int32_t)(v * RCOLOR_DENSITY);
  if (color < RED_CUTOFF) {
    r = (256*color) / RED_CUTOFF;
    g = b = 0;
  } else {
    r = 0xFF;
    g = b = 256 * (color - RED_CUTOFF) / (NCOLORS_RED - RED_CUTOFF);
  }
  return (b << 16) | (g << 8) | r;
}

int32_t color_green_cyan(int32_t iterations, double z) {
  if (iterations == SET_MEMBER) {
    return BLACK;
  }
  int32_t r, g, b;
  double v = iterations - (log(log(z) / log(4)) / log(2));
  int32_t color = (int32_t)(v * GCCOLOR_DENSITY);

  if (color < GRN_CUTOFF) {
    g = (256 * color) / GRN_CUTOFF;
    r = b = 0;
  } else if (color < CYAN_CUTOFF) {
    r = 0;
    g = 0xFF;
    b = 256 * (color - GRN_CUTOFF) / (CYAN_CUTOFF - GRN_CUTOFF);
  } else if (color < WHITE_CUTOFF) {
    b = g = 0xFF;
    r = 256 * (color - CYAN_CUTOFF) / (WHITE_CUTOFF - CYAN_CUTOFF);
  } else {
    return WHITE;
  }

  return (b << 16) | (g << 8) | r;
}

int32_t color_bgy(int32_t iterations, double z) {
  if (iterations == SET_MEMBER) {
    return BLACK;
  }
  int32_t r = 0, g = 0, b = 0;
  double v = iterations - (log(log(z) / log(4)) / log(2));
  int32_t color = (int32_t)(v * BGY_DENSITY);
  int32_t x = BGY_OFFSET;

  for (int i = 0;x < BGY_TOP;i++) {
    int32_t xdiff = color - x;
    int32_t xnorm = 10000 + xdiff*xdiff;
    int32_t w;
    switch(i % 3) {
      case 0:
        //Blue
        b += (2550000 / xnorm);
        x += BGY_GRN_OFFSET;
        break;
      case 2:
        //White
        w = (2550000 / xnorm);
        g += w;
        r += w;
        b += w;
        x += BGY_OFFSET;
        break;
      case 1:
        //Yellow
        r += (2550000 / xnorm);
        g += (2050000 / xnorm);
        x += BGY_YLW_OFFSET;
        break;
    }
  }
  x = color - NCOLORS_BGY;
  int32_t white = 2550000 / (10000 + (x*x));
  r += white;
  g += white;
  b += white;

  r = _MIN(r,0xFF);
  g = _MIN(g,0xFF);
  b = _MIN(b,0xFF);

  return (b << 16) | (g << 8) | r;
}

#ifdef PNG_CAPABLE
void write_png(int32_t *pixbuf, const char *filename, size_t width, size_t height) {
  CvSize size;
  size.width = width;
  size.height = height;

  char *bgrbuf = malloc(width * height * 3 * sizeof(char));
  rgba_to_bgr(pixbuf, bgrbuf, width * height);

  IplImage* image = cvCreateImage(size, IPL_DEPTH_8U, 3);
  cvSetData(image, bgrbuf, width * 3);

  cvSaveImage(filename, image, NULL);

  free(bgrbuf);
}
#endif

void rgba_to_bgr(int32_t *rgba, char *bgr, size_t entries) {
  for (uint32_t i = 0; i < entries; i++) {
    bgr[3*i]     = (rgba[i] & 0xFF0000) >> 16;
    bgr[3*i + 1] = (rgba[i] & 0xFF00) >> 8;
    bgr[3*i + 2] = rgba[i] & 0xFF;
  }
}

void write_mbt(int32_t *pixbuf, const char *filename, size_t width, size_t height) {
  int32_t sizebuf[2];
  sizebuf[0] = width;
  sizebuf[1] = height;

  FILE *output = fopen(filename, "wb");
  if (output == NULL) {
    return;
  }

  size_t rgblen = width * height * 3;
  char *rgbbuf = malloc(rgblen * sizeof(char));
  if (rgbbuf == NULL) {
    fclose(output);
    return;
  }

  rgba_to_rgb(pixbuf, rgbbuf, rgblen);
  fwrite(sizebuf, sizeof(sizebuf), 1, output);
  fwrite(rgbbuf, sizeof(char), rgblen, output);

  fclose(output);
  free(rgbbuf);
}

void rgba_to_rgb(void *rgba, void *rgb, size_t bytes) {
  int32_t *rgba_ptr = (int32_t *)rgba;
  char *rgb_ptr = (char *)rgb;
  size_t max_i = bytes / 3;
  color_u color;

  for (uint32_t i = 0; i < max_i; i++) {
    color.color = *rgba_ptr++;
    rgb_ptr[0] = color.rgba[RGBA_R];
    rgb_ptr[1] = color.rgba[RGBA_G];
    rgb_ptr[2] = color.rgba[RGBA_B];
    rgb_ptr += 3;
  }
}

int main(int argc, char **argv) {
  char *filename = NULL;
  char c;
  extern char *optarg;

  int width = 1250;
  int height = 1000;
  int flags = 0;

#ifdef PNG_CAPABLE
  int32_t output_format = OUTPUT_PNG;
  int32_t name_len;
  char ext[5];
#endif

  double rmin = MIN_REAL;
  double rmax = MAX_REAL;
  double imin = MIN_IMAG;
  double imax = MAX_IMAG;
  rectd_t rect;

  while((c = getopt(argc, argv, "o:w:h:p:r:i:")) != -1) {
    double r, i;
    switch(c) {
      case 'o':
        filename = strdup(optarg);
#ifdef PNG_CAPABLE
        name_len = strlen(filename);
        strncpy(ext, filename + (name_len - 4), 5);
        if (!strcmp(ext, ".mbt")) {
          output_format = OUTPUT_MBT;
        }
#endif
        break;
      case 'w':
        width = atoi(optarg);
        flags = flags | W_SET;
        if (!(flags & H_SET)) {
          //height = (4 * width) / 5;
          height = (int32_t)round((imax - imin) * width / 
              (rmax - rmin));
        }
        break;
      case 'h':
        height = atoi(optarg);
        flags = flags | H_SET;
        if (!(flags & W_SET)) {
          //width = (5 * height) / 4;
          width = (int32_t)round((rmax - rmin) * height / 
              (imax - imin));
        }
        break;
      case 'p':
        sscanf(optarg, "(%lf,%lf)", &r, &i);
        flags = flags | P_SET;
        imax += (i - imin);
        rmax += (r - rmin);
        rmin = r;
        imin = i;
        break;
      case 'r':
        r = rmax;
        rmax = rmin + atof(optarg);
        flags = flags | R_SET;
        if (!(flags & W_SET)) {
          width = (int32_t)round(((rmax - rmin)/ (r - rmin)) * width);
        }
        break;
      case 'i':
        i = imax;
        imax = imin + atof(optarg);
        flags = flags | I_SET;
        if (!(flags & H_SET)) {
          height = (int32_t)round(((imax - imin)/ (i - imin)) * height);
        }
        break;
    }
  }

  rect.x = rmin;
  rect.y = imin;
  rect.width = rmax - rmin;
  rect.height = imax - imin;
  fprintf(stderr, "start re : %lf\timag: %lf\n", rmin, imin);
  fprintf(stderr, "finish re: %lf\timag: %lf\n", rmax, imax);
  fprintf(stderr, "width: %dpx\theight: %dpx\n", width, height);

  if (filename == NULL) {
    fprintf(stderr, "Must provide an output file.\n");
    return 1;
  }

  int32_t *pixbuf = malloc(width * height * sizeof(int32_t));
  if (pixbuf == NULL) {
    fprintf(stderr, "Memory allocation failure.\n");
    return 1;
  }

  render(pixbuf, width, height, &rect, color_bgy);
#ifdef PNG_CAPABLE
  if (output_format == OUTPUT_PNG) {
    write_png(pixbuf, filename, width, height);
  } else {
    write_mbt(pixbuf, filename, width, height);
  }
#else
  write_mbt(pixbuf, filename, width, height);
#endif

  free(pixbuf);
  return 0;
}
