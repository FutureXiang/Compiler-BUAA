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
#include "Quadruple/Quadruple.hpp"

extern std::map<TokenType, TokenType> compSwap;
extern std::map<TokenType, Operator> token2op;

class Parser {
private:
    int line;
    bool has_return;
    ExprType expected_return;
    
    PeekQueue<Token> data;
    SymbolTable table;
    QuadrupleList qcodes;
    
    ExprType factor(Operand *&operand);
    ExprType item(Operand *&operand);
    ExprType expr(Operand *&operand);
    void returnStatement();
    void printfStatement();
    void scanfStatement();
    void assignStatement();
    void voidCaller(Token identifier, bool check_argmatch);
    void nonvoidCaller(Token identifier);
    void loopStatement();
    Operator condition(Operand *&first, Operand *&second);
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
    
    void error(Error e);
    void checkCallerMatch(bool is_void, Token id, std::vector<ExprType> argtypes);
    Token printPop();
    Token mustBeThisToken(TokenType type);
    int mustBeInteger();
public:
    std::set<std::pair<int, std::string> > &errorMessages;
    Parser(std::set<std::pair<int, std::string> > &errorMessages, PeekQueue<Token> data); // Input a copy of Token List, the reference of Global Message Contrainer
    
    std::vector<Quadruple> *getQcodes() {
        return qcodes.getQCodes();
    }
};
#endif /* Parser_hpp */
