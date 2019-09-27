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
    
    while (scanf("%c", &character) != EOF)
        data.add(character);
    return data;
}

void error(string x) {
    cout << "Error @ " << x << endl;
}

bool is_indentifier_alpha(char x) {
    return isupper(x) || islower(x) || x == '_';
}

bool is_indentifier_follow(char x) {
    return is_indentifier_alpha(x) || isdigit(x);
}

Token classifyAndConstruct(string content) {
    TokenType thisType = name;
    if (extractKeys.count(content))
        thisType = extractKeys[content];
    return Token(content, thisType);
}

int main() {
    TextData data = readIn();
    vector<Token> tokens;

    while (!data.empty()) {
        char now = data.pop();
        while (!data.empty() && (isspace(now) || now == '\0'))
            now = data.pop();
        if (data.empty() && (isspace(now) || now == '\0'))
            break;
        Token newToken;
        bool flag = false;
        switch (now) {
            case '(':
                newToken = Token(string(1, now), lBracket);
                flag = true;
                break;
            case ')':
                newToken = Token(string(1, now), rBracket);
                flag = true;
                break;
            case '[':
                newToken = Token(string(1, now), lSquare);
                flag = true;
                break;
            case ']':
                newToken = Token(string(1, now), rSquare);
                flag = true;
                break;
            case '{':
                newToken = Token(string(1, now), lCurly);
                flag = true;
                break;
            case '}':
                newToken = Token(string(1, now), rCurly);
                flag = true;
                break;
            case '+':
                newToken = Token(string(1, now), pluss);
                flag = true;
                break;
            case '-':
                newToken = Token(string(1, now), minuss);
                flag = true;
                break;
            case '*':
                newToken = Token(string(1, now), multi);
                flag = true;
                break;
            case '/':
                newToken = Token(string(1, now), divd);
                flag = true;
                break;
            case ';':
                newToken = Token(string(1, now), semi);
                flag = true;
                break;
            case ',':
                newToken = Token(string(1, now), comma);
                flag = true;
                break;
            default:
                if (now == '!') {
                    // "!="
                    string pre = string(1, now);
                    char next = data.pop();
                    pre += next;
                    if (next == '=') {
                        newToken = Token(pre, neq);
                        flag = true;
                    }
                    else
                        error(pre);
                } else if (now == '=') {
                    // "=" OR "=="
                    if (data.peek() == '=') {
                        data.pop();
                        newToken = Token("==", equalto);
                        flag = true;
                    }
                    else {
                        newToken = Token("=", assign);
                        flag = true;
                    }
                } else if (now == '<') {
                    // "<" OR "<="
                    if (data.peek() == '=') {
                        data.pop();
                        newToken = Token("<=", leq);
                        flag = true;
                    }
                    else {
                        newToken = Token("<", lesss);
                        flag = true;
                    }
                } else if (now == '>') {
                    // ">" OR ">="
                    if (data.peek() == '=') {
                        data.pop();
                        newToken = Token(">=", geq);
                        flag = true;
                    }
                    else {
                        newToken = Token(">", great);
                        flag = true;
                    }
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
                        newToken = Token(content, charConst);
                        flag = true;
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
                        newToken = Token(content, stringConst);
                        flag = true;
                        data.pop();
                    }
                } else if (isdigit(now)) {
                    string content = string(1, now);
                    while (!data.empty() && isdigit(data.peek()))
                        content += data.pop();
                    newToken = Token(content, intConst);
                    flag = true;
                } else if (is_indentifier_alpha(now)) {
                    string content = string(1, now);
                    while (is_indentifier_follow(data.peek()))
                        content += data.pop();
                    newToken = classifyAndConstruct(content);
                    flag = true;
                } else
                    error(string(1, now));
                break;
        }
        if (flag) {
            tokens.push_back(newToken);
            cout << newToken.toString() << endl;
        }
    }
    return 0;
}
