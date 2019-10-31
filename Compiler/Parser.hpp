//
//  Parser.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/27.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Parser_hpp
#define Parser_hpp

#include <iostream>
#include "PeekQueue.hpp"
#include "Tokenize/Token.hpp"
#include "Tokenize/TokenType.hpp"
#include "SymbolTable.hpp"
#include <map>

class Parser {
private:
    PeekQueue<Token> data;
    std::map<std::string, Symbol> table;
    void factor();
    void item();
    void expr();
    void returnStatement();
    void printfStatement();
    void scanfStatement();
    void assignStatement();
    void voidCaller();
    void nonvoidCaller();
    void loopStatement();
    void condition();
    void ifStatement();
    void statement();
    void statementS();
    void codeBlock();
    void declareHead();
    void constDefine();
    void varDefine();
    void constDeclare();
    void varDeclare();
    void argList();
    void valueArgList();
    void nonvoidFunc();
    void voidFunc();
    void mainFunc();
    void program();
public:
    Parser(PeekQueue<Token> data); // Input a copy of Token List
};
#endif /* Parser_hpp */
