#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <vector>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <unordered_set>

static inline std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

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
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            size_t start = pos;
            while (pos < input.size() && (std::isdigit(static_cast<unsigned char>(input[pos])) || input[pos] == '.')) ++pos;

            // 지수 표기법 지원 (e.g. 1e5, 1.5e-3, 2E+4)
            if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
                size_t nextPos = pos + 1;
                if (nextPos < input.size() && (input[nextPos] == '+' || input[nextPos] == '-')) {
                    nextPos++;
                }
                if (nextPos < input.size() && std::isdigit(static_cast<unsigned char>(input[nextPos]))) {
                    pos = nextPos;
                    while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                        ++pos;
                    }
                }
            }

            std::string numStr = input.substr(start, pos - start);
            double v = 0;
            size_t idx = 0;
            try {
                v = std::stod(numStr, &idx);
            } catch (...) {
                throw std::runtime_error("Invalid number: " + numStr);
            }
            if (idx != numStr.size()) {
                throw std::runtime_error("Invalid number: " + numStr);
            }
            return {TokenType::NUMBER, v, numStr};
        }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = pos;
            while (pos < input.size() && (std::isalnum(static_cast<unsigned char>(input[pos])) || input[pos] == '_')) ++pos;
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
        if (current.type != TokenType::END)
            throw std::runtime_error("Unexpected token: " + (current.text.empty() ? "end of input" : current.text));
        return r;
    }
};

double Parser::parseExpr() {
    double left = parseTerm();
    while (current.type == TokenType::OP && (current.text == "+" || current.text == "-")) {
        std::string op = current.text;
        advance();
        double right = parseTerm();
        left = (op == "+") ? (left + right) : (left - right);
        if (!std::isfinite(left)) throw std::runtime_error("Math error: result is not finite");
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
        if (!std::isfinite(left)) throw std::runtime_error("Math error: result is not finite");
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
        double res = std::pow(base, exp);
        if (!std::isfinite(res)) throw std::runtime_error("Math error: result is not finite");
        return res;
    }
    return base;
}

double Parser::parseFunction(const std::string& name) {
    if (current.type != TokenType::LPAREN) throw std::runtime_error("Expected '(' after " + name);
    advance(); // consume '('
    double x = parseExpr();
    double res = 0;
    if (name == "pow") {
        if (current.type != TokenType::COMMA) throw std::runtime_error("pow(x,y) requires two arguments");
        advance();
        double y = parseExpr();
        if (current.type != TokenType::RPAREN) throw std::runtime_error("Expected ')'");
        advance();
        res = std::pow(x, y);
    } else {
        if (current.type != TokenType::RPAREN) throw std::runtime_error("Expected ')'");
        advance(); // consume ')'
        if (name == "sqrt") {
            if (x < 0) throw std::runtime_error("sqrt of negative number");
            res = std::sqrt(x);
        } else if (name == "sin") res = std::sin(x);
        else if (name == "cos") res = std::cos(x);
        else if (name == "tan") res = std::tan(x);
        else if (name == "log" || name == "ln") res = std::log(x);
        else if (name == "log10") res = std::log10(x);
        else if (name == "exp") res = std::exp(x);
        else if (name == "abs") res = std::fabs(x);
        else throw std::runtime_error("Unknown function: " + name);
    }
    if (!std::isfinite(res)) throw std::runtime_error("Math error: result is not finite");
    return res;
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
    throw std::runtime_error("Unexpected token: " + (current.text.empty() ? "end of input" : current.text));
}

// ---------- 계산기 (변수·히스토리·특수 명령) ----------
class Calculator {
    std::map<std::string, double> variables_;
    std::vector<std::string> history_;

    static bool isValidVarName(const std::string& s) {
        if (s.empty()) return false;
        if (!std::isalpha(static_cast<unsigned char>(s[0])) && s[0] != '_') return false;
        for (size_t i = 1; i < s.size(); ++i)
            if (!std::isalnum(static_cast<unsigned char>(s[i])) && s[i] != '_') return false;
        return true;
    }

    static bool isReserved(const std::string& s) {
        static const std::unordered_set<std::string> reserved = {
            "sin", "cos", "tan", "sqrt", "log", "ln", "log10", "exp", "abs", "pow",
            "history", "vars", "quit", "exit", "q",
            "pi", "e", "ans"
        };
        return reserved.find(s) != reserved.end();
    }

public:
    Calculator() {
        variables_["pi"] = 3.14159265358979323846;
        variables_["e"] = 2.71828182845904523536;
    }

    double eval(const std::string& input) {
        std::string trimmed = trim(input);
        if (trimmed.empty()) throw std::runtime_error("Empty input");

        size_t eq = input.find('=');
        if (eq != std::string::npos) {
            std::string left = trim(input.substr(0, eq));
            std::string right = trim(input.substr(eq + 1));
            if (isValidVarName(left)) {
                if (isReserved(left)) {
                    throw std::runtime_error("Cannot assign to reserved word: " + left);
                }
                Parser rightParser(right, variables_);
                double value = rightParser.parse();
                variables_[left] = value;
                variables_["ans"] = value;
                return value;
            }
        }

        Parser parser(input, variables_);
        double result = parser.parse();
        variables_["ans"] = result;
        return result;
    }

    void addHistory(const std::string& expr, double result) {
        std::ostringstream oss;
        oss << expr << " = " << std::setprecision(12) << result;
        history_.push_back(oss.str());
        if (history_.size() > 100) history_.erase(history_.begin());
    }
    const std::vector<std::string>& history() const { return history_; }
    std::map<std::string, double>& variables() { return variables_; }
};

// ---------- 메인 루프 (history, vars 명령) ----------
int main() {
    Calculator calc;
    std::string input;

    std::cout << "=== C++ CLI Calculator ===" << std::endl;
    std::cout << "Expressions: 5+3, (1+2)*3, sin(0), x=10, ans+1, 1e5" << std::endl;
    std::cout << "Constants: pi, e" << std::endl;
    std::cout << "Functions: sin, cos, tan, sqrt, log, ln, log10, exp, abs, pow(x,y)" << std::endl;
    std::cout << "Commands: history, vars, quit" << std::endl;
    std::cout << std::endl;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;

        std::string trimmed = trim(input);
        if (trimmed.empty()) continue;

        if (trimmed == "quit" || trimmed == "exit" || trimmed == "q") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }

        if (trimmed == "history") {
            const auto& h = calc.history();
            if (h.empty()) { std::cout << "(no history)" << std::endl; continue; }
            for (const auto& s : h) std::cout << "  " << s << std::endl;
            continue;
        }
        if (trimmed == "vars") {
            for (const auto& p : calc.variables())
                std::cout << "  " << p.first << " = " << std::setprecision(12) << p.second << std::endl;
            continue;
        }

        try {
            double result = calc.eval(input);
            calc.addHistory(trimmed, result);
            std::cout << "Result: " << std::setprecision(12) << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}
