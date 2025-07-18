extern analysisTest;
extern stdlib;

struct List<T> {
    T val;
}

extends List {}

int main() {
    List<int> test = List(1);
    return 0;
}