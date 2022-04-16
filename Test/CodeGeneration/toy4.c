
int a;

void func(int b, int c)
{
    a += b*c;
}

int main()
{
    a = 3;
    func(3+1, 20/4);
    return a;
}
