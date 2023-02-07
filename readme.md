MLX90640 display with bicubic interpolation, modified from Adafruit code. Processor used was Teensy 3.2, Arduino IDE.
Display used is Adafruit HX8357D, 480x32 https://www.adafruit.com/product/2050

Some sample heatmap lookup tables are included. Simply replace the lookup table initialization in the .ino file with the desired heatmap. Also, check the transformation that scales the temperature range in the image to the heatmap, so that the desired number of table entries is produced, and capped so that the table boundaries are not exceeded. The popular "ironbow" heatmap lookup table has 433 entries.
