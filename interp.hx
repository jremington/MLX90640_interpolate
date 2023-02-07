#ifndef INTERPOLATE_H
#define INTERPOLATE_H

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

float get_point(float *p, uint16_t rows, uint16_t cols, int16_t x, int16_t y);
void set_point(float *p, uint16_t rows, uint16_t cols, int16_t x, int16_t y, float f);
void get_adjacents_1d(float *src, float *dest, uint16_t rows, uint16_t cols, int16_t x, int16_t y);
void get_adjacents_2d(float *src, float *dest, uint16_t rows, uint16_t cols, int16_t x, int16_t y);
float cubicInterpolate(float p[], float x);
float bicubicInterpolate(float p[], float x, float y);
void interpolate_image(float *src, uint16_t src_rows, uint16_t src_cols,
                       float *dest, uint16_t dest_rows, uint16_t dest_cols);

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
      // debug prints
      //       Serial.print("("); Serial.print(y_idx); Serial.print(", "); Serial.print(x_idx); Serial.print(") = ");
      //       Serial.print("("); Serial.print(y); Serial.print(", "); Serial.print(x); Serial.print(") => ");
      get_adjacents_2d(src, adj_2d, src_rows, src_cols, x, y);
      /*
             Serial.print("[");
             for (uint16_t i=0; i<16; i++) {
               Serial.print(adj_2d[i]); Serial.print(", ");
             }
             Serial.println("]");
      */
      frac_x = x - (int)x; // we only need the ~delta~ between the points
      frac_y = y - (int)y; // we only need the ~delta~ between the points
      out = bicubicInterpolate(adj_2d, frac_x, frac_y);
      //       Serial.print("\tInterp: "); Serial.println(out);
      set_point(dest, dest_rows, dest_cols, x_idx, y_idx, out);
    }
  }
}

// p is a list of 4 points, 2 to the left, 2 to the right
float cubicInterpolate(float p[], float x) {
  float r = p[1] + (0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0]))));
  /*
    Serial.print("interpolating: [");
    Serial.print(p[0],2); Serial.print(", ");
    Serial.print(p[1],2); Serial.print(", ");
    Serial.print(p[2],2); Serial.print(", ");
    Serial.print(p[3],2); Serial.print("] w/"); Serial.print(x); Serial.print(" = ");
    Serial.println(r);
  */
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
#endif
