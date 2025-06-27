//
// Created by remsc on 07/06/2025.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <string>

#include "Lexer.h"


class Logger {

public:
    static void Log(const std::string &str);
    static void Error(const std::string &str);
    static void Report(const Token& token, const std::string& message);
};



#endif //LOGGER_H
