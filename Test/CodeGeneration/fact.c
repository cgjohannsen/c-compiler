
int F;

void print(int i, int f)
{
    putchar(i+(int)'0');
    putchar((int)'!');
    putchar((int)' ');
    putchar((int)'=');
    putchar((int)' ');

    putchar(f / 100000 + 48);
    f = f % 100000;
    putchar(f / 10000 + 48);
    f = f % 10000;
    putchar(f / 1000 + 48);
    f = f % 1000;
    putchar((int)',');
    putchar(f / 100 + 48);
    f = f % 100;
    putchar(f / 10 + 48);
    f = f % 10;
    putchar(f + 48);
    putchar(10);
}

int main()
{
    F=1;
    print(1, F);
    print(2, F*=2);
    print(3, F*=3);
    print(4, F*=4);
    print(5, F*=5);
    print(6, F*=6);
    print(7, F*=7);
    print(8, F*=8);
    print(9, F*=9);
    return F;
}
