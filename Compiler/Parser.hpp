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
#include "Token.hpp"
#include "TokenType.hpp"

void Parser(PeekQueue<Token> data);

void factor(PeekQueue<Token> &data);
void item(PeekQueue<Token> &data);
void expr(PeekQueue<Token> &data);
void returnStatement(PeekQueue<Token> &data);
void printfStatement(PeekQueue<Token> &data);
void scanfStatement(PeekQueue<Token> &data);
void assignStatement(PeekQueue<Token> &data);
void voidCaller(PeekQueue<Token> &data);
void nonvoidCaller(PeekQueue<Token> &data);
void loopStatement(PeekQueue<Token> &data);
void condition(PeekQueue<Token> &data);
void ifStatement(PeekQueue<Token> &data);
void statement(PeekQueue<Token> &data);
void statementS(PeekQueue<Token> &data);
void codeBlock(PeekQueue<Token> &data);
void declareHead(PeekQueue<Token> &data);
void constDefine(PeekQueue<Token> &data);
void varDefine(PeekQueue<Token> &data);
void constDeclare(PeekQueue<Token> &data);
void varDeclare(PeekQueue<Token> &data);
void argList(PeekQueue<Token> &data);
void valueArgList(PeekQueue<Token> &data);
void nonvoidFunc(PeekQueue<Token> &data);
void voidFunc(PeekQueue<Token> &data);
void mainFunc(PeekQueue<Token> &data);
void program(PeekQueue<Token> &data);

#endif /* Parser_hpp */
