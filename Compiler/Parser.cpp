//
//  Parser.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/27.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Parser.hpp"
#include "PeekQueue.cpp"

void Parser::error(Error e) {
    errorMessages.insert(std::make_pair(line, ErrorToString(e)));
}

void Parser::checkCallerMatch(bool is_void, Token id, std::vector<ExprType> argtypes) {
    // must exists identifier now (checked)
    std::shared_ptr<Symbol> symbol = table.getSymbolByName(id.getText());
    if (!symbol->isFunc() || is_void != symbol->isVoidFunc()) {
        error(__othererror__);
        return;
    }
    
    std::shared_ptr<SymbolFunct> func = std::dynamic_pointer_cast<SymbolFunct>(symbol);
    std::vector<std::shared_ptr<SymbolVar> > args = func->args;
    
    if (args.size() != argtypes.size()) {
        error(args_lenmis);
        return;
    }
    
    for (auto i = 0; i < args.size(); ++i) {
        SymbolType arg_type_expected = args[i]->getType();
        ExprType arg_type_got = argtypes[i];
        if (!( (arg_type_got == charType && arg_type_expected == charVar)
            || (arg_type_got == intType && arg_type_expected == intVar) )) {
            error(args_typemis);
            return;
        }
    }
}


Token Parser::printPop() {
    Token token = data.pop();
    line = token.lineNo;
//    std::cout << token.toString() << std::endl;
    return token;
}

void mark(std::string str) {
//    std::cout << str << std::endl;
}

Token Parser::mustBeThisToken(TokenType type) {
    if (data.peek().getType() == type)
        return printPop();              // print, pop, return
    else {
        if (data.peek().getType() == __invalidToken__) {
            data.pop();
            return Token("", __invalidToken__);
        }
        if (type == semi)
            error(semi_mis);
        else if (type == rBracket)
            error(bracket_mis);
        else if (type == rSquare)
            error(square_mis);
        else if (type == whileKey)
            error(while_mis);
        else {
//            error(__othererror__); // error
        }
        return Token("", __invalidToken__);
    }
}

int Parser::mustBeInteger() {
    int value = 0;
    if (data.peek().getType() == pluss || data.peek().getType() == minuss) {        // (+|-)＜无符号整数＞ => ＜整数＞
        Token plus_minus = printPop();
        Token intConstTk = mustBeThisToken(intConst);
        mark("<无符号整数>");
        value = std::stoi(intConstTk.getText());
        if (plus_minus.getType() == minuss)
            value = -value;
    } else {
        Token intConstTk = mustBeThisToken(intConst);                               // ＜无符号整数＞ => ＜整数＞
        mark("<无符号整数>");
        value = std::stoi(intConstTk.getText());
    }
    mark("<整数>");
    return value;
}

ExprType Parser::factor(Operand *&operand) {
    ExprType __factor__type__ = intType;
    if (data.peek().getType() == name) {
        Token identifier = printPop();                                              // ＜标识符＞
        if (!table.containsByName(identifier.getText())) {
            error(id_nodef);
        } else {
            if (table.getTypeByName(identifier.getText()) == charCon
                || table.getTypeByName(identifier.getText()) == charVar)
                __factor__type__ = charType;                            // char型因子
        }
        
        Operand *sym_operand = qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()));
        
        if (data.peek().getType() == lSquare) {                                     // ＜标识符＞‘[’＜表达式＞‘]’
            printPop();
            
            /* QUAD-CODE: LOAD ELEMENT TO SYMBOL */
            Operand *subscript = nullptr;
            ExprType subscriptType = expr(subscript);
            operand = qcodes.allocTemp();       // alloc AFTER expr (otherwise it will OCCUPY A TEMP)
            qcodes.addCode(Quadruple(LARR, operand, sym_operand, subscript));
            
            if (subscriptType != intType)
                error(index_notint);
            mustBeThisToken(rSquare);
            if (table.containsByName(identifier.getText()) &&
                table.getTypeByName(identifier.getText()) == charArr)
                __factor__type__ = charType;                            // char型因子

        } else if (data.peek().getType() == lBracket) {                             // ＜标识符＞ '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段

            /* QUAD-CODE: LOAD ARGUMENTS, CALL FUNCTION BY LABEL,
                          THEN GET RETURN VALUE */
            nonvoidCaller(identifier);
            qcodes.addCode(Quadruple(CALL, new OperandLabel("__"+identifier.getText()+"__")));
            operand = qcodes.allocTemp();
            qcodes.addCode(Quadruple(MV, operand, (Operand *)&qcodes.v0Symbol));
            
            if (table.containsByName(identifier.getText()) &&
                table.getTypeByName(identifier.getText()) == charFunct)
                __factor__type__ = charType;                            // char型因子
        } else {                                                                    // ＜标识符＞
            std::shared_ptr<Symbol> thisSymbol = table.getSymbolByName(identifier.getText());
            if (thisSymbol->isCon()) {
                std::shared_ptr<SymbolVar> thisConSymbol = std::dynamic_pointer_cast<SymbolVar>(thisSymbol);
                operand = new OperandInstant(thisConSymbol->getValueAsInt());
            }
            else
                operand = sym_operand;
        }
    } else if (data.peek().getType() == lBracket) {                                 // ‘(’＜表达式＞‘)’
        printPop();
        expr(operand);
        mustBeThisToken(rBracket);
    } else if (data.peek().getType() == charConst) {                                // ＜字符＞
        Token charConstTk = printPop();
        __factor__type__ = charType;
        
        operand = new OperandInstant(charConstTk.getText()[0]);
    } else {
        int intValue = mustBeInteger();                                             // <整数>
        
        operand = new OperandInstant(intValue);
    }
    mark("<因子>");
    return __factor__type__;
    // <表达式> ::= <char标识符>|＜char标识符＞'['＜表达式＞'] | <char型有返回值调用> | <字符>
    //         ---> 表达式的结果为char型
}

ExprType Parser::item(Operand *&operand) {
    Operand *first, *next;
    ExprType __factor__type__ = factor(first);                                      // ＜因子＞ 及其type
    while (data.peek().getType() == multi || data.peek().getType() == divd) {       // {＜乘法运算符＞＜因子＞}
        Token mul_div = printPop();
        factor(next);
        __factor__type__ = intType;
        
        /* QUAD-CODE: ARITHMETIC */
        Operand *temp = qcodes.allocTemp();
        if (mul_div.getType() == multi) {
            qcodes.addCode(Quadruple(MULT, temp, first, next));
        } else {
            qcodes.addCode(Quadruple(DIV, temp, first, next));
        }
        first = temp;
    }
    operand = first;
    mark("<项>");
    return __factor__type__;
}

ExprType Parser::expr(Operand *&operand) {
    Operand *first, *next;
    bool firstItemNegative = false;
    if (data.peek().getType() == pluss) {
        printPop();
    } else if(data.peek().getType() == minuss) {
        firstItemNegative = true;
        printPop();
    }
    ExprType __factor__type__ = item(first);
    if (firstItemNegative) {
        /* QUAD-CODE: ARITHMETIC */
        Operand *temp = qcodes.allocTemp();
        qcodes.addCode(Quadruple(SUB, temp, (Operand *)&qcodes.zeroInstant, first));
        first = temp;
    }

    while (data.peek().getType() == pluss || data.peek().getType() == minuss) {     // {＜加法运算符＞＜项＞}
        Token plus_minus = printPop();
        item(next);
        __factor__type__ = intType;
        
        /* QUAD-CODE: ARITHMETIC */
        Operand *temp = qcodes.allocTemp();
        if (plus_minus.getType() == pluss) {
            qcodes.addCode(Quadruple(ADD, temp, first, next));
        } else {
            qcodes.addCode(Quadruple(SUB, temp, first, next));
        }
        first = temp;
    }
    operand = first;
    mark("<表达式>");
    return __factor__type__;
}

void Parser::returnStatement() {
    ExprType type = voidType;
    mustBeThisToken(returnKey);                                                     // return['('＜表达式＞')']
    if (data.peek().getType() == lBracket) {
        printPop();
        Operand *temp = nullptr;
        type = expr(temp);
        
        /* QUAD-CODE: MOVE RETURN-VALUE TO v0, THEN RETURN */
        qcodes.addCode(Quadruple(MV, (Operand *)&qcodes.v0Symbol, temp));
        qcodes.addCode(Quadruple(RET));
        
        mustBeThisToken(rBracket);
    }
    if (type != expected_return) {
        if (expected_return == voidType)
            error(void_misret);
        else
            error(nonvoid_misret);
    }
    mark("<返回语句>");
}

void Parser::printfStatement() {
    Operand *temp = nullptr;
    mustBeThisToken(print);
    mustBeThisToken(lBracket);                                                      // printf '('
    if (data.peek().getType() == stringConst) {
        Token stringConstTk = printPop();                                           // ＜字符串＞
        mark("<字符串>");
        Operand *string = new OperandString(stringConstTk.getText(), qcodes.allocStringName());

        /* QUAD-CODE: .asciiz string & syscall write string */
        qcodes.getQCodes()->insert(qcodes.getQCodes()->begin(), Quadruple(VAR, string));
        qcodes.addCode(Quadruple(WRITE_STR, string));

        if (data.peek().getType() == comma) {                                       // ＜字符串＞,＜表达式＞
            printPop();
            ExprType resultType = expr(temp);
            if (resultType == intType)
                qcodes.addCode(Quadruple(WRITE_INT, temp));
            else
                qcodes.addCode(Quadruple(WRITE_CHAR, temp));
        }
    } else {
        ExprType resultType = expr(temp);                                           // ＜表达式＞
        if (resultType == intType)
            qcodes.addCode(Quadruple(WRITE_INT, temp));
        else
            qcodes.addCode(Quadruple(WRITE_CHAR, temp));
    }
    mustBeThisToken(rBracket);                                                      // ')'
    mark("<写语句>");
    

    /* QUAD-CODE: .asciiz "\n" */
    qcodes.addCode(Quadruple(WRITE_STR, (Operand *)&qcodes.slashN));
}

void Parser::scanfStatement() {
    mustBeThisToken(scan);
    mustBeThisToken(lBracket);                                                      // scanf '('
    Token identifier = mustBeThisToken(name);                                       // ＜标识符＞{,＜标识符＞}
    if (!table.containsByName(identifier.getText()))
        error(id_nodef);
    SymbolType type = table.getTypeByName(identifier.getText());
    if (type == intCon || type == charCon)
        error(const_assign);
    else if (type == intVar)
        /* QUAD-CODE: syscall read variable */
        qcodes.addCode(Quadruple(READ_INT, qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()))));
    else if (type == charVar)
        qcodes.addCode(Quadruple(READ_CHAR, qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()))));

    while (data.peek().getType() == comma) {
        printPop();
        Token identifier = mustBeThisToken(name);
        if (!table.containsByName(identifier.getText()))
            error(id_nodef);
        SymbolType type = table.getTypeByName(identifier.getText());
        if (type == intCon || type == charCon)
            error(const_assign);
        else if (type == intVar)
            /* QUAD-CODE: syscall read variable */
            qcodes.addCode(Quadruple(READ_INT, qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()))));
        else if (type == charVar)
            qcodes.addCode(Quadruple(READ_CHAR, qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()))));
    }
    mustBeThisToken(rBracket);                                                      // ')'
    mark("<读语句>");
}

void Parser::assignStatement() {
    Operand *result = nullptr;
    Token identifier = mustBeThisToken(name);                                       // ＜标识符＞
    if (!table.containsByName(identifier.getText()))
        error(id_nodef);
    SymbolType type = table.getTypeByName(identifier.getText());
    if (type == intCon || type == charCon)
        error(const_assign);
    
    Operand *sym_operand = qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()));
    
    if (data.peek().getType() == lSquare) {                                         // optional: '['＜表达式＞']'
        printPop();
        
        Operand *subscript = nullptr;
        ExprType subscriptType = expr(subscript);

        if (subscriptType != intType)
            error(index_notint);
        mustBeThisToken(rSquare);
        mustBeThisToken(assign);                                                    // ＜标识符＞'['＜表达式＞']'＝＜表达式＞
        expr(result);
        
        /* QUAD-CODE: SAVE VALUE TO ELEMENT */
        qcodes.addCode(Quadruple(SARR, result, sym_operand, subscript));
    } else {
        mustBeThisToken(assign);                                                    // ＜标识符＞＝＜表达式＞
        expr(result);
        
        /* QUAD-CODE: SAVE VALUE TO SYMBOL */
        qcodes.addCode(Quadruple(MV, sym_operand, result));
    }
    mark("<赋值语句>");
}

void Parser::voidCaller(Token identifier, bool check_argmatch) {
    mustBeThisToken(lBracket);                                                      // '('＜值参数表＞')' => ＜无返回值函数调用语句＞的不含<标识符>的后半段
    std::vector<ExprType> argtypes;
    if (data.peek().getType() != rBracket)                                          // ＜值参数表＞此处禁止为空时进入，但是为空时也算＜值参数表＞，要输出
        argtypes = valueArgList();
    mark("<值参数表>");
    mustBeThisToken(rBracket);
    mark("<无返回值函数调用语句>");
    if (check_argmatch)
        checkCallerMatch(true, identifier, argtypes);
}

void Parser::nonvoidCaller(Token identifier) {
    mustBeThisToken(lBracket);                                                      // '('＜值参数表＞')' => ＜有返回值函数调用语句＞的不含<标识符>的后半段
    std::vector<ExprType> argtypes;
    if (data.peek().getType() != rBracket)                                          // ＜值参数表＞此处禁止为空时进入，但是为空时也算＜值参数表＞，要输出
        argtypes = valueArgList();
    mark("<值参数表>");
    mustBeThisToken(rBracket);
    mark("<有返回值函数调用语句>");
    checkCallerMatch(false, identifier, argtypes);
}

void Parser::loopStatement() {
    if (data.peek().getType() == whileKey) {                                        //  while '('＜条件＞')'＜语句＞
        printPop();
        mustBeThisToken(lBracket);
        condition();
        mustBeThisToken(rBracket);
        statement();
    } else if (data.peek().getType() == doKey) {                                    // do＜语句＞while '('＜条件＞')'
        printPop();
        statement();
        mustBeThisToken(whileKey);
        mustBeThisToken(lBracket);
        condition();
        mustBeThisToken(rBracket);
    } else {
        mustBeThisToken(forKey);                                                    // for'(' ＜标识符＞＝＜表达式＞;＜条件＞; ＜标识符＞＝＜标识符＞(+|-)＜步长＞ ')'＜语句＞
        mustBeThisToken(lBracket);
        Token identifier = mustBeThisToken(name);
        if (!table.containsByName(identifier.getText()))
            error(id_nodef);
        mustBeThisToken(assign);
        Operand *temp = nullptr;
        expr(temp);
        mustBeThisToken(semi);
        condition();
        mustBeThisToken(semi);
        
        Token identifier2 = mustBeThisToken(name);
        if (!table.containsByName(identifier2.getText()))
            error(id_nodef);
        mustBeThisToken(assign);
        Token identifier3 = mustBeThisToken(name);
        if (!table.containsByName(identifier3.getText()))
            error(id_nodef);
        if (data.peek().getType() == pluss || data.peek().getType() == minuss) {
            Token plus_minus = printPop();
            Token step = mustBeThisToken(intConst);
            mark("<无符号整数>");
            mark("<步长>");
        } else {
//            error(__othererror__);
        }
        mustBeThisToken(rBracket);
        statement();
    }
    mark("<循环语句>");
}

void Parser::condition() {
    Operand *temp = nullptr;
    ExprType first = expr(temp);                                                        // ＜表达式＞
    TokenType nextType = data.peek().getType();
    if (nextType == lesss || nextType == leq || nextType == great || nextType == geq || nextType == neq || nextType == equalto) {
        Token relationship = printPop();                                            // ＜表达式＞＜关系运算符＞＜表达式＞
        ExprType second = expr(temp);
        if (first != intType || second != intType)
            error(cond_invalid);
    } else {
        if (first != intType)
            error(cond_invalid);
    }
    mark("<条件>");
}

void Parser::ifStatement() {
    mustBeThisToken(ifKey);                                                         // if '('＜条件＞')'＜语句＞
    mustBeThisToken(lBracket);
    condition();
    mustBeThisToken(rBracket);
    statement();
    if (data.peek().getType() == elseKey) {                                         // [else＜语句＞]
        printPop();
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
            printPop();
            statementS();
            mustBeThisToken(rCurly);
            break;
        case scan:                                                                  // ＜读语句＞;
            scanfStatement();
            mustBeThisToken(semi);
            break;
        case print:                                                                 // ＜写语句＞;
            printfStatement();
            mustBeThisToken(semi);
            break;
        case semi:                                                                  // ＜空＞;
            printPop();
            break;
        case returnKey:                                                             // ＜返回语句＞;
            returnStatement();
            has_return = true;
            mustBeThisToken(semi);
            break;
        default:                                                                    // ＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞; | ＜赋值语句＞;
            if (data.peek(2).getType() == assign || data.peek(2).getType() == lSquare) {
                assignStatement();
                mustBeThisToken(semi);
            } else {
                Token identifier = mustBeThisToken(name);
                if (!table.containsByName(identifier.getText())) {
                    error(id_nodef);
                    voidCaller(identifier, false);
                } else {
                    if (table.getSymbolByName(identifier.getText())->isVoidFunc())
                        voidCaller(identifier, true);
                    else
                        nonvoidCaller(identifier);
                }
                mustBeThisToken(semi);
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
        constDeclare(false);
    if (data.peek().getType() == intKey || data.peek().getType() == charKey) {
        varDeclare(false);
    }
    has_return = false;
    statementS();
    if (expected_return != voidType && !has_return)
        error(nonvoid_misret);
    mark("<复合语句>");
}

void Parser::constDefine(bool is_global) {
    if (data.peek().getType() == intKey) {                                          // int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
        Token type = printPop();
        Token identifier = mustBeThisToken(name);
        if (!table.addVar(identifier.getText(), intCon, is_global))
            error(id_redef);
        std::shared_ptr<Symbol> thisSymbol = table.getSymbolByName(identifier.getText());
        std::shared_ptr<SymbolVar> thisConSymbol = std::dynamic_pointer_cast<SymbolVar>(thisSymbol);
        
        mustBeThisToken(assign);
        if (data.peek().getType() == pluss || data.peek().getType() == minuss) {    // (+|-)＜无符号整数＞ => ＜整数＞
            Token plus_minus = printPop();
            if (data.peek().getType() == intConst) {
                Token intConstTk = printPop();
                int value = std::stoi(intConstTk.getText());
                if (plus_minus.getType() == minuss)
                    value = -value;
                thisConSymbol->setValueAsInt(value);
            } else {
                error(const_value);
                data.pop();
            }
            mark("<无符号整数>");
        } else {
            if (data.peek().getType() == intConst) {                                // ＜无符号整数＞ => ＜整数＞
                Token intConstTk = printPop();
                int value = std::stoi(intConstTk.getText());
                thisConSymbol->setValueAsInt(value);
            } else {
                error(const_value);
                data.pop();
            }
            mark("<无符号整数>");
        }
        mark("<整数>");
        while (data.peek().getType() == comma) {
            printPop();
            Token identifier = mustBeThisToken(name);
            if (!table.addVar(identifier.getText(), intCon, is_global))
                error(id_redef);
            std::shared_ptr<Symbol> thisSymbol = table.getSymbolByName(identifier.getText());
            std::shared_ptr<SymbolVar> thisConSymbol = std::dynamic_pointer_cast<SymbolVar>(thisSymbol);
            
            mustBeThisToken(assign);
            if (data.peek().getType() == pluss || data.peek().getType() == minuss) {// (+|-)＜无符号整数＞ => ＜整数＞
                Token plus_minus = printPop();
                if (data.peek().getType() == intConst) {
                    Token intConstTk = printPop();
                    int value = std::stoi(intConstTk.getText());
                    if (plus_minus.getType() == minuss)
                        value = -value;
                    thisConSymbol->setValueAsInt(value);
                } else {
                    error(const_value);
                    data.pop();
                }
                mark("<无符号整数>");
            } else {
                if (data.peek().getType() == intConst) {                            // ＜无符号整数＞ => ＜整数＞
                    Token intConstTk = printPop();
                    int value = std::stoi(intConstTk.getText());
                    thisConSymbol->setValueAsInt(value);
                } else {
                    error(const_value);
                    data.pop();
                }
                mark("<无符号整数>");
            }
            mark("<整数>");
        }
    } else if (data.peek().getType() == charKey) {                                  // char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
        Token type = printPop();
        Token identifier = mustBeThisToken(name);
        if (!table.addVar(identifier.getText(), charCon, is_global))
            error(id_redef);
        std::shared_ptr<Symbol> thisSymbol = table.getSymbolByName(identifier.getText());
        std::shared_ptr<SymbolVar> thisConSymbol = std::dynamic_pointer_cast<SymbolVar>(thisSymbol);
        
        mustBeThisToken(assign);
        if (data.peek().getType() == charConst) {
            Token charConstTk = printPop();
            char value = charConstTk.getText()[0];
            thisConSymbol->setValueAsInt(value);
        } else {
            error(const_value);
            data.pop();
        }
        while (data.peek().getType() == comma) {
            printPop();
            Token identifier = mustBeThisToken(name);
            if (!table.addVar(identifier.getText(), charCon, is_global))
                error(id_redef);
            std::shared_ptr<Symbol> thisSymbol = table.getSymbolByName(identifier.getText());
            std::shared_ptr<SymbolVar> thisConSymbol = std::dynamic_pointer_cast<SymbolVar>(thisSymbol);
            
            mustBeThisToken(assign);
            if (data.peek().getType() == charConst) {
                Token charConstTk = printPop();
                char value = charConstTk.getText()[0];
                thisConSymbol->setValueAsInt(value);
            } else {
                error(const_value);
                data.pop();
            }
        }
    } else {
//        error(__othererror__);
    }
    mark("<常量定义>");
}

void Parser::varDefine(bool is_global) {
    Token type = printPop();
    if (type.getType() != intKey && type.getType() != charKey) {                    // int|char
//        error(__othererror__);
    }
    
    Token identifier = mustBeThisToken(name);                                       // ＜标识符＞|＜标识符＞'['＜无符号整数＞']'
    if (data.peek().getType() == lSquare) {
        printPop();
        Token arrayLength = mustBeThisToken(intConst);
        if (!table.addArr(identifier.getText(), type.getType()==intKey ? intArr : charArr, is_global, arrayLength.getText()))
            error(id_redef);
        
        mark("<无符号整数>");
        mustBeThisToken(rSquare);

        /* QUAD-CODE: variable array define */
        qcodes.addCode(Quadruple(VAR,
                                 qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText())),
                                 new OperandInstant(std::stoi(arrayLength.getText()))));
    } else {
        if (!table.addVar(identifier.getText(), type.getType()==intKey ? intVar : charVar, is_global))
            error(id_redef);

        /* QUAD-CODE: variable define */
        qcodes.addCode(Quadruple(VAR,
                                 qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()))));
    }
    while (data.peek().getType() == comma) {                                        // {,(＜标识符＞|＜标识符＞'['＜无符号整数＞']')}
        printPop();
        Token identifier = mustBeThisToken(name);
        if (data.peek().getType() == lSquare) {
            printPop();
            Token arrayLength = mustBeThisToken(intConst);
            if (!table.addArr(identifier.getText(), type.getType()==intKey ? intArr : charArr, is_global, arrayLength.getText()))
                error(id_redef);
            
            mark("<无符号整数>");
            mustBeThisToken(rSquare);

            /* QUAD-CODE: variable array define */
            qcodes.addCode(Quadruple(VAR,
                                     qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText())),
                                     new OperandInstant(std::stoi(arrayLength.getText()))));
        } else {
           if (!table.addVar(identifier.getText(), type.getType()==intKey ? intVar : charVar, is_global))
               error(id_redef);

            /* QUAD-CODE: variable define */
            qcodes.addCode(Quadruple(VAR,
                                     qcodes.getOperandSymbol(table.getSymbolByName(identifier.getText()))));
        }
    }
    mark("<变量定义>");
}

void Parser::constDeclare(bool is_global) {                                         // const＜常量定义＞;{const＜常量定义＞;}
    mustBeThisToken(constKey);
    constDefine(is_global);
    mustBeThisToken(semi);
    while (data.peek().getType() == constKey) {
        printPop();
        constDefine(is_global);
        mustBeThisToken(semi);
    }
    mark("<常量说明>");
}

// 程序中的变量说明：截止时，后面必为(nonvoid|void|main)函数定义，因此不能用int|char判断，应该用左括号判断
// 复合语句中的变量说明：截止时，后面是语句列，因此可以用int|char判断
void Parser::varDeclare(bool is_global) {                                           // ＜变量定义＞;{＜变量定义＞;}
    varDefine(is_global);
    mustBeThisToken(semi);
    while ((data.peek().getType() == intKey || data.peek().getType() == charKey)
           && data.peek(2).getType() == name && data.peek(3).getType() != lBracket) {
        varDefine(is_global);
        mustBeThisToken(semi);
    }
    mark("<变量说明>");
}

void Parser::argList(std::vector<std::shared_ptr<SymbolVar> > &args) {
    Token type = printPop();                                                        // ＜参数表＞此处进入时禁止为空
    if (type.getType() != intKey && type.getType() != charKey) {
//        error(__othererror__);
    }
    Token identifier = mustBeThisToken(name);
    std::shared_ptr<SymbolVar> arg = std::make_shared<SymbolVar>(identifier.getText(), type.getType()==intKey ? intVar : charVar, false);
    args.push_back(arg);
    
    while (data.peek().getType() == comma) {                                        // {,＜类型标识符＞＜标识符＞}
        printPop();
        Token type = printPop();
        if (type.getType() != intKey && type.getType() != charKey) {
//            error(__othererror__);
        }
        Token identifier = mustBeThisToken(name);
        std::shared_ptr<SymbolVar> arg = std::make_shared<SymbolVar>(identifier.getText(), type.getType()==intKey ? intVar : charVar, false);
        args.push_back(arg);
    }
    // "<参数表>" 由上级有返回值函数定义、无返回值函数定义输出
}

std::vector<ExprType> Parser::valueArgList() {
    Operand *arg = nullptr;
    std::vector<ExprType> argtypes;
    std::vector<Operand *> argvalues;
    argtypes.push_back(expr(arg));                                                  // ＜值参数表＞此处进入时禁止为空
    argvalues.push_back(arg);
    
    while (data.peek().getType() == comma) {                                        // ＜表达式＞{,＜表达式＞}
        printPop();
        argtypes.push_back(expr(arg));
        argvalues.push_back(arg);
    }
    /* QUAD-CODE: MOVE VALUES TO a0, a1, a2, ...  AT THE END !!! [RECURSIVE: func(1,2,func(3,4,5))] */
    for (int i = 0; i < argvalues.size(); ++i)
        qcodes.addCode(Quadruple(MV, new OperandSymbol("a" + std::to_string(i)), argvalues[i]));
    // "<值参数表>" 由上级有返回值函数调用语句、无返回值函数调用语句输出
    return argtypes;
}

void Parser::nonvoidFunc() {
    Token type = printPop();
    if (type.getType() != intKey && type.getType() != charKey) {                    // int|char
//        error(__othererror__);
    }
    Token identifier = mustBeThisToken(name);
    mark("<声明头部>");

    /* QUAD-CODE: set label here. */
    qcodes.addCode(Quadruple(LABEL, new OperandLabel("__"+identifier.getText()+"__")));
    qcodes.now_scope_prefix = "_"+identifier.getText()+"_";
    
    std::vector<std::shared_ptr<SymbolVar> > args;
    mustBeThisToken(lBracket);
    if (data.peek().getType() != rBracket)                                          // ＜参数表＞此处禁止为空时进入，但是为空时也算＜参数表＞，要输出
        argList(args);
    mark("<参数表>");
    
    size_t prev_stack_size = table.size();
    // SAVE FUNCTION NAME AS GLOBAL IDENTIFIER IF SUCCEED, BUT NO ARGS
    
    if (!table.addFunc(identifier.getText(), type.getType()==intKey ? intFunct : charFunct, true, args))
        error(id_redef);
    else
        prev_stack_size++;
    
    /* QUAD-CODE: add "VAR declare"s, but actually they ARE A0,A1,A2,...  */
    for (auto arg: args)
        qcodes.addCode(Quadruple(PARAM, qcodes.getOperandSymbol(arg)));
    
    mustBeThisToken(rBracket);
    mustBeThisToken(lCurly);
    expected_return = (type.getType() == intKey ? intType : charType);
    codeBlock();
    mustBeThisToken(rCurly);
    mark("<有返回值函数定义>");
    
    size_t now_stack_size = table.size();
    for (int i = 0; i < now_stack_size - prev_stack_size; ++i)
        table.pop();
}

void Parser::voidFunc() {
    mustBeThisToken(voidKey);
    Token identifier = mustBeThisToken(name);
    
    /* QUAD-CODE: set label here. */
    qcodes.addCode(Quadruple(LABEL, new OperandLabel("__"+identifier.getText()+"__")));
    qcodes.now_scope_prefix = "_"+identifier.getText()+"_";
    
    std::vector<std::shared_ptr<SymbolVar> > args;
    mustBeThisToken(lBracket);
    if (data.peek().getType() != rBracket)                                          // ＜参数表＞此处禁止为空时进入，但是为空时也算＜参数表＞，要输出
        argList(args);
    mark("<参数表>");
    
    size_t prev_stack_size = table.size();
    // SAVE FUNCTION NAME AS GLOBAL IDENTIFIER IF SUCCEED, BUT NO ARGS
    
    if (!table.addFunc(identifier.getText(), voidFunct, true, args))
        error(id_redef);
    else
        prev_stack_size++;
    
    /* QUAD-CODE: add "VAR declare"s, but actually they ARE A0,A1,A2,...  */
    for (auto arg: args)
        qcodes.addCode(Quadruple(PARAM, qcodes.getOperandSymbol(arg)));
    
    mustBeThisToken(rBracket);
    mustBeThisToken(lCurly);
    expected_return = voidType;
    codeBlock();
    mustBeThisToken(rCurly);
    mark("<无返回值函数定义>");
    
    size_t now_stack_size = table.size();
    for (int i = 0; i < now_stack_size - prev_stack_size; ++i)
        table.pop();
}

void Parser::mainFunc() {
    /* QUAD-CODE: set label here. */
    qcodes.addCode(Quadruple(LABEL, new OperandLabel("__main__")));
    qcodes.now_scope_prefix = "_main_";
    
    mustBeThisToken(voidKey);
    mustBeThisToken(mainKey);
    mustBeThisToken(lBracket);
    mustBeThisToken(rBracket);
    mustBeThisToken(lCurly);
    expected_return = voidType;
    codeBlock();
    mustBeThisToken(rCurly);
    mark("<主函数>");
}

void Parser::program() {
    if (data.peek().getType() == constKey)
        constDeclare(true);
    if (data.peek(3).getType() != lBracket) {
        varDeclare(true);
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

Parser::Parser(std::set<std::pair<int, std::string> > &mess, PeekQueue<Token> data) : errorMessages(mess) {
    this->data = data;
    this->table = SymbolTable();
    this->qcodes = QuadrupleList();
    program();
    
    /* QUAD-CODE: .asciiz "\n" */
    qcodes.getQCodes()->insert(qcodes.getQCodes()->begin(), Quadruple(VAR, (Operand *)&qcodes.slashN));
    
//    for(auto qcode: *qcodes.getQCodes())
//        std::cout << qcode.toString() << std::endl;
}
