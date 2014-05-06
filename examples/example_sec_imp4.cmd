high int a = 10;
high int b;
int c = 3;
if a > 1 {
  a = 1;
} else {
  // b is now declared as high, so this should work
  b = 2;
}
a = 2;
// This should work because we are outside a high context
c = 1;
