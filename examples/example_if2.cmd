int a = 10;
int b;
if a > 1 {
  b = 2;
  a = 1;
  int c = 2;
  // This should fail because we shouldn't be able to redeclare a
  high int a = 5;
} else {
  a = 1;
}
a = 1;
