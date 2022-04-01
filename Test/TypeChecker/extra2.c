
int a=0;

int bar(int const c)
{
    c;
    5;
    c+5;
    return c;
}

int foo(int b)
{
    const int c=5, d=b, e=bar(b);
    int f=c;
    c;
    d;
    c+d;
    f=c*e;
    return c+d+e;
}

