
int foo(int x, float y)
{
    x;
    y;
    x+3;
    2.5-y;
    x*(int)y;
    return x ? foo(x-1, y) : --x;
}

int bar()
{
    foo(3); /* type error */
}
