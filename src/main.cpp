#include <cmath>
#include <regex>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>

using std::cout;
using std::vector;
using std::regex;
using std::regex_search;
using std::smatch;
using std::string;


enum TokenType {
    NUMBER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    POWER,
    LPAREN,
    RPAREN,
    WHITESPACE,
    NO_MORE_TOKENS,
    LEXICAL_ERROR
};

struct Token {
    TokenType type = LEXICAL_ERROR;
    string value;

    Token() = default;

    Token(const TokenType type, const string &value)
        : type(type), value(value) {
    }

    bool operator==(const Token &other) const {
        return type == other.type && value == other.value;
    }
};

class Lexer {
    string input;
    string remaining;

public:
    explicit Lexer(const string &input)
        : input(input) {
        remaining = input;
    }

    Token getNextToken() {
        Token token;
        do {
            if (remaining.empty()) {
                token = Token(NO_MORE_TOKENS, "");
                break;
            }

            smatch match;
            if (regex_search(remaining, match, regex("^([0-9]+)"))) {
                token = Token(NUMBER, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\+)"))) {
                token = Token(PLUS, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\-)"))) {
                token = Token(MINUS, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\*)"))) {
                token = Token(MULTIPLY, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\/)"))) {
                token = Token(DIVIDE, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\^)"))) {
                token = Token(POWER, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\()"))) {
                token = Token(LPAREN, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\))"))) {
                token = Token(RPAREN, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\s+)"))) {
                token = Token(WHITESPACE, match[1].str());
                removeToken(token);
            } else {
                token = Token(LEXICAL_ERROR, match[1].str());
            }
        } while (token.type == WHITESPACE);

        return token;
    }

    bool getNextToken(Token &token) {
        token = getNextToken();
        return token.type != LEXICAL_ERROR;
    }

    void removeToken(const Token &token) {
        remaining = remaining.substr(token.value.size());
    }
};


// expr = factor {- factor}
// factor -> INTEGER

class ASTNode {
public:
    virtual int process() = 0;

    virtual string toString() = 0;

    virtual ~ASTNode() = default;
};

class ASTNodeAdd final : public ASTNode {
public:
    ASTNode *pLeft;
    ASTNode *pRight;

    ASTNodeAdd(ASTNode *pLeft, ASTNode *pRight)
        : pLeft(pLeft), pRight(pRight) {
    }

    ~ASTNodeAdd() override {
        delete pLeft;
        delete pRight;
    }

    int process() override {
        return pLeft->process() + pRight->process();
    }

    string toString() override {
        return "add(" + pLeft->toString() + ", " + pRight->toString() + ")";
    }
};

class ASTNodeSubtract final : public ASTNode {
public:
    ASTNode *pLeft;
    ASTNode *pRight;

    ASTNodeSubtract(ASTNode *pLeft, ASTNode *pRight)
        : pLeft(pLeft), pRight(pRight) {
    }

    ~ASTNodeSubtract() override {
        delete pLeft;
        delete pRight;
    }

    int process() override {
        return pLeft->process() - pRight->process();
    }

    string toString() override {
        return "subtract(" + pLeft->toString() + ", " + pRight->toString() + ")";
    }
};

class ASTNodeMultiply final : public ASTNode {
public:
    ASTNode *pLeft;
    ASTNode *pRight;

    ASTNodeMultiply(ASTNode *pLeft, ASTNode *pRight)
        : pLeft(pLeft), pRight(pRight) {
    }

    ~ASTNodeMultiply() override {
        delete pLeft;
        delete pRight;
    }

    int process() override {
        return pLeft->process() * pRight->process();
    }

    string toString() override {
        return "multiply(" + pLeft->toString() + ", " + pRight->toString() + ")";
    }
};

class ASTNodeDivide final : public ASTNode {
public:
    ASTNode *pLeft;
    ASTNode *pRight;

    ASTNodeDivide(ASTNode *pLeft, ASTNode *pRight)
        : pLeft(pLeft), pRight(pRight) {
    }

    ~ASTNodeDivide() override {
        delete pLeft;
        delete pRight;
    }

    int process() override {
        return pLeft->process() / pRight->process();
    }

    string toString() override {
        return "divide(" + pLeft->toString() + ", " + pRight->toString() + ")";
    }
};

class ASTNodePower final : public ASTNode {
public:
    ASTNode *pLeft;
    ASTNode *pRight;

    ASTNodePower(ASTNode *pLeft, ASTNode *pRight)
        : pLeft(pLeft), pRight(pRight) {
    }

    ~ASTNodePower() override {
        delete pLeft;
        delete pRight;
    }

    int process() override {
        return static_cast<int>(std::pow(pLeft->process(), pRight->process()));
    }

    string toString() override {
        return "power(" + pLeft->toString() + ", " + pRight->toString() + ")";
    }
};

class ASTNodeNumber final : public ASTNode {
    int number;

public:
    explicit ASTNodeNumber(const int number)
        : number(number) {
    }

    int process() override {
        return number;
    }

    string toString() override {
        return "number(" + std::to_string(number) + ")";
    }
};


// expr = factor - expr
// factor -> INTEGER

class ASTParser {
    ASTNode *expr(Lexer &lexer) {
        ASTNode *pLeft = term(lexer);
        Token token;
        while (lexer.getNextToken(token) && (token.type == PLUS || token.type == MINUS)) {
            lexer.removeToken(token);
            ASTNode *pRight = term(lexer);
            if (token.type == PLUS) {
                pLeft = new ASTNodeAdd(pLeft, pRight);
            } else {
                pLeft = new ASTNodeSubtract(pLeft, pRight);
            }
        }

        return pLeft;
    }

    ASTNode *term(Lexer &lexer) {
        ASTNode *pLeft = power(lexer);
        Token token;
        while (lexer.getNextToken(token) && (token.type == MULTIPLY || token.type == DIVIDE)) {
            lexer.removeToken(token);
            ASTNode *pRight = power(lexer);
            if (token.type == MULTIPLY) {
                pLeft = new ASTNodeMultiply(pLeft, pRight);
            } else {
                pLeft = new ASTNodeDivide(pLeft, pRight);
            }
        }

        return pLeft;
    }

    ASTNode *power(Lexer &lexer) {
        ASTNode *pLeft = factor(lexer);
        Token token;
        if (lexer.getNextToken(token) && token.type == POWER) {
            lexer.removeToken(token);
            // NOTE: power operators are right-associative instead of left associative like everything else
            ASTNode *pRight = power(lexer);
            pLeft = new ASTNodePower(pLeft, pRight);
        }
        return pLeft;
    }

    ASTNode *factor(Lexer &lexer) {
        Token token;
        lexer.getNextToken(token);
        ASTNode *pNode;
        if (token.type == NUMBER) {
            lexer.removeToken(token);
            pNode = new ASTNodeNumber(stoi(token.value));
        } else if (token.type == LPAREN) {
            lexer.removeToken(token);
            pNode = expr(lexer);
            lexer.getNextToken(token);
            if (token.type == RPAREN) {
                lexer.removeToken(token);
            } else {
                throw std::runtime_error("missing right parenthesis");
            }
        } else {
            throw std::runtime_error("parse error");
        }
        return pNode;
    }

public:
    ASTParser() = default;

    ASTNode *parse(const string &statement) {
        Lexer lexer(statement);
        return expr(lexer);
    }
};

class EvalParser {
    int expr(Lexer &lexer) {
        int left = term(lexer);
        Token token;
        while (lexer.getNextToken(token) && (token.type == PLUS || token.type == MINUS)) {
            lexer.removeToken(token);
            const int right = term(lexer);
            if (token.type == PLUS) {
                left = left + right;
            } else {
                left = left - right;
            }
        }

        return left;
    }

    int term(Lexer &lexer) {
        int left = power(lexer);
        Token token;
        while (lexer.getNextToken(token) && (token.type == MULTIPLY || token.type == DIVIDE)) {
            lexer.removeToken(token);
            const int right = power(lexer);
            if (token.type == MULTIPLY) {
                left = left * right;
            } else {
                left = left / right;
            }
        }

        return left;
    }

    int power(Lexer &lexer) {
        int left = factor(lexer);
        Token token;
        if (lexer.getNextToken(token) && token.type == POWER) {
            lexer.removeToken(token);
            // NOTE: power operators are right-associative instead of left associative like everything else
            const int right = power(lexer);
            left = static_cast<int>(std::pow(left, right));
        }
        return left;
    }

    int factor(Lexer &lexer) {
        Token token;
        lexer.getNextToken(token);
        int num;
        if (token.type == NUMBER) {
            lexer.removeToken(token);
            num = stoi(token.value);
        } else if (token.type == LPAREN) {
            lexer.removeToken(token);
            num = expr(lexer);
            lexer.getNextToken(token);
            if (token.type == RPAREN) {
                lexer.removeToken(token);
            } else {
                throw std::runtime_error("missing right parenthesis");
            }
        } else {
            throw std::runtime_error("parse error");
        }
        return num;
    }

public:
    EvalParser() = default;

    int parse(const string &statement) {
        Lexer lexer(statement);
        return expr(lexer);
    }
};


int main() {
    ASTParser parser;
    EvalParser eval;
    const vector<std::pair<string, int> > statements = {
        {"1-2-3", -4},
        {"1", 1},
        {"2-1", 1},
        {"5 - 4 - 3", -2},
        {"2 - 1", 1},
        {"2 * 3 / 2", 3},
        {" 2 *2 / 3", 1},
        {"2 - 2 * 3", -4},
        {"(2 - 2) * 3", 0},
        {"2 ^ 2 ^ 3", 256},
        {"(1 - 2 ^ 2 + 1) * 3", -6}
    };

    int failedTest = 0;
    for (const auto &test: statements) {
        ASTNode *ast = parser.parse(test.first);
        const int val = ast->process();
        const int evalVal = eval.parse(test.first);
        cout << "stmt: " << test.first << std::endl;
        cout << "  ast: " << ast->toString() << std::endl;
        cout << "  expected result: " << test.second << std::endl;
        cout << "  result: " << val << "\n";
        if (val == test.second) {
            cout << "    TEST PASSED" << std::endl;
        } else {
            cout << "    TEST FAILED" << std::endl;
            failedTest++;
        }
        cout << "  eval: " << evalVal << std::endl;
        if (evalVal == test.second) {
            cout << "    TEST PASSED" << std::endl;
        } else {
            cout << "    TEST FAILED" << std::endl;
        }

        delete ast;
    }
    return failedTest;
}
