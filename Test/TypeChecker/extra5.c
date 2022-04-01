
struct thing {
    char string[128];
    int len;
};

int whatever(struct thing A[], int n)
{
    return A[n].len;
}

int foo()
{
    struct thing T;
    T;
    T.len;
    T.string;
    T.string[5];
    T.nope;
    T.string.nope;
    T.string[0].nope;
}

int bar(struct nope n)
{
    return 0;
}
