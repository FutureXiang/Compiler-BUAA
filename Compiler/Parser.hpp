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
#include "Symbol/SymbolTable.hpp"
#include <vector>
#include <set>

class Parser {
private:
    PeekQueue<Token> data;
    SymbolTable table;
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
    void constDefine();
    void varDefine();
    void constDeclare();
    void varDeclare();
    void argList(std::vector<std::shared_ptr<SymbolVar> > &args);
    void valueArgList();
    void nonvoidFunc();
    void voidFunc();
    void mainFunc();
    void program();
    
    void error(Token x);
    Token mustBeThisToken(TokenType type);
    void mustBeInteger();
public:
    std::set<std::pair<int, std::string> > &errorMessages;
    Parser(std::set<std::pair<int, std::string> > &errorMessages, PeekQueue<Token> data); // Input a copy of Token List, the reference of Global Message Contrainer
};
#endif /* Parser_hpp */
