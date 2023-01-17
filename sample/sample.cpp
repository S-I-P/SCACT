
#include <stdio.h>
#define ss struct stc

struct stc{
    int struct_field_1;
    char struct_field_2;
    void inc_func() {
        struct_field_1++;
    }
};

enum enm {e1, e2, e3};

class cls
{
public:
    int class_field_1;
    float class_field_2=1.5;
    cls() {
        class_field_1=0;
    }
    cls(int c) {
        class_field_1=c;
    }
    void func(int c) {
        class_field_1=c;
    }
    ~cls() {
        class_field_2=0;
    }
};

struct {
    const double anon_struct_field = 1.2;
} anon;

typedef cls cs;

void print(ss stc_) {
    printf("%d %c\n", stc_.struct_field_1, stc_.struct_field_2);
}

void print(cls cl, enm en) {
    printf("%d %d\n", cl.class_field_1, en);
}

int main(int argc, char const *argv[])
{
    int var1 = 1;
    cs class_var;
    ss struct_var;
    struct_var.struct_field_1 = 2;
    struct_var.struct_field_2 = 's';
    enum enm enum_var = e3;
    class_var.func(var1);
    print(struct_var);
    print(class_var, enum_var);
    printf("%lf\n", anon.anon_struct_field);
    return 0;
}