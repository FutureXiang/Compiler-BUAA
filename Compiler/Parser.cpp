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

void factor(PeekQueue<Token> &data) {
    if (data.peek().getType() == name) {
        Token identifier = printPop(data);                                          // ＜标识符＞
        if (data.peek().getType() == lSquare) {                                     // ＜标识符＞‘[’＜表达式＞‘]’
            printPop(data);
            expr(data);
            if (data.peek().getType() == rSquare)
                printPop(data);
            else
                error(data.peek());
        } else if (data.peek().getType() == lBracket) {                             // ＜标识符＞ '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段
            nonvoidCaller(data);
        }
    } else if (data.peek().getType() == lBracket) {                                 // ‘(’＜表达式＞‘)’
        printPop(data);
        expr(data);
        if (data.peek().getType() == rBracket)
            printPop(data);
        else
            error(data.peek());
    } else if (data.peek().getType() == pluss || data.peek().getType() == minuss) { // (+|-)＜无符号整数＞
        Token plus_minus = printPop(data);
        if (data.peek().getType() == intConst)
            Token intConstTk = printPop(data);
        else
            error(data.peek());
    } else if (data.peek().getType() == intConst) {                                 // ＜无符号整数＞
        Token intConstTk = printPop(data);
    } else if (data.peek().getType() == charConst) {                                // ＜字符＞
        Token charConstTk = printPop(data);
    } else
        error(data.peek());
    mark("＜因子＞");
}

void item(PeekQueue<Token> &data) {
    factor(data);
    while (data.peek().getType() == multi || data.peek().getType() == divd) {
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
    while (data.peek().getType() == pluss || data.peek().getType() == minuss) {
        Token plus_minus = printPop(data);
        item(data);
    }
    mark("＜表达式＞");
}

void returnStatement(PeekQueue<Token> &data) {
    if (data.peek().getType() == returnKey) {                                       // return['('＜表达式＞')']
        printPop(data);
        if (data.peek().getType() == lBracket) {
            printPop(data);
            expr(data);
            if (data.peek().getType() == rBracket)
                printPop(data);
            else
                error(data.peek());
        }
    } else
        error(data.peek());
    mark("＜返回语句＞");
}

void printfStatement(PeekQueue<Token> &data) {
    if (data.peek().getType() == print) {
        printPop(data);
        if (data.peek().getType() == lBracket) {                                    // printf '('
            printPop(data);
            
            if (data.peek().getType() == stringConst) {
                Token stringConstTk = printPop(data);                               // ＜字符串＞
                if (data.peek().getType() == comma) {                               // ＜字符串＞,＜表达式＞
                    printPop(data);
                    expr(data);
                }
            } else
                expr(data);                                                         // ＜表达式＞
            
            if (data.peek().getType() == rBracket)                                  // ')'
                printPop(data);
            else
                error(data.peek());
        } else
            error(data.peek());
    } else
        error(data.peek());
    mark("＜写语句＞");
}

void scanfStatement(PeekQueue<Token> &data) {
    if (data.peek().getType() == scan) {
        printPop(data);
        if (data.peek().getType() == lBracket) {                                    // scanf '('
            printPop(data);
            
            if (data.peek().getType() == name) {                                    // ＜标识符＞{,＜标识符＞}
                Token identifier = printPop(data);
                while (data.peek().getType() == comma) {
                    printPop(data);
                    Token identifier = printPop(data);
                }
            } else
                error(data.peek());
            
            if (data.peek().getType() == rBracket)                                  // ')'
                printPop(data);
            else
                error(data.peek());
        } else
            error(data.peek());
    } else
        error(data.peek());
    mark("＜读语句＞");
}

void assignStatement(PeekQueue<Token> &data) {
    if (data.peek().getType() == name) {                                            // ＜标识符＞
        Token identifier = printPop(data);
        if (data.peek().getType() == lSquare) {                                     // optional: '['＜表达式＞']'
            printPop(data);
            expr(data);
            if (data.peek().getType() == rSquare)
                printPop(data);
            else
                error(data.peek());
        }
        
        if (data.peek().getType() == assign) {                                      // ＝＜表达式＞
            printPop(data);
            expr(data);
        } else
            error(data.peek());
    } else
        error(data.peek());
    mark("＜赋值语句＞");
}

void voidCaller(PeekQueue<Token> &data) {
    if (data.peek().getType() == lBracket) {                                        // '('＜值参数表＞')' => ＜无返回值函数调用语句＞的不含<标识符>的后半段
        printPop(data);
        valueArgList(data);
        if (data.peek().getType() == rBracket)
            printPop(data);
        else
            error(data.peek());
    }
    mark("＜无返回值函数调用语句＞");
}

void nonvoidCaller(PeekQueue<Token> &data) {
    if (data.peek().getType() == lBracket) {                                        // '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段
        printPop(data);
        valueArgList(data);
        if (data.peek().getType() == rBracket)
            printPop(data);
        else
            error(data.peek());
    }
    mark("＜有返回值函数调用语句＞");
}

void loopStatement(PeekQueue<Token> &data) {
    
}

void ifStatement(PeekQueue<Token> &data) {
    
}

void statement(PeekQueue<Token> &data) {
    
}

void codeBlock(PeekQueue<Token> &data) {
    
}

void declareHead(PeekQueue<Token> &data) {
    
}

void constDefine(PeekQueue<Token> &data) {
    
}

void varDefine(PeekQueue<Token> &data) {
    
}

void constDeclare(PeekQueue<Token> &data) {
    
}

void varDeclare(PeekQueue<Token> &data) {
    
}

void argList(PeekQueue<Token> &data) {
    
}

void valueArgList(PeekQueue<Token> &data) {
    
}

void nonvoidFunc(PeekQueue<Token> &data) {
    
}

void voidFunc(PeekQueue<Token> &data) {
    
}

void mainFunc(PeekQueue<Token> &data) {
    
}

void program(PeekQueue<Token> &data) {
    expr(data);
    mark("<程序>");
}

void Parser(PeekQueue<Token> data) {
    program(data);
}
