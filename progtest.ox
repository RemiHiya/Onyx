extern analysisTest*;
extern stdlib*;

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
    print(1);
    return test.testMethod(1);
}