//
//  Quadruple.hpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/13.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Quadruple_hpp
#define Quadruple_hpp

#include "../Symbol/Symbol.hpp"
#include "Operand.hpp"
#include <vector>
#include <stack>
#include <map>


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
    // LI t, s;  MV t, s;  BEQZ l, s;
    Quadruple(Operator opt, Operand *target, Operand *source);
    // ADD t, f, s;  SLT t, f, s;  LARR t, s, i;  BEQ l, f, s;
    Quadruple(Operator opt, Operand *target, Operand *first, Operand *second);

    std::string toString() const;
    friend bool operator < (const Quadruple &x, const Quadruple &y);
};


class QuadrupleList {
private:
    std::vector<Quadruple> qcode;
    int stringCounter;
    int labelCounter;
    int tempCounter;
    std::stack<Operand *> allocedButFree;
    std::map<std::shared_ptr<Symbol>, OperandSymbol *> operandSymbolPool;
public:
    static const std::string temp_head;
    static const std::string label_head;
    static const std::string string_head;
    static const OperandInstant zeroInstant;
    static const OperandSymbol v0Symbol;
    static const OperandString slashN;

    QuadrupleList() = default;
    std::string allocStringName();
    std::string allocLabel();
    Operand *allocTemp();
    void addCode(Quadruple code);
    std::vector<Quadruple> *getQCodes() {
        return &qcode;
    }
    
    std::string now_scope_prefix = "_42global_";
    OperandSymbol *getOperandSymbol(std::shared_ptr<Symbol> symbol);
    
    std::map<std::string, std::vector<Quadruple> > get_inlineable_functions();
    void inline_functions_single(std::map<std::string, std::vector<Quadruple> > inlineable_functions);
    void inline_functions();
    void sortout_labels();
};


bool is_function_label(Quadruple q);
bool is_var(Operand *rand);
bool is_localvar(Operand *rand);
bool is_control_label(Quadruple q);
bool using_control_label(Quadruple q);

#endif /* Quadruple_hpp */
