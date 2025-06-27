//extern analysisTest*;

struct Parent {
    int a;
    float b;
}

extends Parent {
    void test(int a) {
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