//
//  Parser.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/27.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Parser.hpp"
#include "PeekQueue.cpp"

void error(Token x) {
    std::cout << "Error @ " << x.toString() << std::endl;
}

Token printPop(PeekQueue<Token> &data) {
    Token token = data.pop();
    std::cout << token.toString() << std::endl;
    return token;
}

void mark(std::string str) {
    std::cout << str << std::endl;
}

Token mustBeThisToken(PeekQueue<Token> &data, TokenType type) {
    if (data.peek().getType() == type)
        return printPop(data);              // print, pop, return
    else {
        error(data.peek());                 // error, panic
        while(1);
    }
}

void mustBeInteger(PeekQueue<Token> &data) {
    if (data.peek().getType() == pluss || data.peek().getType() == minuss) {        // (+|-)＜无符号整数＞ => ＜整数＞
        Token plus_minus = printPop(data);
        Token intConstTk = mustBeThisToken(data, intConst);
        mark("<无符号整数>");
    } else {
        Token intConstTk = mustBeThisToken(data, intConst);                         // ＜无符号整数＞ => ＜整数＞
        mark("<无符号整数>");
    }
    mark("<整数>");
}

void Parser::factor() {
    if (data.peek().getType() == name) {
        Token identifier = printPop(data);                                          // ＜标识符＞
        if (data.peek().getType() == lSquare) {                                     // ＜标识符＞‘[’＜表达式＞‘]’
            printPop(data);
            expr();
            mustBeThisToken(data, rSquare);
        } else if (data.peek().getType() == lBracket) {                             // ＜标识符＞ '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段
            nonvoidCaller();
        }
    } else if (data.peek().getType() == lBracket) {                                 // ‘(’＜表达式＞‘)’
        printPop(data);
        expr();
        mustBeThisToken(data, rBracket);
    } else if (data.peek().getType() == charConst) {                                // ＜字符＞
        Token charConstTk = printPop(data);
    } else
        mustBeInteger(data);                                                        // <整数>
    mark("<因子>");
}

void Parser::item() {
    factor();                                                                       // ＜因子＞
    while (data.peek().getType() == multi || data.peek().getType() == divd) {       // {＜乘法运算符＞＜因子＞}
        Token mul_div = printPop(data);
        factor();
    }
    mark("<项>");
}

void Parser::expr() {
    bool firstItemNegative = false;
    if (data.peek().getType() == pluss) {
        printPop(data);
    } else if(data.peek().getType() == minuss) {
        firstItemNegative = true;
        printPop(data);
    }
    item();
    while (data.peek().getType() == pluss || data.peek().getType() == minuss) {     // {＜加法运算符＞＜项＞}
        Token plus_minus = printPop(data);
        item();
    }
    mark("<表达式>");
}

void Parser::returnStatement() {
    mustBeThisToken(data, returnKey);                                               // return['('＜表达式＞')']
    if (data.peek().getType() == lBracket) {
        printPop(data);
        expr();
        mustBeThisToken(data, rBracket);
    }
    mark("<返回语句>");
}

void Parser::printfStatement() {
    mustBeThisToken(data, print);
    mustBeThisToken(data, lBracket);                                                // printf '('
    if (data.peek().getType() == stringConst) {
        Token stringConstTk = printPop(data);                                       // ＜字符串＞
        mark("<字符串>");
        if (data.peek().getType() == comma) {                                       // ＜字符串＞,＜表达式＞
            printPop(data);
            expr();
        }
    } else
        expr();                                                                 // ＜表达式＞
    mustBeThisToken(data, rBracket);                                                // ')'
    mark("<写语句>");
}

void Parser::scanfStatement() {
    mustBeThisToken(data, scan);
    mustBeThisToken(data, lBracket);                                                // scanf '('
    Token identifier = mustBeThisToken(data, name);                                 // ＜标识符＞{,＜标识符＞}
    while (data.peek().getType() == comma) {
        printPop(data);
        Token identifier = mustBeThisToken(data, name);
    }
    mustBeThisToken(data, rBracket);                                                // ')'
    mark("<读语句>");
}

void Parser::assignStatement() {
    Token identifier = mustBeThisToken(data, name);                                 // ＜标识符＞
    if (data.peek().getType() == lSquare) {                                         // optional: '['＜表达式＞']'
        printPop(data);
        expr();
        mustBeThisToken(data, rSquare);
    }
    mustBeThisToken(data, assign);                                                  // ＝＜表达式＞
    expr();
    mark("<赋值语句>");
}

void Parser::voidCaller() {
    mustBeThisToken(data, lBracket);                                                // '('＜值参数表＞')' => ＜无返回值函数调用语句＞的不含<标识符>的后半段
    if (data.peek().getType() != rBracket)                                          // ＜值参数表＞此处禁止为空时进入，但是为空时也算＜值参数表＞，要输出
        valueArgList();
    mark("<值参数表>");
    mustBeThisToken(data, rBracket);
    mark("<无返回值函数调用语句>");
}

void Parser::nonvoidCaller() {
    mustBeThisToken(data, lBracket);                                                // '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段
    if (data.peek().getType() != rBracket)                                          // ＜值参数表＞此处禁止为空时进入，但是为空时也算＜值参数表＞，要输出
        valueArgList();
    mark("<值参数表>");
    mustBeThisToken(data, rBracket);
    mark("<有返回值函数调用语句>");
}

void Parser::loopStatement() {
    if (data.peek().getType() == whileKey) {                                        //  while '('＜条件＞')'＜语句＞
        printPop(data);
        mustBeThisToken(data, lBracket);
        condition();
        mustBeThisToken(data, rBracket);
        statement();
    } else if (data.peek().getType() == doKey) {                                    // do＜语句＞while '('＜条件＞')'
        printPop(data);
        statement();
        mustBeThisToken(data, whileKey);
        mustBeThisToken(data, lBracket);
        condition();
        mustBeThisToken(data, rBracket);
    } else {
        mustBeThisToken(data, forKey);                                              // for'(' ＜标识符＞＝＜表达式＞;＜条件＞; ＜标识符＞＝＜标识符＞(+|-)＜步长＞ ')'＜语句＞
        mustBeThisToken(data, lBracket);
        Token identifier = mustBeThisToken(data, name);
        mustBeThisToken(data, assign);
        expr();
        mustBeThisToken(data, semi);
        condition();
        mustBeThisToken(data, semi);
        
        Token identifier2 = mustBeThisToken(data, name);
        mustBeThisToken(data, assign);
        Token identifier3 = mustBeThisToken(data, name);
        if (data.peek().getType() == pluss || data.peek().getType() == minuss) {
            Token plus_minus = printPop(data);
            Token step = mustBeThisToken(data, intConst);
            mark("<无符号整数>");
            mark("<步长>");
        } else
            error(data.peek());
        mustBeThisToken(data, rBracket);
        statement();
    }
    mark("<循环语句>");
}

void Parser::condition() {
    expr();                                                                         // ＜表达式＞
    TokenType nextType = data.peek().getType();
    if (nextType == lesss || nextType == leq || nextType == great || nextType == geq || nextType == neq || nextType == equalto) {
        Token relationship = printPop(data);                                        // ＜表达式＞＜关系运算符＞＜表达式＞
        expr();
    }
    mark("<条件>");
}

void Parser::ifStatement() {
    mustBeThisToken(data, ifKey);                                                   // if '('＜条件＞')'＜语句＞
    mustBeThisToken(data, lBracket);
    condition();
    mustBeThisToken(data, rBracket);
    statement();
    if (data.peek().getType() == elseKey) {                                         // [else＜语句＞]
        printPop(data);
        statement();
    }
    mark("<条件语句>");
}

void Parser::statement() {
    TokenType nextType = data.peek().getType();
    switch (nextType) {
        case ifKey:                                                                 // ＜条件语句＞
            ifStatement();
            break;
        case whileKey:                                                              // ＜循环语句＞
        case doKey:
        case forKey:
            loopStatement();
            break;
        case lCurly:                                                                // '{'＜语句列＞'}'
            printPop(data);
            statementS();
            mustBeThisToken(data, rCurly);
            break;
        case scan:                                                                  // ＜读语句＞;
            scanfStatement();
            mustBeThisToken(data, semi);
            break;
        case print:                                                                 // ＜写语句＞;
            printfStatement();
            mustBeThisToken(data, semi);
            break;
        case semi:                                                                  // ＜空＞;
            printPop(data);
            break;
        case returnKey:                                                             // ＜返回语句＞;
            returnStatement();
            mustBeThisToken(data, semi);
            break;
        default:                                                                    // ＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞; | ＜赋值语句＞;
            if (data.peek(2).getType() == assign) {
                assignStatement();
                mustBeThisToken(data, semi);
            } else {
                Token identifier = mustBeThisToken(data, name);
                if (table[identifier.getText()].isVoidFunc())
                    voidCaller();
                else
                    nonvoidCaller();
                mustBeThisToken(data, semi);
            }
    }
    mark("<语句>");
}

void Parser::statementS() {
    while (data.peek().getType() != rCurly) {                                       // {＜语句＞}
        statement();
    }
    mark("<语句列>");
}

void Parser::codeBlock() {
    if (data.peek().getType() == constKey)
        constDeclare();
    if (data.peek().getType() == intKey || data.peek().getType() == charKey) {
        varDeclare();
    }
    statementS();
    mark("<复合语句>");
}

void Parser::declareHead() {                                                        // 仅仅被＜有返回值函数定义＞调用！
    Token type = printPop(data);
    if (type.getType() != intKey && type.getType() != charKey)                      // int|char
        error(type);
    Token identifier = mustBeThisToken(data, name);
    if (type.getType() == intKey)
        table[identifier.getText()] = Symbol(identifier.getText(), intFunct);
    else
        table[identifier.getText()] = Symbol(identifier.getText(), charFunct);
    mark("<声明头部>");
}

void Parser::constDefine() {
    if (data.peek().getType() == intKey) {                                          // int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
        Token type = printPop(data);
        Token identifier = mustBeThisToken(data, name);
        mustBeThisToken(data, assign);
        mustBeInteger(data);
        while (data.peek().getType() == comma) {
            printPop(data);
            Token identifier = mustBeThisToken(data, name);
            mustBeThisToken(data, assign);
            mustBeInteger(data);
        }
    } else if (data.peek().getType() == charKey) {                                  // char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
        Token type = printPop(data);
        Token identifier = mustBeThisToken(data, name);
        mustBeThisToken(data, assign);
        Token charConstTk = mustBeThisToken(data, charConst);
        while (data.peek().getType() == comma) {
            printPop(data);
            Token identifier = mustBeThisToken(data, name);
            mustBeThisToken(data, assign);
            Token charConstTk = mustBeThisToken(data, charConst);
        }
    } else
        error(data.peek());
    mark("<常量定义>");
}

void Parser::varDefine() {
    Token type = printPop(data);
    if (type.getType() != intKey && type.getType() != charKey)                      // int|char
        error(type);
    
    Token identifier = mustBeThisToken(data, name);                                 // ＜标识符＞|＜标识符＞'['＜无符号整数＞']'
    if (data.peek().getType() == lSquare) {
        printPop(data);
        Token arrayLength = mustBeThisToken(data, intConst);
        mark("<无符号整数>");
        mustBeThisToken(data, rSquare);
    }
    while (data.peek().getType() == comma) {                                        // {,(＜标识符＞|＜标识符＞'['＜无符号整数＞']')}
        printPop(data);
        Token identifier = mustBeThisToken(data, name);
        if (data.peek().getType() == lSquare) {
            printPop(data);
            Token arrayLength = mustBeThisToken(data, intConst);
            mark("<无符号整数>");
            mustBeThisToken(data, rSquare);
        }
    }
    mark("<变量定义>");
}

void Parser::constDeclare() {                                                       // const＜常量定义＞;{const＜常量定义＞;}
    mustBeThisToken(data, constKey);
    constDefine();
    mustBeThisToken(data, semi);
    while (data.peek().getType() == constKey) {
        printPop(data);
        constDefine();
        mustBeThisToken(data, semi);
    }
    mark("<常量说明>");
}

// 程序中的变量说明：截止时，后面必为(nonvoid|void|main)函数定义，因此不能用int|char判断，应该用左括号判断
// 复合语句中的变量说明：截止时，后面是语句列，因此可以用int|char判断
void Parser::varDeclare() {                                                         // ＜变量定义＞;{＜变量定义＞;}
    varDefine();
    mustBeThisToken(data, semi);
    while ((data.peek().getType() == intKey || data.peek().getType() == charKey)
           && data.peek(2).getType() == name && data.peek(3).getType() != lBracket) {
        varDefine();
        mustBeThisToken(data, semi);
    }
    mark("<变量说明>");
}

void Parser::argList() {
    Token type = printPop(data);                                                    // ＜参数表＞此处进入时禁止为空
    if (type.getType() != intKey && type.getType() != charKey)
        error(type);
    Token identifier = mustBeThisToken(data, name);
    while (data.peek().getType() == comma) {                                        // {,＜类型标识符＞＜标识符＞}
        printPop(data);
        Token type = printPop(data);
        if (type.getType() != intKey && type.getType() != charKey)
            error(type);
        Token identifier = mustBeThisToken(data, name);
    }
    // "<参数表>" 由上级有返回值函数定义、无返回值函数定义输出
}

void Parser::valueArgList() {
    expr();                                                                         // ＜值参数表＞此处进入时禁止为空
    while (data.peek().getType() == comma) {                                        // ＜表达式＞{,＜表达式＞}
        printPop(data);
        expr();
    }
    // "<值参数表>" 由上级有返回值函数调用语句、无返回值函数调用语句输出
}

void Parser::nonvoidFunc() {
    declareHead();
    mustBeThisToken(data, lBracket);
    if (data.peek().getType() != rBracket)                                          // ＜参数表＞此处禁止为空时进入，但是为空时也算＜参数表＞，要输出
        argList();
    mark("<参数表>");
    mustBeThisToken(data, rBracket);
    mustBeThisToken(data, lCurly);
    codeBlock();
    mustBeThisToken(data, rCurly);
    mark("<有返回值函数定义>");
}

void Parser::voidFunc() {
    mustBeThisToken(data, voidKey);
    Token identifier = mustBeThisToken(data, name);
    table[identifier.getText()] = Symbol(identifier.getText(), voidFunct);
    mustBeThisToken(data, lBracket);
    if (data.peek().getType() != rBracket)                                          // ＜参数表＞此处禁止为空时进入，但是为空时也算＜参数表＞，要输出
        argList();
    mark("<参数表>");
    mustBeThisToken(data, rBracket);
    mustBeThisToken(data, lCurly);
    codeBlock();
    mustBeThisToken(data, rCurly);
    mark("<无返回值函数定义>");
}

void Parser::mainFunc() {
    mustBeThisToken(data, voidKey);
    mustBeThisToken(data, mainKey);
    mustBeThisToken(data, lBracket);
    mustBeThisToken(data, rBracket);
    mustBeThisToken(data, lCurly);
    codeBlock();
    mustBeThisToken(data, rCurly);
    mark("<主函数>");
}

void Parser::program() {
    if (data.peek().getType() == constKey)
        constDeclare();
    if (data.peek(3).getType() != lBracket) {
        varDeclare();
    }
    while (!(data.peek().getType() == voidKey && data.peek(2).getType() == mainKey)) {
        if (data.peek().getType() == voidKey)
            voidFunc();
        else
            nonvoidFunc();
    }
    mainFunc();
    mark("<程序>");
}

Parser::Parser(PeekQueue<Token> data) {
    this->data = data;
    program();
}
