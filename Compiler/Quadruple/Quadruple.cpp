//
//  Quadruple.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/13.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Quadruple.hpp"
#include <iostream>
#include <set>

std::string QuadrupleList::allocStringName() {
    return string_head + std::to_string(stringCounter++);
}

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
    if (code.op == SARR || (code.op >= WRITE_INT && code.op <= WRITE_STR))
        if (code.target != nullptr && code.target->isTemp())
            allocedButFree.push(code.target);
}

const std::string QuadrupleList::temp_head = "t";
const std::string QuadrupleList::label_head = "label_";
const std::string QuadrupleList::string_head = "str_";
const OperandInstant QuadrupleList::zeroInstant = OperandInstant(0);
const OperandSymbol QuadrupleList::v0Symbol = OperandSymbol("$v0");
const OperandString QuadrupleList::slashN = OperandString("\\n", "newline");


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

OperandSymbol* QuadrupleList::getOperandSymbol(std::shared_ptr<Symbol> symbol) {
    if (operandSymbolPool.count(symbol) == 0) {
        return operandSymbolPool[symbol] = new OperandSymbol(now_scope_prefix + symbol->getName());
    } else {
        return operandSymbolPool[symbol];
    }
}

void QuadrupleList::inline_functions() {
    std::map<std::string, std::vector<Quadruple> > inlineable_functions = get_inlineable_functions();
    std::vector<Quadruple> new_qcode;
    std::map<std::string, std::set<std::string> > vars_for_each_function;
    std::string scope_name = "_42global_";
    for (std::vector<Quadruple>::iterator it = qcode.begin(); it != qcode.end(); ++it) {
        if (it->op == LABEL && it->target->toString()[0]=='_')      // Only Function Labels can start with '_' !!
            scope_name = it->target->toString();
        if (inlineable_functions.count(scope_name))
            continue;
        
        if ((it->op == MV && it->target->name[0] == 'a') || it->op == CALL) {
            std::vector<Operand *> valueArgs;
            
            std::vector<Quadruple>::iterator caller = it;
            while (caller->op != CALL) {
                valueArgs.push_back(caller->first);
                caller++;
            }
            std::string function_name = caller->target->toString();
            std::vector<Quadruple>::iterator use_ret_value = caller;
            if ((caller+1) != qcode.end() && (caller+1)->first != nullptr && (caller+1)->first->toString() == "$v0")
                use_ret_value = caller + 1;
            
            if (inlineable_functions.count(function_name) == 0) { // do not inline un-inline-able callee !!
                new_qcode.push_back(*it);
                continue;
            }
            it = use_ret_value;
            
            std::vector<Quadruple> func_codes = inlineable_functions[function_name];
            const std::string inline_vars_head = "_inline_" + function_name;
            Operand *end_label = new OperandLabel(allocLabel());
            
            for (int i = 0; i < valueArgs.size(); ++i) {
                // Params must be a "PARAM" as a local VAR.
                Operand *funcArg = new OperandSymbol(inline_vars_head + func_codes[i].target->toString());
                vars_for_each_function[scope_name].insert(funcArg->toString());
                new_qcode.push_back(Quadruple(MV, funcArg, valueArgs[i]));
            }
            
            std::map<std::string, Operand*> label_mapping;
            
            for (Quadruple func_code: func_codes) {
                if (func_code.op == PARAM || func_code.op == VAR)
                    continue;
                if (func_code.op == RET) {
                    new_qcode.push_back(Quadruple(GOTO, end_label));
                    continue;
                }
                if (func_code.op == MV && func_code.target->toString() == "$v0" && use_ret_value != caller) {   // instruction after caller is using $v0 @ MV, xx, $v0
                    Quadruple replaced = func_code;
                    if (replaced.first->is_symbol && replaced.first->toString().substr(0, 3) != "_42")
                        replaced.first = new OperandSymbol(inline_vars_head + replaced.first->toString());
                    new_qcode.push_back(Quadruple(MV, use_ret_value->target, replaced.first));
                    continue;
                }
                Quadruple replaced = func_code;
                if (replaced.target != nullptr && replaced.target->is_symbol && replaced.target->toString().substr(0, 3) != "_42") {
                    replaced.target = new OperandSymbol(inline_vars_head + replaced.target->toString());
                    vars_for_each_function[scope_name].insert(replaced.target->toString());
                }
                if (replaced.first != nullptr && replaced.first->is_symbol && replaced.first->toString().substr(0, 3) != "_42") {
                    replaced.first = new OperandSymbol(inline_vars_head + replaced.first->toString());
                    vars_for_each_function[scope_name].insert(replaced.first->toString());
                }
                if (replaced.second != nullptr && replaced.second->is_symbol && replaced.second->toString().substr(0, 3) != "_42") {
                    replaced.second = new OperandSymbol(inline_vars_head + replaced.second->toString());
                    vars_for_each_function[scope_name].insert(replaced.second->toString());
                }
                if (replaced.op == LABEL || replaced.op == GOTO || (replaced.op >= BEQ && replaced.op <= BNEZ)) {
                    std::string label = replaced.target->toString();
                    if (label_mapping.count(label) == 0)
                        label_mapping[label] = new OperandLabel(allocLabel());
                    replaced.target = label_mapping[label];
                }
                new_qcode.push_back(replaced);
            }
            new_qcode.push_back(Quadruple(LABEL, end_label));
        } else {
            new_qcode.push_back(*it);
        }
    }
    scope_name = "_42global_";
    for (std::vector<Quadruple>::iterator it = new_qcode.begin(); it != new_qcode.end(); ++it) {
        if (it->op == LABEL && it->target->toString()[0]=='_')
            scope_name = it->target->toString();
        else if ((it->op != VAR && it->op != PARAM) && ((it-1)->op == VAR ||
                                   (it-1)->op == PARAM ||
                                   ((it-1)->op == LABEL && (it-1)->target->toString()[0]=='_'))
                               && vars_for_each_function.count(scope_name) != 0) {
            for (std::string var: vars_for_each_function[scope_name]) {
                it = new_qcode.insert(it, Quadruple(VAR, new OperandSymbol(var)));
                it++;
            }
        }
    }
    qcode = new_qcode;
}

std::map<std::string, std::vector<Quadruple> > QuadrupleList::get_inlineable_functions() {
    std::map<std::string, std::vector<Quadruple> > inlineable_functions;
    
    std::string scope_name = "_42global_";
    for (Quadruple q: qcode) {
        if (q.op == LABEL && q.target->toString()[0]=='_' && q.target->toString()!="__main__") {
            scope_name = q.target->toString();
            inlineable_functions[scope_name] = std::vector<Quadruple>();
        } else if (q.op == CALL) {
            std::map<std::string, std::vector<Quadruple> >::iterator find_it = inlineable_functions.find(scope_name);
            if (find_it != inlineable_functions.end())
                inlineable_functions.erase(find_it);
        } else if (inlineable_functions.count(scope_name) != 0){
            inlineable_functions[scope_name].push_back(q);
        }
    }
//    std::cerr << "---------- inlineable functions ----------" << std::endl;
//    for (std::pair<std::string, std::vector<Quadruple> > inlineable: inlineable_functions)
//        std::cerr << inlineable.first << std::endl;
    return inlineable_functions;
}

bool is_control_label(Quadruple q) {
    return q.op == LABEL && q.target->toString().substr(0, 6) == "label_";
}

bool using_control_label(Quadruple q) {
    return (q.op == GOTO || (q.op >= BEQ && q.op <= BNEZ)) && q.target->toString().substr(0, 6) == "label_";
}

void QuadrupleList::sortout_labels() {
    std::map<std::string, std::string> labelDeleted;
    std::map<std::string, int> label_using_count;
    
    // GOTO x;  GOTO x; Label x;
    for (std::vector<Quadruple>::iterator it = qcode.begin(); it != qcode.end(); ++it) {
        if (using_control_label(*it)) {
            if (label_using_count.count(it->target->toString()) == 0)
                label_using_count[it->target->toString()] = 1;
            else
                label_using_count[it->target->toString()]++;
        }
    }
    for (std::vector<Quadruple>::iterator it = qcode.begin() + 1; it != qcode.end(); ++it) {
        if (is_control_label(*it) && using_control_label(*(it-1))
            && it->target->toString() == (it-1)->target->toString()) {
            // ONLY USED BY it-1 --> delete them !
            if (label_using_count[it->target->toString()] == 1)
                it = qcode.erase(it-1, it+1) - 1;
            else
            // USED BY SEVERAL --> delete this jump !
                it = qcode.erase(it-1) - 1;
        }
    }
    
    // Label x; Label y;
    for (std::vector<Quadruple>::iterator it = qcode.begin() + 1; it != qcode.end(); ++it) {
        if (is_control_label(*it) && is_control_label(*(it-1))) {
            if (labelDeleted.count(it->target->toString()) == 0)
                labelDeleted[it->target->toString()] = (it-1)->target->toString();
            it = qcode.erase(it) - 1;
        }
    }
    for (std::vector<Quadruple>::iterator it = qcode.begin(); it != qcode.end(); ++it) {
        if (is_control_label(*it) || using_control_label(*it)) {
            if (labelDeleted.count(it->target->toString()))
                ((OperandLabel *)(it->target))->label = labelDeleted[it->target->toString()];
        }
    }
}
