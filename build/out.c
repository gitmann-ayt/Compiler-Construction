#include <math.h>

int main(void) {
  int n;
  int i;
  double acc;
  n = 5;
  i = 1;
  acc = 1;
  while ((i <= n))
  {
    acc = (acc * i);
    i = (i + 1);
  }
  if ((acc > 10))
  {
    acc = (log(acc) + exp(1));
  }
  else
  {
    acc = pow(acc, 2);
  }
  return acc;
  return 0;
}
