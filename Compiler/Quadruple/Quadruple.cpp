//
//  Quadruple.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/13.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Quadruple.hpp"
#include <iostream>

std::string QuadrupleList::allocLabel() {
    return label_head + std::to_string(labelCounter++);
}

Operand *QuadrupleList::allocTemp() {
    if (allocedButFree.empty())
        return new OperandSymbol(temp_head + std::to_string(tempCounter++), true);
    else {
        Operand *pop = allocedButFree.top();
        allocedButFree.pop();
        return pop;
    }
}

void QuadrupleList::addCode(Quadruple code) {
    qcode.push_back(code);
    if (code.first != nullptr && code.first->isTemp())
        allocedButFree.push(code.first);
    if (code.second != nullptr && code.second->isTemp())
        allocedButFree.push(code.second);
}

const std::string QuadrupleList::temp_head = "t";
const std::string QuadrupleList::label_head = "label_";
const OperandInstant QuadrupleList::zeroInstant = OperandInstant(0);
const OperandSymbol QuadrupleList::v0Symbol = OperandSymbol("v0");


Quadruple::Quadruple(Operator opt) {
    op = opt;
}

Quadruple::Quadruple(Operator opt, Operand *target) {
    op = opt;
    this->target = target;
}

Quadruple::Quadruple(Operator opt, Operand *target, Operand *source) {
    op = opt;
    this->target = target;
    this->first = source;
}

Quadruple::Quadruple(Operator opt, Operand *target, Operand *first, Operand *second) {
    op = opt;
    this->target = target;
    this->first = first;
    this->second = second;
}

std::string Quadruple::toString() {
    std::string str = op2str[op];
    if (target != nullptr)
        str.append(", " + target->toString());
    if (first != nullptr)
        str.append(", " + first->toString());
    if (second != nullptr)
        str.append(", " + second->toString());
    return str;
}
