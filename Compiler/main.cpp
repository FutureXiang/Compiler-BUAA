//
//  main.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include <iostream>
#include <cstdio>
#include <vector>
#include "TextData.hpp"
#include "Token.hpp"

using namespace std;

TextData readIn() {
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    char character;
    
    TextData data = TextData();
    
    while (cin >> character)
        data.add(character);
    return data;
}

void error(string x) {
    cout << "Error @ " << x << endl;
}

int main() {
    TextData data = readIn();
    vector<Token> tokens;

    while (!data.empty()) {
        char now = data.pop();
        while (now == ' ' || now == '\n' || now == '\t')
            now = data.pop();
        switch (now) {
            case '(':
                tokens.push_back(Token(string(1, now), lBracket));
                break;
            case ')':
                tokens.push_back(Token(string(1, now), rBracket));
                break;
            case '[':
                tokens.push_back(Token(string(1, now), lSquare));
                break;
            case ']':
                tokens.push_back(Token(string(1, now), rSquare));
                break;
            case '{':
                tokens.push_back(Token(string(1, now), lCurly));
                break;
            case '}':
                tokens.push_back(Token(string(1, now), rCurly));
                break;
            case '+':
                tokens.push_back(Token(string(1, now), pluss));
                break;
            case '-':
                tokens.push_back(Token(string(1, now), minuss));
                break;
            case '*':
                tokens.push_back(Token(string(1, now), multi));
                break;
            case '/':
                tokens.push_back(Token(string(1, now), divd));
                break;
            case ';':
                tokens.push_back(Token(string(1, now), semi));
                break;
            case ',':
                tokens.push_back(Token(string(1, now), comma));
                break;
            default:
                if (now == '!') {
                    // "!="
                    string pre = string(1, now);
                    char next = data.pop();
                    pre += next;
                    if (next == '=')
                        tokens.push_back(Token(pre, neq));
                    else
                        error(pre);
                } else if (now == '=') {
                    // "=" OR "=="
                    if (data.peek() == '=') {
                        data.pop();
                        tokens.push_back(Token("==", equalto));
                    }
                    else
                        tokens.push_back(Token("=", assign));
                } else if (now == '<') {
                    // "<" OR "<="
                    if (data.peek() == '=') {
                        data.pop();
                        tokens.push_back(Token("<=", leq));
                    }
                    else
                        tokens.push_back(Token("<", lesss));
                } else if (now == '>') {
                    // ">" OR ">="
                    if (data.peek() == '=') {
                        data.pop();
                        tokens.push_back(Token(">=", geq));
                    }
                    else
                        tokens.push_back(Token(">", great));
                } else if (now == '\'') {
                    string content = "";
                    while (!data.empty() && data.peek() != '\'' && data.peek() != '\n')
                        content += data.pop();
                    if (data.empty())
                        error("'"+content+"\\EOF");
                    else if (data.peek() == '\n') {
                        error("'"+content+"\\NEWLINE");
                        data.pop();
                    } else if (content.length() != 1) {
                        error("'"+content+"'");
                        data.pop();
                    }
                    else {
                        tokens.push_back(Token(content, charConst));
                        data.pop();
                    }
                } else if (now == '"') {
                    string content = "";
                    while (!data.empty() && data.peek() != '"' && data.peek() != '\n')
                        content += data.pop();
                    if (data.empty())
                        error("\""+content+"\\EOF");
                    else if (data.peek() == '\n') {
                        error("\""+content+"\\NEWLINE");
                        data.pop();
                    }
                    else {
                        tokens.push_back(Token(content, stringConst));
                        data.pop();
                    }
                } else if (isdigit(now)) {
                    string content = string(1, now);
                    while (!data.empty() && isdigit(data.peek()))
                        content += data.pop();
                    tokens.push_back(Token(content, intConst));
                }
                // TODO
                break;
        }
    }
    
    for (Token t: tokens)
        cout << t.toString() << endl;
    return 0;
}
