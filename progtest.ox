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

Child extends Parent {
    int c;

}

int test(int a, int b) {
    return a+b;
}

int main() {
    return test(1, 2);
}