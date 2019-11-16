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
    VAR, PARAM,                         // Var / VarArray Declare
    WRITE_INT, WRITE_CHAR, WRITE_STR,   // printf: string / expr(int/char)
    READ_INT, READ_CHAR,                // scanf: variable(int/char)
};

extern std::map<Operator, std::string> op2str;


class Operand {
public:
    std::string name;
    bool is_instant = false, is_string = false;
    
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
public:
    bool is_temp;

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
public:
    int value;

    OperandInstant(int v) {
        value = v;
        is_instant = true;
    }
    std::string toString() override {
        return std::to_string(value);
    }
};


class OperandString: public Operand {
public:
    std::string value;

    OperandString(std::string v, std::string n) {
        value = v;
        name = n;
        is_string = true;
    }
    std::string toString() override {
        return "\"" + value + "\"" + " (" + name + ")";
    }
};


class OperandLabel: public Operand {
public:
    std::string label;

    OperandLabel(std::string l) {
        label = l;
    }
    std::string toString() override {
        return label;
    }
};

#endif /* Operand_hpp */
