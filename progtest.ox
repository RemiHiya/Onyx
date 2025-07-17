extern analysisTest*;
extern stdlib*;

struct Parent {
    int a;
    float b;
}

struct other {
    int o;
}

extends other {
}

extends Parent {
    int testMethod(other oth) {
        int tmp = this.a;
        oth.o = this.a;
        return oth.o;
    }
}

int main() {
    Parent test = Parent(42, 0.69);
    other o = other(666);
    test.testMethod(o);
    return o.o;
}