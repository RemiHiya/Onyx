extern analysisTest*;

struct Parent {
    int a;
    float b;
}

extends Parent {
    int testMethod(int tt) {
        return a;
    }
}

Child extends Parent {
    int c;

}

int test(int a, int b) {
    return a+b;
}

int main() {
    Parent test = Parent(42, 0.69);
    test.testMethod(1);
    return ret;
}