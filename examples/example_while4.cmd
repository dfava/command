high int a = 0;
int b = 0;
while a < 10 {  // High context
  if (b == 0) { // Still high context
    b = b + 1;
  } else {
    skip;
  }
}
