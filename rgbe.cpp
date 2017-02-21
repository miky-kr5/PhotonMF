#include <cmath>

#include "rgbe.hpp"

/* standard conversion from float pixels to rgbe pixels */
/* note: you can remove the "inline"s if your compiler complains about it */
void float2rgbe(unsigned char rgbe[4], float red, float green, float blue) {
  float v;
  int e;

  v = red;
  if (green > v) v = green;
  if (blue > v) v = blue;
  if (v < 1e-32) {
    rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
  }
  else {
    v = frexp(v,&e) * 256.0/v;
    rgbe[0] = (unsigned char) (red * v);
    rgbe[1] = (unsigned char) (green * v);
    rgbe[2] = (unsigned char) (blue * v);
    rgbe[3] = (unsigned char) (e + 128);
  }
}

/* standard conversion from rgbe to float pixels */
/* note: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels */
/*       in the range [0,1] to map back into the range [0,1].            */
void rgbe2float(float & red, float & green, float & blue, unsigned char rgbe[4]) {
  float f;

  if (rgbe[3]) {   /*nonzero pixel*/
    f = ldexp(1.0,rgbe[3]-(int)(128+8));
    red = rgbe[0] * f;
    green = rgbe[1] * f;
    blue = rgbe[2] * f;
  }
  else
    red = green = blue = 0.0;
}
