
int a;

int func(int b, int c)
{
    a += b*c;
    // Should complain about missing return
}

int main()
{
    a = 3;
    func(3+1, 20/4);
    return a;
}
