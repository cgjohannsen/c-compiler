
int a;

int func(int b, int c)
{
    return a+b*c;
}

int main()
{
    a = 3;
    return func(3+1, 20/4);
}
