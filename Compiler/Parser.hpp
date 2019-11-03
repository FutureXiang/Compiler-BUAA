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
#include "Symbol/Symbol.hpp"
#include <vector>
#include <set>
#include "Error.hpp"

class Parser {
private:
    bool has_return;
    ExprType expected_return;
    
    PeekQueue<Token> data;
    SymbolTable table;
    ExprType factor();
    ExprType item();
    ExprType expr();
    void returnStatement();
    void printfStatement();
    void scanfStatement();
    void assignStatement();
    void voidCaller(Token identifier);
    void nonvoidCaller(Token identifier);
    void loopStatement();
    void condition();
    void ifStatement();
    void statement();
    void statementS();
    void codeBlock();
    void constDefine(bool is_global);
    void varDefine(bool is_global);
    void constDeclare(bool is_global);
    void varDeclare(bool is_global);
    void argList(std::vector<std::shared_ptr<SymbolVar> > &args);
    std::vector<ExprType> valueArgList();
    void nonvoidFunc();
    void voidFunc();
    void mainFunc();
    void program();
    
    void error(Token x, Error e);
    void checkCallerMatch(bool is_void, Token id, std::vector<ExprType> argtypes);
    Token mustBeThisToken(TokenType type);
    void mustBeInteger();
public:
    std::set<std::pair<int, std::string> > &errorMessages;
    Parser(std::set<std::pair<int, std::string> > &errorMessages, PeekQueue<Token> data); // Input a copy of Token List, the reference of Global Message Contrainer
};
#endif /* Parser_hpp */
