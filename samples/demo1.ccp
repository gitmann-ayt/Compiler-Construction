// demo1.ccp - sample input for the integrated pipeline

int n;
int i;
float acc;

n = 5;
i = 1;
acc = 1;

while (i <= n) {
  acc = acc * i;
  i = i + 1;
}

if (acc > 10) {
  acc = log(acc) + exp(1);
} else {
  acc = acc ^ 2;
}

return acc;
