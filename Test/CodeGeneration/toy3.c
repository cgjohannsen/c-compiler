
int a;

int func(int b, int c)
{
    b *= c;
    return a += b;
}

int main()
{
    a = 3;
    return func(3+1, 20/4);
}
