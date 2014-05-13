high int a = 10;
int b;
if a > 1 {
  a = 1;
  // Can't have a declaration of a low var inside a high context
  //int c = 2;
  if (b < 0) { // Any expression can be typed as high (E-1)
    a = 1;     // Can only assign to high vars because we are still in a high context
               // even though b is not marked as high
    // This would fail:
    //b = 1;
  } else {
    a = 2;
  }
} else {
  a = 1;
}
a = 1;
