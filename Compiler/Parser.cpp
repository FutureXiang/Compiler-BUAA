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
    } else {
        Token intConstTk = mustBeThisToken(data, intConst);                         // ＜无符号整数＞ => ＜整数＞
    }
    mark("＜整数＞");
}

void factor(PeekQueue<Token> &data) {
    if (data.peek().getType() == name) {
        Token identifier = printPop(data);                                          // ＜标识符＞
        if (data.peek().getType() == lSquare) {                                     // ＜标识符＞‘[’＜表达式＞‘]’
            printPop(data);
            expr(data);
            mustBeThisToken(data, rSquare);
        } else if (data.peek().getType() == lBracket) {                             // ＜标识符＞ '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段
            nonvoidCaller(data);
        }
    } else if (data.peek().getType() == lBracket) {                                 // ‘(’＜表达式＞‘)’
        printPop(data);
        expr(data);
        mustBeThisToken(data, rBracket);
    } else if (data.peek().getType() == charConst) {                                // ＜字符＞
        Token charConstTk = printPop(data);
    } else
        mustBeInteger(data);                                                        // <整数>
    mark("＜因子＞");
}

void item(PeekQueue<Token> &data) {
    factor(data);                                                                   // ＜因子＞
    while (data.peek().getType() == multi || data.peek().getType() == divd) {       // {＜乘法运算符＞＜因子＞}
        Token mul_div = printPop(data);
        factor(data);
    }
    mark("＜项＞");
}

void expr(PeekQueue<Token> &data) {
    bool firstItemNegative = false;
    if (data.peek().getType() == pluss) {
        printPop(data);
    } else if(data.peek().getType() == minuss) {
        firstItemNegative = true;
        printPop(data);
    }
    item(data);
    while (data.peek().getType() == pluss || data.peek().getType() == minuss) {     // {＜加法运算符＞＜项＞}
        Token plus_minus = printPop(data);
        item(data);
    }
    mark("＜表达式＞");
}

void returnStatement(PeekQueue<Token> &data) {
    mustBeThisToken(data, returnKey);                                               // return['('＜表达式＞')']
    if (data.peek().getType() == lBracket) {
        printPop(data);
        expr(data);
        mustBeThisToken(data, rBracket);
    }
    mark("＜返回语句＞");
}

void printfStatement(PeekQueue<Token> &data) {
    mustBeThisToken(data, print);
    mustBeThisToken(data, lBracket);                                                // printf '('
    if (data.peek().getType() == stringConst) {
        Token stringConstTk = printPop(data);                                       // ＜字符串＞
        mark("＜字符串＞");
        if (data.peek().getType() == comma) {                                       // ＜字符串＞,＜表达式＞
            printPop(data);
            expr(data);
        }
    } else
        expr(data);                                                                 // ＜表达式＞
    mustBeThisToken(data, rBracket);                                                // ')'
    mark("＜写语句＞");
}

void scanfStatement(PeekQueue<Token> &data) {
    mustBeThisToken(data, scan);
    mustBeThisToken(data, lBracket);                                                // scanf '('
    Token identifier = mustBeThisToken(data, name);                                 // ＜标识符＞{,＜标识符＞}
    while (data.peek().getType() == comma) {
        printPop(data);
        Token identifier = mustBeThisToken(data, name);
    }
    mustBeThisToken(data, rBracket);                                                // ')'
    mark("＜读语句＞");
}

void assignStatement(PeekQueue<Token> &data) {
    Token identifier = mustBeThisToken(data, name);                                 // ＜标识符＞
    if (data.peek().getType() == lSquare) {                                         // optional: '['＜表达式＞']'
        printPop(data);
        expr(data);
        mustBeThisToken(data, rSquare);
    }
    mustBeThisToken(data, assign);                                                  // ＝＜表达式＞
    expr(data);
    mark("＜赋值语句＞");
}

void voidCaller(PeekQueue<Token> &data) {
    mustBeThisToken(data, lBracket);                                                // '('＜值参数表＞')' => ＜无返回值函数调用语句＞的不含<标识符>的后半段
    if (data.peek().getType() != rBracket)                                          // ＜值参数表＞此处禁止为空时进入，但是为空时也算＜值参数表＞，要输出
        valueArgList(data);
    mark("＜值参数表＞");
    mustBeThisToken(data, rBracket);
    mark("＜无返回值函数调用语句＞");
}

void nonvoidCaller(PeekQueue<Token> &data) {
    mustBeThisToken(data, lBracket);                                                // '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段
    if (data.peek().getType() != rBracket)                                          // ＜值参数表＞此处禁止为空时进入，但是为空时也算＜值参数表＞，要输出
        valueArgList(data);
    mark("＜值参数表＞");
    mustBeThisToken(data, rBracket);
    mark("＜有返回值函数调用语句＞");
}

void loopStatement(PeekQueue<Token> &data) {
    if (data.peek().getType() == whileKey) {                                        //  while '('＜条件＞')'＜语句＞
        printPop(data);
        mustBeThisToken(data, lBracket);
        condition(data);
        mustBeThisToken(data, rBracket);
        statement(data);
    } else if (data.peek().getType() == doKey) {                                    // do＜语句＞while '('＜条件＞')'
        printPop(data);
        statement(data);
        mustBeThisToken(data, whileKey);
        mustBeThisToken(data, lBracket);
        condition(data);
        mustBeThisToken(data, rBracket);
    } else {
        mustBeThisToken(data, forKey);                                              // for'(' ＜标识符＞＝＜表达式＞;＜条件＞; ＜标识符＞＝＜标识符＞(+|-)＜步长＞ ')'＜语句＞
        mustBeThisToken(data, lBracket);
        Token identifier = mustBeThisToken(data, name);
        mustBeThisToken(data, assign);
        expr(data);
        mustBeThisToken(data, semi);
        condition(data);
        mustBeThisToken(data, semi);
        
        Token identifier2 = mustBeThisToken(data, name);
        mustBeThisToken(data, assign);
        Token identifier3 = mustBeThisToken(data, name);
        if (data.peek().getType() == pluss || data.peek().getType() == minuss) {
            Token plus_minus = printPop(data);
            Token step = mustBeThisToken(data, intConst);
            mark("＜无符号整数＞");
            mark("<步长>");
        } else
            error(data.peek());
        mustBeThisToken(data, rBracket);
        statement(data);
    }
    mark("＜循环语句＞");
}

void condition(PeekQueue<Token> &data) {
    expr(data);                                                                     // ＜表达式＞
    TokenType nextType = data.peek().getType();
    if (nextType == lesss || nextType == leq || nextType == great || nextType == geq || nextType == neq || nextType == equalto) {
        Token relationship = printPop(data);                                        // ＜表达式＞＜关系运算符＞＜表达式＞
        expr(data);
    }
    mark("＜条件＞");
}

void ifStatement(PeekQueue<Token> &data) {
    mustBeThisToken(data, ifKey);                                                   // if '('＜条件＞')'＜语句＞
    mustBeThisToken(data, lBracket);
    condition(data);
    mustBeThisToken(data, rBracket);
    statement(data);
    if (data.peek().getType() == elseKey) {                                         // [else＜语句＞]
        printPop(data);
        statement(data);
    }
    mark("＜条件语句＞");
}

void statement(PeekQueue<Token> &data) {
    TokenType nextType = data.peek().getType();
    switch (nextType) {
        case ifKey:                                                                 // ＜条件语句＞
            ifStatement(data);
            break;
        case whileKey:                                                              // ＜循环语句＞
        case doKey:
        case forKey:
            loopStatement(data);
            break;
        case lCurly:                                                                // '{'＜语句列＞'}'
            printPop(data);
            statementS(data);
            mustBeThisToken(data, rCurly);
            break;
        case name:                                                                  // ＜赋值语句＞;
            assignStatement(data);
            mustBeThisToken(data, semi);
            break;
        case scan:                                                                  // ＜读语句＞;
            scanfStatement(data);
            mustBeThisToken(data, semi);
            break;
        case print:                                                                 // ＜写语句＞;
            printfStatement(data);
            mustBeThisToken(data, semi);
            break;
        case semi:                                                                  // ＜空＞;
            printPop(data);
            break;
        case returnKey:                                                             // ＜返回语句＞;
            returnStatement(data);
            mustBeThisToken(data, semi);
            break;
        default:                                                                    // ＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞;
            // TODO: 如何区分void和non-void ?????
            mustBeThisToken(data, semi);
            break;
    }
    mark("＜语句＞");
}

void statementS(PeekQueue<Token> &data) {
    while (data.peek().getType() != rCurly) {                                       // {＜语句＞}
        statement(data);
    }
    mark("＜语句列＞");
}

void codeBlock(PeekQueue<Token> &data) {
    if (data.peek().getType() == constKey)
        constDeclare(data);
    if (data.peek().getType() == intKey || data.peek().getType() == charKey) {
        varDeclare(data);
    }
    statementS(data);
    mark("＜复合语句＞");
}

void declareHead(PeekQueue<Token> &data) {
    Token type = printPop(data);
    if (type.getType() != intKey && type.getType() != charKey)                      // int|char
        error(type);
    Token identifier = mustBeThisToken(data, name);
    mark("＜声明头部＞");
}

void constDefine(PeekQueue<Token> &data) {
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
    mark("＜常量定义＞");
}

void varDefine(PeekQueue<Token> &data) {
    Token type = printPop(data);
    if (type.getType() != intKey && type.getType() != charKey)                      // int|char
        error(type);
    
    Token identifier = mustBeThisToken(data, name);                                 // ＜标识符＞|＜标识符＞'['＜无符号整数＞']'
    if (data.peek().getType() == lSquare) {
        printPop(data);
        Token arrayLength = mustBeThisToken(data, intConst);
        mark("＜无符号整数＞");
        mustBeThisToken(data, rSquare);
    }
    while (data.peek().getType() == comma) {                                        // {,(＜标识符＞|＜标识符＞'['＜无符号整数＞']')}
        printPop(data);
        Token identifier = mustBeThisToken(data, name);
        if (data.peek().getType() == lSquare) {
            printPop(data);
            Token arrayLength = mustBeThisToken(data, intConst);
            mark("＜无符号整数＞");
            mustBeThisToken(data, rSquare);
        }
    }
    mark("＜变量定义＞");
}

void constDeclare(PeekQueue<Token> &data) {                                         // const＜常量定义＞;{const＜常量定义＞;}
    mustBeThisToken(data, constKey);
    constDefine(data);
    mustBeThisToken(data, semi);
    while (data.peek().getType() == constKey) {
        printPop(data);
        constDefine(data);
        mustBeThisToken(data, semi);
    }
    mark("＜常量说明＞");
}

void varDeclare(PeekQueue<Token> &data) {                                           // ＜变量定义＞;{＜变量定义＞;}
    varDefine(data);
    mustBeThisToken(data, semi);
    while (data.peek(3).getType() != lBracket) {
        varDefine(data);
        mustBeThisToken(data, semi);
    }
    mark("＜变量说明＞");
}

void argList(PeekQueue<Token> &data) {
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
    // "＜参数表＞" 由上级有返回值函数定义、无返回值函数定义输出
}

void valueArgList(PeekQueue<Token> &data) {
    expr(data);                                                                     // ＜值参数表＞此处进入时禁止为空
    while (data.peek().getType() == comma) {                                        // ＜表达式＞{,＜表达式＞}
        printPop(data);
        expr(data);
    }
    // "＜值参数表＞" 由上级有返回值函数调用语句、无返回值函数调用语句输出
}

void nonvoidFunc(PeekQueue<Token> &data) {
    declareHead(data);
    mustBeThisToken(data, lBracket);
    if (data.peek().getType() != rBracket)                                          // ＜参数表＞此处禁止为空时进入，但是为空时也算＜参数表＞，要输出
        argList(data);
    mark("＜参数表＞");
    mustBeThisToken(data, rBracket);
    mustBeThisToken(data, lCurly);
    codeBlock(data);
    mustBeThisToken(data, rCurly);
    mark("＜有返回值函数定义＞");
}

void voidFunc(PeekQueue<Token> &data) {
    mustBeThisToken(data, voidKey);
    Token identifier = mustBeThisToken(data, name);
    mustBeThisToken(data, lBracket);
    if (data.peek().getType() != rBracket)                                          // ＜参数表＞此处禁止为空时进入，但是为空时也算＜参数表＞，要输出
        argList(data);
    mark("＜参数表＞");
    mustBeThisToken(data, rBracket);
    mustBeThisToken(data, lCurly);
    codeBlock(data);
    mustBeThisToken(data, rCurly);
    mark("＜无返回值函数定义＞");
}

void mainFunc(PeekQueue<Token> &data) {
    mustBeThisToken(data, voidKey);
    mustBeThisToken(data, mainKey);
    mustBeThisToken(data, lBracket);
    mustBeThisToken(data, rBracket);
    mustBeThisToken(data, lCurly);
    codeBlock(data);
    mustBeThisToken(data, rCurly);
    mark("＜主函数＞");
}

void program(PeekQueue<Token> &data) {
    if (data.peek().getType() == constKey)
        constDeclare(data);
    if (data.peek(3).getType() != lBracket) {
        varDeclare(data);
    }
    while (!(data.peek().getType() == voidKey && data.peek(2).getType() == mainKey)) {
        if (data.peek().getType() == voidKey)
            voidFunc(data);
        else
            nonvoidFunc(data);
    }
    mainFunc(data);
    mark("<程序>");
}

void Parser(PeekQueue<Token> data) {
    program(data);
}
