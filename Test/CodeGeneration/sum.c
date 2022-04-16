
int main()
{
  /*
    Read two digits and sum them.
    No spaces between the digits on input.
    No error checking: if the two characters
    are not digits, we get interesting results.
  */
  int d1, d2, sum;
  d1 = getchar() - 48;
  d2 = getchar() - 48;
  sum = d1 + d2;
  putchar(d1 + 48);
  putchar(43);
  putchar(d2 + 48);
  putchar(61);
  /*
    Print the two digit sum, one digit at a time.
    Leading digit might be zero, but there's no
    way to suppress that without using if.
  */
  putchar(sum / 10 + 48);
  putchar(sum % 10 + 48);
  putchar(10);
  return sum;   /* Produced Java bytecode will show this return value */
}
