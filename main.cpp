#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

class Calculator {
public:
    double calculate(double a, double b, char op) {
        switch (op) {
            case '+':
                return a + b;
            case '-':
                return a - b;
            case '*':
                return a * b;
            case '/':
                if (b == 0) {
                    throw std::runtime_error("Division by zero!");
                }
                return a / b;
            default:
                throw std::runtime_error("Invalid operator!");
        }
    }
};

double parseNumber(const std::string& str) {
    std::istringstream iss(str);
    double num;
    if (!(iss >> num)) {
        throw std::runtime_error("Invalid number: " + str);
    }
    return num;
}

bool parseExpression(const std::string& input, double& num1, char& op, double& num2) {
    // 공백 제거
    std::string cleaned;
    for (char c : input) {
        if (c != ' ' && c != '\t') {
            cleaned += c;
        }
    }
    
    if (cleaned.empty()) {
        return false;
    }
    
    // 연산자 찾기
    size_t opPos = std::string::npos;
    char operators[] = {'+', '-', '*', '/'};
    
    for (char opChar : operators) {
        size_t pos = cleaned.find(opChar);
        if (pos != std::string::npos && pos > 0 && pos < cleaned.length() - 1) {
            // 음수 처리를 위해 '-'는 첫 번째 문자가 아니어야 함
            if (opChar == '-' && pos == 0) {
                continue;
            }
            opPos = pos;
            op = opChar;
            break;
        }
    }
    
    if (opPos == std::string::npos) {
        return false;
    }
    
    // 숫자 파싱
    try {
        std::string num1Str = cleaned.substr(0, opPos);
        std::string num2Str = cleaned.substr(opPos + 1);
        
        num1 = parseNumber(num1Str);
        num2 = parseNumber(num2Str);
        return true;
    } catch (...) {
        return false;
    }
}

int main() {
    Calculator calc;
    std::string input;
    
    std::cout << "=== C++ CLI Calculator ===" << std::endl;
    std::cout << "Enter expressions like: 5+3 or 5 + 3" << std::endl;
    std::cout << "Type 'quit' or 'exit' to exit" << std::endl;
    std::cout << std::endl;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "quit" || input == "exit" || input == "q") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        try {
            double num1, num2;
            char op;
            
            if (!parseExpression(input, num1, op, num2)) {
                std::cout << "Error: Invalid format. Use: number operator number (e.g., 5+3 or 5 + 3)" << std::endl;
                continue;
            }
            
            double result = calc.calculate(num1, num2, op);
            std::cout << "Result: " << result << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
