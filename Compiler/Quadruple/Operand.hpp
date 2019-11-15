//
//  Operand.hpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/12.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Operand_hpp
#define Operand_hpp

#include <map>
#include <string>

enum Operator {
    ADD, SUB, MULT, DIV,                // Arithmetic ( (Temp)Symbol / InstantNum )
    SLT, SLEQ, SGT, SGEQ, SEQ, SNE,     // Comparison ( (Temp)Symbol / InstantNum )
    GOTO, BEZ, BNZ,                     // Jump       ( Label )
    LI, MV,                             // Load       ( (Temp)Symbol / InstantNum )
    LARR, SARR,                         // L/S Array  ( Symbol, (Temp)Symbol / InstantNum )
    LABEL,                              // Set Label  ( None )
    CALL, RET,                          // Call Function, Return Function
    VAR,                                // Var / VarArray Declare
    WRITE_INT, WRITE_CHAR, WRITE_STR,   // printf: string / expr(int/char)
    READ_INT, READ_CHAR,                // scanf: variable(int/char)
};

extern std::map<Operator, std::string> op2str;


class Operand {
private:
public:
    Operand() = default;
    virtual ~Operand() {};
    virtual std::string toString() = 0;
    virtual bool isTemp() {
        return false;
    }
};


/*
 ARGUMENTS: a0, a1, a2, ..., a_inf
 RETURN:    v0
 */

class OperandSymbol: public Operand {
private:
    std::string name;
    bool is_temp;
public:
    OperandSymbol(std::string symbolName) {
        name = symbolName;
        is_temp = false;
    }
    OperandSymbol(std::string symbolName, bool temp) {
        name = symbolName;
        is_temp = temp;
    }
    std::string toString() override {
        return name;
    }
    bool isTemp() override {
        return is_temp;
    }
};


class OperandInstant: public Operand {
private:
    int value;
public:
    OperandInstant(int v) {
        value = v;
    }
    std::string toString() override {
        return std::to_string(value);
    }
};


class OperandLabel: public Operand {
private:
    std::string label;
public:
    OperandLabel(std::string l) {
        label = l;
    }
    std::string toString() override {
        return label;
    }
};

#endif /* Operand_hpp */
