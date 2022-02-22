const float pi = 3.1415926535897932384626433; /* approximately */

struct point {
  int x, y;
};

struct rectangle {
  struct point upperleft;
  struct point lowerright;
};

struct window {
  struct rectangle area;
  char text[1024];
};

void display(const struct window W[], int n);

struct point strange(int z)
{
  /*
    Syntactically correct.
    Will fail type checking in part 4.
  */
  int y;
  struct mything {
    float a, b, c;
  };
  struct other A;

  display(y, A.b.c[15].d.e, F[15].g);

  for (;;) {
    if (y.x == y) return;
    y++;
    --y.x;
    break;
  }
  return A.x;
}
