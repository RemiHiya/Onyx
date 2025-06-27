#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Lexer.h"
#include "Onyx.h"
#include "Parser.h"

int main() {


    Onyx().Compile("./progtest.ox");

    return 0;
}
