#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <vector>
#include <cmath>
#include <cctype>

// ---------- 토큰 & 렉서 ----------
enum class TokenType { NUMBER, IDENT, OP, LPAREN, RPAREN, COMMA, ASSIGN, END };

struct Token {
    TokenType type;
    double value;
    std::string text;
};

class Lexer {
    std::string input;
    size_t pos = 0;
public:
    explicit Lexer(std::string s) : input(std::move(s)) {}
    Token next() {
        while (pos < input.size() && (input[pos] == ' ' || input[pos] == '\t')) ++pos;
        if (pos >= input.size()) return {TokenType::END, 0, ""};

        char c = input[pos];
        if (c == '(') { ++pos; return {TokenType::LPAREN, 0, "("}; }
        if (c == ')') { ++pos; return {TokenType::RPAREN, 0, ")"}; }
        if (c == ',') { ++pos; return {TokenType::COMMA, 0, ","}; }
        if (c == '=') { ++pos; return {TokenType::ASSIGN, 0, "="}; }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            ++pos;
            return {TokenType::OP, 0, std::string(1, c)};
        }
        if (std::isdigit(c) || c == '.') {
            size_t start = pos;
            while (pos < input.size() && (std::isdigit(input[pos]) || input[pos] == '.')) ++pos;
            std::string numStr = input.substr(start, pos - start);
            double v = 0;
            try {
                v = std::stod(numStr);
            } catch (...) {
                throw std::runtime_error("Invalid number: " + numStr);
            }
            return {TokenType::NUMBER, v, numStr};
        }
        if (std::isalpha(c) || c == '_') {
            size_t start = pos;
            while (pos < input.size() && (std::isalnum(input[pos]) || input[pos] == '_')) ++pos;
            return {TokenType::IDENT, 0, input.substr(start, pos - start)};
        }
        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }
};

// ---------- 파서 (재귀 하강, 괄호·우선순위·함수 지원) ----------
class Parser {
    Lexer lexer;
    Token current;
    std::map<std::string, double>& vars;

    void advance() { current = lexer.next(); }

    double parseExpr();
    double parseTerm();
    double parseFactor();
    double parseBase();
    double parseFunction(const std::string& name);

public:
    Parser(std::string input, std::map<std::string, double>& variables)
        : lexer(std::move(input)), vars(variables) {
        advance();
    }
    double parse() {
        double r = parseExpr();
        if (current.type != TokenType::END && current.type != TokenType::ASSIGN && current.type != TokenType::RPAREN && current.type != TokenType::COMMA)
            throw std::runtime_error("Unexpected token");
        return r;
    }
    bool isAssignment() const {
        return current.type == TokenType::IDENT;
    }
    std::string getAssignName() const {
        return current.type == TokenType::IDENT ? current.text : "";
    }
};

double Parser::parseExpr() {
    double left = parseTerm();
    while (current.type == TokenType::OP && (current.text == "+" || current.text == "-")) {
        std::string op = current.text;
        advance();
        double right = parseTerm();
        left = (op == "+") ? (left + right) : (left - right);
    }
    return left;
}

double Parser::parseTerm() {
    double left = parseFactor();
    while (current.type == TokenType::OP && (current.text == "*" || current.text == "/")) {
        std::string op = current.text;
        advance();
        double right = parseFactor();
        if (op == "*") left = left * right;
        else {
            if (right == 0) throw std::runtime_error("Division by zero!");
            left = left / right;
        }
    }
    return left;
}

double Parser::parseFactor() {
    if (current.type == TokenType::OP && (current.text == "+" || current.text == "-")) {
        std::string op = current.text;
        advance();
        double v = parseFactor();
        return (op == "-") ? -v : v;
    }
    double base = parseBase();
    if (current.type == TokenType::OP && current.text == "^") {
        advance();
        double exp = parseFactor();
        return std::pow(base, exp);
    }
    return base;
}

double Parser::parseFunction(const std::string& name) {
    if (current.type != TokenType::LPAREN) throw std::runtime_error("Expected '(' after " + name);
    advance(); // consume '('
    double x = parseExpr();
    if (name == "pow") {
        if (current.type != TokenType::COMMA) throw std::runtime_error("pow(x,y) requires two arguments");
        advance();
        double y = parseExpr();
        if (current.type != TokenType::RPAREN) throw std::runtime_error("Expected ')'");
        advance();
        return std::pow(x, y);
    }
    if (current.type != TokenType::RPAREN) throw std::runtime_error("Expected ')'");
    advance(); // consume ')'
    if (name == "sqrt") {
        if (x < 0) throw std::runtime_error("sqrt of negative number");
        return std::sqrt(x);
    }
    if (name == "sin") return std::sin(x);
    if (name == "cos") return std::cos(x);
    if (name == "tan") return std::tan(x);
    if (name == "log" || name == "ln") return std::log(x);
    if (name == "log10") return std::log10(x);
    if (name == "exp") return std::exp(x);
    if (name == "abs") return std::fabs(x);
    throw std::runtime_error("Unknown function: " + name);
}

double Parser::parseBase() {
    if (current.type == TokenType::NUMBER) {
        double v = current.value;
        advance();
        return v;
    }
    if (current.type == TokenType::IDENT) {
        std::string id = current.text;
        advance();
        if (current.type == TokenType::LPAREN) {
            return parseFunction(id);
        }
        auto it = vars.find(id);
        if (it != vars.end()) return it->second;
        throw std::runtime_error("Unknown variable: " + id);
    }
    if (current.type == TokenType::LPAREN) {
        advance();
        double v = parseExpr();
        if (current.type != TokenType::RPAREN) throw std::runtime_error("Expected ')'");
        advance();
        return v;
    }
    throw std::runtime_error("Expected number, variable, or '('");
}

// ---------- 계산기 (변수·히스토리·특수 명령) ----------
class Calculator {
    std::map<std::string, double> variables_;
    std::vector<std::string> history_;
    double lastResult_ = 0;
    bool hasResult_ = false;

    static bool isValidVarName(const std::string& s) {
        if (s.empty()) return false;
        if (!std::isalpha(s[0]) && s[0] != '_') return false;
        for (size_t i = 1; i < s.size(); ++i)
            if (!std::isalnum(s[i]) && s[i] != '_') return false;
        return true;
    }

public:
    double eval(const std::string& input) {
        std::string cleaned;
        for (char c : input) {
            if (c != ' ' && c != '\t') cleaned += c;
        }
        if (cleaned.empty()) throw std::runtime_error("Empty input");

        size_t eq = cleaned.find('=');
        if (eq != std::string::npos && eq > 0 && eq < cleaned.size() - 1) {
            std::string left = cleaned.substr(0, eq);
            std::string right = cleaned.substr(eq + 1);
            if (isValidVarName(left)) {
                Parser rightParser(right, variables_);
                double value = rightParser.parse();
                variables_[left] = value;
                variables_["ans"] = value;
                lastResult_ = value;
                hasResult_ = true;
                return value;
            }
        }

        Parser parser(cleaned, variables_);
        if (hasResult_) variables_["ans"] = lastResult_;
        double result = parser.parse();
        lastResult_ = result;
        hasResult_ = true;
        variables_["ans"] = result;
        return result;
    }

    void addHistory(const std::string& expr, double result) {
        std::ostringstream oss;
        oss << expr << " = " << result;
        history_.push_back(oss.str());
        if (history_.size() > 100) history_.erase(history_.begin());
    }
    const std::vector<std::string>& history() const { return history_; }
    std::map<std::string, double>& variables() { return variables_; }
    double lastResult() const { return lastResult_; }
    bool hasResult() const { return hasResult_; }
};

// ---------- 메인 루프 (history, vars 명령) ----------
int main() {
    Calculator calc;
    std::string input;

    std::cout << "=== C++ CLI Calculator ===" << std::endl;
    std::cout << "Expressions: 5+3, (1+2)*3, sin(0), x=10, ans+1" << std::endl;
    std::cout << "Functions: sin, cos, tan, sqrt, log, ln, log10, exp, abs, pow(x,y)" << std::endl;
    std::cout << "Commands: history, vars, quit" << std::endl;
    std::cout << std::endl;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "quit" || input == "exit" || input == "q") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }

        std::string trimmed;
        for (char c : input) if (c != ' ' && c != '\t') trimmed += c;
        if (trimmed.empty()) continue;

        if (trimmed == "history") {
            const auto& h = calc.history();
            if (h.empty()) { std::cout << "(no history)" << std::endl; continue; }
            for (const auto& s : h) std::cout << "  " << s << std::endl;
            continue;
        }
        if (trimmed == "vars") {
            for (const auto& p : calc.variables())
                std::cout << "  " << p.first << " = " << p.second << std::endl;
            continue;
        }

        try {
            double result = calc.eval(input);
            calc.addHistory(trimmed, result);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}

