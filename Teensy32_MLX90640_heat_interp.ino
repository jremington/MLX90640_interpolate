#include <Adafruit_MLX90640.h>
Adafruit_MLX90640 mlx;
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
// control pin definitions for HX8357 display
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

//heatmap color table 565 format
// 433 entries
 uint16_t ironbow[]=
{1,2,3,4,5,5,6,6,7,7,
8,8,9,9,10,10,10,11,11,11,
12,12,12,12,13,2061,2061,2062,2062,2062,
2062,2062,2062,4111,4111,4111,4111,4111,6159,6160,
6160,6160,8208,8208,8208,8208,10257,10257,10257,10257,
12305,12305,12305,12305,14353,14354,14354,14354,14354,14354,
16402,16402,16402,16402,16402,18450,18450,18450,18450,18450,
20498,20499,20499,20499,22547,22547,22547,22547,22547,24595,
24595,24595,24595,26643,26643,26643,26643,26643,28691,28691,
28691,28691,28691,30739,30739,30739,30739,30739,32787,32787,
32787,32787,32787,34835,34835,34835,34835,34835,36883,36883,
36883,36883,38931,38931,38931,38931,38931,38931,40979,40979,
40979,40979,40979,40979,43027,43027,43027,43027,43027,43027,
43027,45075,45075,45074,45074,45074,45074,45074,45074,45074,
47122,47154,47154,47154,47154,47154,47154,47154,47154,47154,
49202,49202,49234,49234,49233,49233,49233,49265,49265,49265,
49265,49265,51345,51344,51344,51344,51344,51376,51376,51376,
51376,51407,51407,51407,53455,53455,53486,53486,53486,53486,
53518,53517,53517,53517,53549,53548,53548,55596,55628,55628,
55627,55659,55659,55658,55658,55690,55689,55689,55688,55720,
55720,55719,57767,57798,57798,57798,57797,57829,57828,57828,
57828,57859,57859,57859,57891,57891,57890,57890,57922,57922,
59970,60002,60001,60001,60001,60001,60001,60033,60033,60033,
60033,60065,60065,60065,60097,60096,60096,60096,60128,60128,
60128,60128,60128,60160,60160,60160,62208,62240,62240,62240,
62240,62240,62272,62272,62272,62272,62272,62304,62304,62304,
62304,62336,62336,62336,62336,62368,62368,62368,62368,62400,
62400,62400,62432,62432,62432,62464,62464,62464,62464,62496,
62496,62496,64544,64576,64576,64576,64576,64576,64608,64608,
64608,64608,64608,64640,64640,64640,64640,64672,64672,64672,
64704,64704,64704,64736,64736,64736,64768,64768,64768,64768,
64800,64800,64800,64832,64832,64832,64864,64864,64864,64864,
64896,64896,64896,64896,64928,64928,64928,64960,64960,64960,
64960,64960,64992,64992,64992,65024,65024,65024,65024,65056,
65056,65056,65056,65088,65088,65088,65088,65088,65120,65120,
65120,65120,65120,65152,65152,65153,65185,65185,65185,65185,
65217,65217,65217,65217,65218,65250,65250,65250,65251,65251,
65251,65284,65284,65284,65284,65285,65317,65317,65318,65318,
65319,65319,65351,65352,65352,65353,65353,65386,65386,65386,
65387,65387,65388,65388,65421,65421,65422,65422,65423,65424,
65424,65425,65425,65458,65458,65459,65459,65460,65460,65461,
65461,65462,65462,65495,65495,65496,65496,65496,65497,65497,
65498,65498,65499,65531,65531,65532,65532,65533,65533,65533,
65534,65534,65534};

// bicubic interpolation function templates
// interpolation code heavily modified by SJR from Adafruit's code in this tutorial:
// https://learn.adafruit.com/adafruit-amg8833-8x8-thermal-camera-sensor?view=all

float get_point(float *p, uint16_t rows, uint16_t cols, int16_t x, int16_t y);
void set_point(float *p, uint16_t rows, uint16_t cols, int16_t x, int16_t y, float f);
void get_adjacents_1d(float *src, float *dest, uint16_t rows, uint16_t cols, int16_t x, int16_t y);
void get_adjacents_2d(float *src, float *dest, uint16_t rows, uint16_t cols, int16_t x, int16_t y);
float cubicInterpolate(float p[], float x);
float bicubicInterpolate(float p[], float x, float y);
void interpolate_image(float *src, uint16_t src_rows, uint16_t src_cols,
                       float *dest, uint16_t dest_rows, uint16_t dest_cols);

// 480x320 pixel
#define H_RES 480
#define V_RES   320
#define INPUT_COLS 32
#define INPUT_ROWS 24
// 10x10 interp
#define INTERPOLATED_COLS 320
#define INTERPOLATED_ROWS 240
// 1x1 box pixel output
#define DISPLAY_COLS 320
#define DISPLAY_ROWS 240

float frame[32 * 24]; // buffer for full frame of temperatures
float dest_2d;  //output value for box fill
int boxsize; //pixel blocksize for output

void setup()
{
  tft.begin();
  tft.setRotation(1); // horizontal wide screen
  tft.fillScreen( HX8357_BLACK );

  //  tft.setCursor(0, 2);
  //  tft.setTextColor(HX8357_GREEN);  //tft.setTextSize(3);
  //  tft.setTextSize(1);
  //  tft.print("MLX90640");

  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("MLX90640 bicubic interpolate");
  while (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
  }
  Serial.println("Found MLX90640");

  //mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setMode(MLX90640_CHESS);
  Serial.print("Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS) {
    Serial.println("Chess");
  } else {
    Serial.println("Interleave");
  }

  mlx.setResolution(MLX90640_ADC_18BIT);
  Serial.print("Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res) {
    case MLX90640_ADC_16BIT: Serial.println("16 bit"); break;
    case MLX90640_ADC_17BIT: Serial.println("17 bit"); break;
    case MLX90640_ADC_18BIT: Serial.println("18 bit"); break;
    case MLX90640_ADC_19BIT: Serial.println("19 bit"); break;
  }

  mlx.setRefreshRate(MLX90640_2_HZ);
  Serial.print("Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate) {
    case MLX90640_0_5_HZ: Serial.println("0.5 Hz"); break;
    case MLX90640_1_HZ: Serial.println("1 Hz"); break;
    case MLX90640_2_HZ: Serial.println("2 Hz"); break;
    case MLX90640_4_HZ: Serial.println("4 Hz"); break;
    case MLX90640_8_HZ: Serial.println("8 Hz"); break;
    case MLX90640_16_HZ: Serial.println("16 Hz"); break;
    case MLX90640_32_HZ: Serial.println("32 Hz"); break;
    case MLX90640_64_HZ: Serial.println("64 Hz"); break;
  }
  boxsize = min(DISPLAY_COLS / INTERPOLATED_COLS, DISPLAY_ROWS / INTERPOLATED_ROWS);
  Serial.print("Box size = ");
  Serial.println(boxsize);
}

void loop(void)
{
  static int frame_number = 0;
  int n;
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  }

  Serial.print("frame "); Serial.println(frame_number++);

  // image stats, subtract min
  int npx = 32 * 24;
  float Tavg = 0.0;
  float Tmin = 1000.0;
  float Tmax = -1000.0;

  for (n = 0; n < npx; n++) {
    Tavg += frame[n];
    if (Tmin > frame[n]) Tmin = frame[n];
    if (Tmax < frame[n]) Tmax = frame[n];
  }
  Tavg = Tavg / npx;
  //  Serial.print("avg = "); Serial.println(Tavg);
  //  Serial.print("max = "); Serial.println(Tmax);
  //  Serial.print("min = "); Serial.println(Tmin);
  float scale = 425.0 / (Tmax - Tmin); //leave some headroom for bicubic interpolation (433 max)

  // scale image
  for (uint8_t h = 0; h < 24; h++) {
    for (uint8_t w = 0; w < 32; w++) {
      frame[h * 32 + w] = scale * (frame[h * 32 + w] - Tmin);
      //    Serial.print(frame[h*32+w],0); Serial.print(" ");
    }
    //   Serial.println();
  }

  interpolate_image(frame, INPUT_ROWS, INPUT_COLS, &dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);
}

// fetch the value at input array p(x, y) (cols X rows)
float get_point(float *p, uint16_t rows, uint16_t cols, int16_t x, int16_t y) {
  if (x < 0)        x = 0;
  if (y < 0)        y = 0;
  if (x >= cols)    x = cols - 1;
  if (y >= rows)    y = rows - 1;
  return p[y * cols + x];
}

// send blocks of points to destination array (rows X cols) (global boxsize)
void set_point(float *p, uint16_t rows, uint16_t cols, int16_t x, int16_t y, float f) {

  if ((x < 0) || (x >= cols)) return;
  if ((y < 0) || (y >= rows)) return;
  //    p[y * cols + x] = f;  //was, for output array

  // now, draw directly on screen

  int colorTemp = f;
  if (colorTemp < 0) colorTemp = 0;
  if (colorTemp > 432) colorTemp = 432;
  colorTemp = ironbow[colorTemp];

  if (boxsize == 1) tft.drawPixel(x, y, colorTemp);
  else   tft.fillRect(x * boxsize, y * boxsize, boxsize, boxsize, colorTemp);
}

// src is the input array src_rows * src_cols
// dest is a pre-allocated output array, dest_rows*dest_cols
void interpolate_image(float *src, uint16_t src_rows, uint16_t src_cols,
                       float *dest, uint16_t dest_rows, uint16_t dest_cols) {
  float mu_x = (src_cols - 1.0) / (dest_cols - 1.0);
  float mu_y = (src_rows - 1.0) / (dest_rows - 1.0);

  float adj_2d[16]; // 4x4 matrix for storing adjacents
  float frac_x, frac_y, out;

  for (uint16_t y_idx = 0; y_idx < dest_rows; y_idx++) {
    for (uint16_t x_idx = 0; x_idx < dest_cols; x_idx++) {
      float x = x_idx * mu_x;
      float y = y_idx * mu_y;
 
      get_adjacents_2d(src, adj_2d, src_rows, src_cols, x, y);

      frac_x = x - (int)x; // we only need the ~delta~ between the points
      frac_y = y - (int)y; // we only need the ~delta~ between the points
      out = bicubicInterpolate(adj_2d, frac_x, frac_y);
      set_point(dest, dest_rows, dest_cols, x_idx, y_idx, out);
    }
  }
}

// p is a list of 4 points, 2 to the left, 2 to the right
float cubicInterpolate(float p[], float x) {
  float r = p[1] + (0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0]))));

  return r;
}

// p is a 16-point 4x4 array of the 2 rows & columns left/right/above/below
float bicubicInterpolate(float p[], float x, float y) {
  float arr[4] = {0, 0, 0, 0};
  arr[0] = cubicInterpolate(p + 0, x);
  arr[1] = cubicInterpolate(p + 4, x);
  arr[2] = cubicInterpolate(p + 8, x);
  arr[3] = cubicInterpolate(p + 12, x);
  return cubicInterpolate(arr, y);
}

// src is rows*cols and dest is a 4-point array passed in already allocated!
void get_adjacents_1d(float *src, float *dest, uint16_t rows, uint16_t cols, int16_t x, int16_t y) {
  //    Serial.print("adj_1d("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.println(")");
  // pick two items to the left
  dest[0] = get_point(src, rows, cols, x - 1, y);
  dest[1] = get_point(src, rows, cols, x, y);
  // pick two items to the right
  dest[2] = get_point(src, rows, cols, x + 1, y);
  dest[3] = get_point(src, rows, cols, x + 2, y);
}


// src is rows*cols and dest is a 16-point array passed in, already allocated.
void get_adjacents_2d(float *src, float *dest, uint16_t rows, uint16_t cols, int16_t x, int16_t y) {
  //    Serial.print("adj_2d("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.println(")");
  for (int16_t delta_y = -1; delta_y < 3; delta_y++) { // -1, 0, 1, 2
    float *row = dest + 4 * (delta_y + 1); // index into each chunk of 4
    for (int16_t delta_x = -1; delta_x < 3; delta_x++) { // -1, 0, 1, 2
      row[delta_x + 1] = get_point(src, rows, cols, x + delta_x, y + delta_y);
    }
  }
}
