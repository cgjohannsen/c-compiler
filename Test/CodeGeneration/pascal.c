
/*
 * Kind of a stress test.
 * All this ugliness would go away though if we had loops.
 */

int row[9];

void show(int x)
{
    putchar(x/10 + 48);
    putchar(x%10 + 48);
    putchar(32);
}

void show2(int x, int y)
{
    show(x);
    show(y);
}

void show5(int a, int b, int c, int d, int e)
{
    show2(a, b);
    show(c);
    show2(d, e);
}

void show_row()
{
    show2(row[0], row[1]);
    show5(row[2], row[3], row[4], row[5], row[6]);
    show2(row[7], row[8]);
    putchar(10);
}

int update(int add, int i)
{
    int old;
    old = row[i];
    row[i] += add;
    return old;
}

void update_row()
{
    update(update(update(update(update(update(update(update(1, 1), 2), 3), 4), 5), 6), 7), 8);
}

int main()
{
    row[0] = 1;
    row[1] = row[2] = row[3] = row[4] = row[5] = row[6] = row[7] = row[8] = 0;

    show_row();
    update_row(); show_row();
    update_row(); show_row();
    update_row(); show_row();
    update_row(); show_row();
    update_row(); show_row();
    update_row(); show_row();
    update_row(); show_row();
    update_row(); show_row();
    return row[0];
}
