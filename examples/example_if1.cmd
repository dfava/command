int a = 10;
int b;
if a > 1 {
  b = 2;
  a = 1;
  int c = 2;
} else {
  a = 1;
}
a = 1;
// This c is not in scope
c = 1;
