high int a = 10;
int b;
if a > 1 {
  // This must fail because b is low, and we are in a high context
  b = 2;
} else {
  a = 1;
}
a = 2;
