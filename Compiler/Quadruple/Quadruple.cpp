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

std::string Quadruple::toString() const {
    std::string str = op2str[op];
    if (target != nullptr)
        str.append(", " + target->toString());
    if (first != nullptr)
        str.append(", " + first->toString());
    if (second != nullptr)
        str.append(", " + second->toString());
    return str;
}

bool operator < (const Quadruple &x, const Quadruple &y) {
    return x.toString() < y.toString();
}

OperandSymbol* QuadrupleList::getOperandSymbol(std::shared_ptr<Symbol> symbol) {
    if (operandSymbolPool.count(symbol) == 0) {
        return operandSymbolPool[symbol] = new OperandSymbol(now_scope_prefix + symbol->getName());
    } else {
        return operandSymbolPool[symbol];
    }
}

bool is_function_label(Quadruple q) {
    return q.op == LABEL && q.target->toString().substr(0, 2) == "__";
}

bool is_var(Operand *rand) {
    if (!rand->is_symbol)
        return false;
    std::string s = rand->toString();
    return s[0] != 'a' && s[0] != '$';
}

bool is_localvar(Operand *rand) {
    if (!rand->is_symbol)
        return false;
    std::string s = rand->toString();
    return s[0] != 'a' && s[0] != '$' && s.substr(0, 3) != "_42";
}

bool is_globalvar(Operand *rand) {
    if (!rand->is_symbol)
        return false;
    std::string s = rand->toString();
    return s.substr(0, 3) == "_42";
}

void QuadrupleList::inline_functions() {
    std::map<std::string, std::vector<Quadruple> > inlineable_functions;
    while ((inlineable_functions = get_inlineable_functions()).size() != 0) {
        inline_functions_single(inlineable_functions);
//        for (Quadruple q: qcode)
//            std::cerr << q.toString() << std::endl;
    }
}

void QuadrupleList::inline_functions_single(std::map<std::string, std::vector<Quadruple> > inlineable_functions) {
    std::vector<Quadruple> new_qcode;
    std::map<std::string, std::set<Quadruple> > vars_for_each_function;
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
            
            std::set<std::string> params_only_refed;
            std::map<std::string, Operand*> params_source_args;
            
            auto args_copy_to_params_start = new_qcode.size();
            for (int i = 0; i < valueArgs.size(); ++i) {
                // Params must be a "PARAM" as a local VAR.
                Operand *funcArg = new OperandSymbol(inline_vars_head + func_codes[i].target->toString());
                new_qcode.push_back(Quadruple(MV, funcArg, valueArgs[i]));
                params_only_refed.insert(funcArg->toString());
                params_source_args[funcArg->toString()] = valueArgs[i];
            }
            
            std::map<std::string, Operand*> label_mapping;
            
            for (Quadruple func_code: func_codes) {
                if (func_code.op == PARAM || func_code.op == VAR) {
                    Quadruple replaced = func_code;
                    replaced.op = VAR;
                    replaced.target = new OperandSymbol(inline_vars_head + replaced.target->toString());
                    vars_for_each_function[scope_name].insert(replaced);
                    continue;
                }
                if (func_code.op == RET) {
                    new_qcode.push_back(Quadruple(GOTO, end_label));
                    continue;
                }
                Quadruple replaced = func_code;
                if (replaced.target != nullptr && is_localvar(replaced.target)) {
                    replaced.target = new OperandSymbol(inline_vars_head + replaced.target->toString());
                    if (func_code.target->isTemp())
                        vars_for_each_function[scope_name].insert(Quadruple(VAR, replaced.target));
                    if (params_only_refed.count(replaced.target->toString()) && modify_target_operators.count(replaced.op))
                        params_only_refed.erase(replaced.target->toString());
                }
                if (replaced.first != nullptr && is_localvar(replaced.first)) {
                    replaced.first = new OperandSymbol(inline_vars_head + replaced.first->toString());
                    if (func_code.first->isTemp())
                        vars_for_each_function[scope_name].insert(Quadruple(VAR, replaced.first));
                }
                if (replaced.second != nullptr && is_localvar(replaced.second)) {
                    replaced.second = new OperandSymbol(inline_vars_head + replaced.second->toString());
                    if (func_code.second->isTemp())
                        vars_for_each_function[scope_name].insert(Quadruple(VAR, replaced.second));
                }
                if (replaced.op == LABEL || replaced.op == GOTO || (replaced.op >= BEQ && replaced.op <= BNEZ)) {
                    std::string label = replaced.target->toString();
                    if (label_mapping.count(label) == 0)
                        label_mapping[label] = new OperandLabel(allocLabel());
                    replaced.target = label_mapping[label];
                } else if (replaced.target->toString() == "$v0" && use_ret_value != caller) {           // {MV $v0, t0 / ADDU $v0, t0, t1} + MV ans, $v0
                    replaced.target = use_ret_value->target;
                }
                new_qcode.push_back(replaced);
            }
            new_qcode.push_back(Quadruple(LABEL, end_label));
            
            auto it = new_qcode.begin() + args_copy_to_params_start;
            int checked = 0;
            while (checked < valueArgs.size()) {
                checked++;
                if (params_only_refed.count(it->target->toString())) {
//                    std::cerr << "removed: " << it->toString() << std::endl;
                    it = new_qcode.erase(it);
                } else
                    it++;
            }
            
            for (; it != new_qcode.end(); ++it) {
                if (it->target != nullptr && params_only_refed.count(it->target->toString()))
                    it->target = params_source_args[it->target->toString()];
                if (it->first != nullptr && params_only_refed.count(it->first->toString()))
                    it->first = params_source_args[it->first->toString()];
                if (it->second != nullptr && params_only_refed.count(it->second->toString()))
                    it->second = params_source_args[it->second->toString()];
            }
            
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
            for (Quadruple var: vars_for_each_function[scope_name]) {
                it = new_qcode.insert(it, var);
                it++;
            }
        }
    }
    qcode = new_qcode;
}

std::map<std::string, std::vector<Quadruple> > QuadrupleList::get_inlineable_functions() {
    std::map<std::string, std::vector<Quadruple> > inlineable_functions;
    std::map<std::string, std::set<std::string> > call_relations;
    std::vector<std::string> functions;
    
    std::string scope_name = "_42global_";
    for (Quadruple q: qcode) {
        if (is_function_label(q)) {
            scope_name = q.target->toString();
            call_relations[scope_name] = std::set<std::string>();
            functions.push_back(scope_name);
        } else if (q.op == CALL) {
            call_relations[scope_name].insert(q.target->toString());
        }
    }
    for (std::string caller: functions) {
        unsigned long out_degree = call_relations[caller].size();
        for (std::string callee: call_relations[caller]) {
            if (call_relations[callee].count(callee) && caller != callee)
                out_degree--;
        }
        if (out_degree == 0 && caller != "__main__") { // Degree_out == 0 (except for calling recursive functions)
            inlineable_functions[caller] = std::vector<Quadruple>();
        }
    }
    
    scope_name = "_42global_";
    for (Quadruple q: qcode) {
        if (is_function_label(q)) {
            scope_name = q.target->toString();
        } else if (inlineable_functions.count(scope_name)) {        // ignore the "Label __func__"
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
    reduce();
    
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
        // Label without reference
        if (is_control_label(*it) && label_using_count.count(it->target->toString()) == 0) {
            it = qcode.erase(it) - 1;
        }
    }
}

void QuadrupleList::reduce() {
    for (std::vector<Quadruple>::iterator it = qcode.begin() + 1; it != qcode.end(); ++it) {
        Quadruple code = *it;
        if (code.op >= BEQ && code.op <= BLE) {
            if (code.first->is_instant && code.second->is_instant) {
                int firstValue = ((OperandInstant *)code.first)->value;
                int secondValue = ((OperandInstant *)code.second)->value;
                bool True;
                switch (code.op) {
                   case BLT:
                       True = firstValue < secondValue;
                       break;
                   case BLE:
                       True = firstValue <= secondValue;
                       break;
                   case BGT:
                       True = firstValue > secondValue;
                       break;
                   case BGE:
                       True = firstValue >= secondValue;
                       break;
                   case BNE:
                       True = firstValue != secondValue;
                       break;
                   case BEQ:
                       True = firstValue == secondValue;
                       break;
                   default:
                       True = false;
                       break;
                }
                if (True) {
                    it->op = GOTO;
                    it->first = nullptr;
                    it->second = nullptr;
                } else
                    it = qcode.erase(it) - 1;
            }
        }
    }
}
