//
//  Quadruple.hpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/13.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Quadruple_hpp
#define Quadruple_hpp

#include "Operand.hpp"
#include <vector>
#include <stack>


class Quadruple {
public:
    Operand *target = nullptr, *first = nullptr, *second = nullptr;
    Operator op;
public:
    Quadruple() = default;
    // LABEL;
    Quadruple(Operator opt);
    // GOTO t;
    Quadruple(Operator opt, Operand *target);
    // BEZ t, s;  BNZ t, s;  LI t, s;  MV t, s;
    Quadruple(Operator opt, Operand *target, Operand *source);
    // ADD t, f, s;  SLT t, f, s;  LARR t, s, i;
    Quadruple(Operator opt, Operand *target, Operand *first, Operand *second);

    std::string toString();
};


class QuadrupleList {
private:
    std::vector<Quadruple> qcode;
    int labelCounter;
    int tempCounter;
    std::stack<Operand *> allocedButFree;
public:
    static const std::string temp_head;
    static const std::string label_head;
    static const OperandInstant zeroInstant;
    static const OperandSymbol v0Symbol;

    QuadrupleList() = default;
    std::string allocLabel();
    Operand *allocTemp();
    void addCode(Quadruple code);
    std::vector<Quadruple> *getQCodes() {
        return &qcode;
    }
};


#endif /* Quadruple_hpp */
