//extern analysisTest*;

struct Parent {
    int a;
    float b;
}

extends Parent {
    int testMethod(int tt) {
        return a;
    }
}

int main() {
    Parent test = Parent(42, 0.69);
    return test.testMethod(1);
}