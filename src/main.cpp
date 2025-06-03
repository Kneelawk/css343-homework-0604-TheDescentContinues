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
    MINUS,
    WHITESPACE,
    NO_MORE_TOKENS,
    LEXICAL_ERROR
};

struct Token {
    TokenType type;
    string value;

    Token() {
    }

    Token(const TokenType type, const string &value)
        : type(type), value(value) { ; }

    bool operator==(const Token &other) const {
        return type == other.type && value == other.value;
    }
};

class Lexer {
    string input;
    string remaining;

public:
    Lexer(const string &input)
        : input(input) {
        remaining = input;
    }

    Token getNextToken() {
        Token token;
        do {
            if (remaining.size() == 0) {
                token = Token(NO_MORE_TOKENS, "");
                break;
            }

            smatch match;
            if (regex_search(remaining, match, regex("^([0-9]+)"))) {
                token = Token(NUMBER, match[1].str());
            } else if (regex_search(remaining, match, regex("^(\\-)"))) {
                token = Token(MINUS, match[1].str());
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

    virtual ~ASTNode() {
    }
};

class ASTNodeSubtract final : public ASTNode {
public:
    ASTNode *pLeft;
    ASTNode *pRight;

    ASTNodeSubtract(ASTNode *pLeft, ASTNode *pRight)
        : pLeft(pLeft), pRight(pRight) { ; }

    ~ASTNodeSubtract() {
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

class ASTNodeNumber final : public ASTNode {
    int number;

public:
    ASTNodeNumber(const int number)
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
        ASTNode *pLeft = factor(lexer);
        Token token;
        while (lexer.getNextToken(token) && token.type == MINUS) {
            lexer.removeToken(token);
            ASTNode *pRight = factor(lexer);
            pLeft = new ASTNodeSubtract(pLeft, pRight);
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
        } else {
            throw new std::runtime_error("parse error");
        }
        return pNode;
    }

public:
    ASTParser() {
    }

    ASTNode *parse(const string &statement) {
        Lexer lexer(statement);
        return expr(lexer);
    }
};

class EvalParser {
    int expr(Lexer &lexer) {
        int pLeft = factor(lexer);
        Token token;
        while (lexer.getNextToken(token) && token.type == MINUS) {
            lexer.removeToken(token);
            const int pRight = factor(lexer);
            pLeft = pLeft - pRight;
        }

        return pLeft;
    }

    int factor(Lexer &lexer) {
        Token token;
        lexer.getNextToken(token);
        int pNode;
        if (token.type == NUMBER) {
            lexer.removeToken(token);
            pNode = stoi(token.value);
        } else {
            throw new std::runtime_error("parse error");
        }
        return pNode;
    }

public:
    EvalParser() {
    }

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
        {"2 - 1", 1}
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
