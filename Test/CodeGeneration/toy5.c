
int a;

void func(int b, int c)
{
    a += b*c;
    return;
    /* This code is dead; could be eliminated */
    a = 2;
    return;
}

int main()
{
    a = 3;
    func(3+1, 20/4);
    return a;
}
