high int a = 10;
high int b;
if a > 1 {
  b = 2;
} else {
  a = 1;
  int c;
  skip;
  if (c==0) {
    // The low if(c==0) does not override the enclosing high if
    // Thus, we are still in a high context 
    // and the assignment below must fail
    c = 1;
  } else {
    skip;
  }
}
a = 2;
