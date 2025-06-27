//
// Created by remsc on 07/06/2025.
//

#include "Logger.h"

#include <iostream>

void Logger::Log(const std::string &str) {
    std::cout << "Log : " << str << std::endl;
}

void Logger::Error(const std::string &str) {
    std::cerr << "Error : " << str << std::endl;
}

void Logger::Report(const Token &token, const std::string &message) {

    const std::string ANSI_RED = "\033[31m";
    const std::string ANSI_RESET = "\033[0m";
    const std::string ANSI_BLUE = "\033[34m";

    const int numSize = static_cast<int>(std::to_string(token.line).length());
    std::cerr << "" << "todo:filename" << "@" << token.line << ":" << token.col << "" << std::endl;
    std::cerr << std::string(numSize, ' ') << " |" << std::endl;

    std::cerr << token.line << " | " << "todo:print the actual line" << std::endl;
    std::cerr << std::string(numSize, ' ') << " | " << std::string(token.col - 1, ' ') << "" << "^";
    if (token.value.length() > 1) {
        std::cerr << std::string(token.value.length() - 1, '~');
    }
    std::cerr << " " << message << "" << std::endl;
}
