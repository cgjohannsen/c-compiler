int ok()
{
  /* Empty functions are allowed! */
}

int printf(int n)   // Because we can
{
  int i;
  for (i=0;;i++) {
    n/=2;
    if (n) continue;
    return i;
  }
  for (;;) {
    return;
  }
}

int too_many_elses(int a, int b)
{
  if (a<b) {
    return 1;
  } else {
    return 2;
  } else {      /* This should cause a syntax error */
    return 3;
  }
}

