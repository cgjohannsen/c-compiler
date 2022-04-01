
struct pair {
    int x, y;
};

struct triangle {
    char color;
    struct pair vertex[3];
};

void foo()
{
    struct pair p, q;

    p;
    p.x;
    q.y / p.x;

    struct ornament {
        struct pair hook;
        int length;
        struct triangle faces[24];
    };

    struct ornament t;

    t;
    t.hook;
    t.length;
    t.faces;
    t.faces[5];
    t.faces[5].color;
    t.faces[5].vertex[1];
    t.faces[5].vertex[1].x;
}
