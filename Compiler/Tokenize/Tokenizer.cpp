//
//  Tokenizer.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/27.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Tokenizer.hpp"
#include "../PeekQueue.cpp"
using namespace std;

void Tokenizer::error(int line, Error e) {
    errorMessages.insert(make_pair(line, ErrorToString(e)));
//    tokens.add(Token(x, __invalidToken__));
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

bool charSetForChar(char x) {
    return x=='+' || x=='-' || x=='*' || x=='/' || is_indentifier_alpha(x) || isdigit(x);
}

bool _charSetForString(char x) {
    return x==32 || x==33 || (x>=35 && x<=126);
}

bool charSetForString(string x) {
    for(auto ch: x) {
        if (!_charSetForString(ch))
            return false;
    }
    return true;
}

bool unsignedInteger(string num) {
    if (num=="0") return true;
    else if (num[0]=='0') return false;
    else return true;
}

Tokenizer::Tokenizer(std::set<std::pair<int, std::string> > &mess, PeekQueue<char> data) : errorMessages(mess) {
    
    int lineCounter = 1;
    while (!data.empty()) {
        char now = data.pop();
        while (!data.empty() && (isspace(now) || now == '\0')) {
            if (now == '\n')
                lineCounter++;
            now = data.pop();
        }
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
                        error(lineCounter, token_invalid);
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
                        error(lineCounter, token_invalid);
                    else if (data.peek() == '\n') {
                        error(lineCounter, token_invalid);
                        data.pop();
                        lineCounter++;
                    } else if (content.length() != 1) {
                        error(lineCounter, token_invalid);
                        data.pop();
                    } else if (!charSetForChar(content[0])) {
                        error(lineCounter, token_invalid);
                        data.pop();
                    }
                    else {
                        newToken = Token(content, charConst);
                        flag = true;
                        data.pop();
                    }
                } else if (now == '"') {
                    string content = "";
                    while (!data.empty() && data.peek() != '"' && data.peek() != '\n') {
                        char now = data.pop();
                        content += now;
                        if (now == '\\')
                            content += '\\';
                    }
                    if (data.empty())
                        error(lineCounter, token_invalid);
                    else if (data.peek() == '\n') {
                        error(lineCounter, token_invalid);
                        data.pop();
                        lineCounter++;
                    } else if (!charSetForString(content)) {
                        error(lineCounter, token_invalid);
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
                    if (unsignedInteger(content)) {
                        newToken = Token(content, intConst);
                        flag = true;
                    } else {
                        error(lineCounter, token_invalid);
                    }
                } else if (is_indentifier_alpha(now)) {
                    string content = string(1, now);
                    while (is_indentifier_follow(data.peek()))
                        content += data.pop();
                    newToken = classifyAndConstruct(content);
                    flag = true;
                } else
                    error(lineCounter, token_invalid);
                break;
        }
        if (flag) {
            newToken.setLineNo(lineCounter);
            tokens.add(newToken);
        }
    }
}
