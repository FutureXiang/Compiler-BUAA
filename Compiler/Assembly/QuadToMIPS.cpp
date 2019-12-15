//
//  QuadToMIPS.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/16.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "QuadToMIPS.hpp"

void Interpreter::run() {
    addCode(".data");
    while (qcodes.peek().op != LABEL)
        VarArrStr_Def();
    addCode("\n");
    addCode(".text");
    // ADD global regs for global arrays
    for (auto pair: name2symbol)
        if (pair.second->is_array && name2globalreg_assignment.count(pair.first))
            addCode(format("la", "$"+std::to_string(name2globalreg_assignment[pair.first]), pair.first));
    addCode(format("j", "__main__"));
    addCode("\n");
    while (!qcodes.empty()) {
        scope_name = (code = qcodes.pop()).target->toString();
        Function_Def();
        addCode("\n\n");
    }
}

const std::vector<int> Interpreter::reg_avail = {
    8,9,10,11,12,13,14,15, 24,25,
};
const std::vector<int> Interpreter::global_reg_avail = {
    16,17,18,19,20,21,22,23,
};

void Interpreter::VarArrStr_Def() {
    code = qcodes.pop();
    std::string identifier = code.target->name;
    
    if (code.first != nullptr) {                                    // VAR, array, 10
        int size = ((OperandInstant *)code.first)->value;
        name2symbol[identifier] = new UniqueSymbol(identifier, true);
        addCode(dataLabel(identifier, size).toString());
    } else if (code.target->is_string) {                            // VAR, "this is a string"
        std::string content = ((OperandString *)code.target)->value;
        name2symbol[identifier] = new UniqueSymbol(identifier);
        addCode(dataLabel(identifier, content).toString());
    } else {                                                        // VAR, aword
        name2symbol[identifier] = new UniqueSymbol(identifier);
        addCode(dataLabel(identifier).toString());
    }
}


void Interpreter::Function_Def() {
    addCode(scope_name + ":");
    
    std::vector<std::string> need_spaces = std::vector<std::string>();
    std::map<std::string, int> sp_pre_counter = std::map<std::string, int>();
    int sp_words = 0;
    for (int i = 1; i <= qcodes.size() && !(qcodes.peek(i).op == LABEL && qcodes.peek(i).target->toString()[0] == '_'); ++i) {
        Quadruple code = qcodes.peek(i);

        if (code.op == PARAM || code.op == VAR) {                   // PARAM, VAR, VAR[]
            need_spaces.push_back(code.target->name);
            int size_words = 1;
            if (code.first != nullptr) {
                size_words = ((OperandInstant *)code.first)->value;
                name2symbol[code.target->name] = new UniqueSymbol(code.target->name, true);
            } else {
                name2symbol[code.target->name] = new UniqueSymbol(code.target->name);
                if (code.op == PARAM)
                    name2symbol[code.target->name]->inited = true;      // only ARGS, they ARE initialized !!!
            }
            sp_pre_counter[code.target->name] = sp_words + size_words;
            sp_words += size_words;
        } else {                                                    // t0, t1, t2, ...
            if (code.target != nullptr && code.target->isTemp()) {
                if (sp_pre_counter.count(code.target->name) == 0) {
                    need_spaces.push_back(code.target->name);
                    sp_pre_counter[code.target->name] = (++sp_words);
                    name2symbol[code.target->name] = new UniqueSymbol(code.target->name);
                }
            }
            if (code.first != nullptr && code.first->isTemp()) {
                if (sp_pre_counter.count(code.first->name) == 0) {
                    need_spaces.push_back(code.first->name);
                    sp_pre_counter[code.first->name] = (++sp_words);
                    name2symbol[code.first->name] = new UniqueSymbol(code.first->name);
                }
            }
            if (code.second != nullptr && code.second->isTemp()) {
                if (sp_pre_counter.count(code.second->name) == 0) {
                    need_spaces.push_back(code.second->name);
                    sp_pre_counter[code.second->name] = (++sp_words);
                    name2symbol[code.second->name] = new UniqueSymbol(code.second->name);
                }
            }
        }
    }
    addCode(format("addiu", "$sp", "$sp", std::to_string(-sp_words * 4)));
//    std::cerr << "==========" << std::endl;
    for (auto identifier: need_spaces) {
        name2symbol[identifier]->addr = (sp_words - sp_pre_counter[identifier])*4;
//        std::cerr << identifier << " -> " << name2symbol[identifier]->addr << std::endl;
    }
    addCode("\n");
    
    // WE CAN USE:  "lw  $x,    name2symbol[x]->addr ($sp)" TO FETCH ARGs + VARs + TXs NOW !!!!
    while (!qcodes.empty() && !(qcodes.peek().op == LABEL && qcodes.peek().target->toString()[0] == '_')) {
        code = qcodes.pop();
//        std::cerr << code.toString() << std::endl;

        if (deadcodes_no.count(qcodes.last_poped_index())) {
//            std::cout << "Skipping dead code : " << code.toString() << std::endl;
            continue;
        }
        
        if (code.op == PARAM || code.op == VAR) {
            // Load Params / Array @ function begin
            if (name2globalreg_assignment.count(code.target->toString()))
                regForThis(code.target->toString(), true);
            continue;
        }
        replaceSymbolToRegs();  // ALWAYS AFTER qcode.pop()!!!

        if (code.op >= ADD && code.op <= DIV)
            addCode(Arith(code));
        else if (code.op >= BEQ && code.op <= BNEZ)
            addCode(CompBranch(code));
        else if (code.op == GOTO)
            addCode(format("j", code.target->toString()));
        else if (code.op == LI || code.op == MV) {
            if (code.op == MV && code.target->name[0] == 'a') {          // MV, a0, 5 ----> SAVE ARGS a_x TO -4(x+1)($sp)
                Function_Call();                // Call Functions with args
            } else
                addCode(format((code.first->is_instant) ? "li" : "move", code.target->name, code.first->toString()));
        }
        else if (code.op == CALL) {
            Function_Call();                    // Call Functions without args
        }
        else if (code.op == LARR) {
            std::string offset = code.second->toString();
            std::string base = code.first->name;
            if (!code.second->is_instant) {                     // x = arr[bvar];
                addCode(format("sll", "$a1", offset, "2"));     // $a1 = bvar * 4
                addCode(format("addu", "$a1", base, "$a1"));    // $a1 = arr + bvar * 4
                base = "$a1";
                offset = "0";
            } else
                offset = std::to_string(((OperandInstant *)code.second)->value * 4);
            addCode(format("lw", code.target->name, base, offset));
        }
        else if (code.op == SARR) {
            std::string offset = code.second->toString();
            std::string base = code.first->name;
            if (!code.second->is_instant) {                     // arr[bvar] = 5;
                addCode(format("sll", "$a1", offset, "2"));     // $a1 = bvar * 4
                addCode(format("addu", "$a1", base, "$a1"));    // $a1 = arr + bvar * 4
                base = "$a1";
                offset = "0";
            } else
                offset = std::to_string(((OperandInstant *)code.second)->value * 4);
            if (code.target->is_instant) {
                addCode(format("li", "$a0", code.target->toString()));
                addCode(format("sw", "$a0", base, offset));
            } else
                addCode(format("sw", code.target->name, base, offset));
        } else if (code.op == LABEL)
            addCode(code.target->toString() + ":");
        else if (code.op == RET) {
            if (scope_name != "__main__") {
                addCode("\n");
                releaseAllGlobals(false);
                addCode(format("addiu", "$sp", "$sp", std::to_string(sp_words * 4)));
                addCode(format("jr", "$ra"));
            } else {
                addCode(format("li", "$v0", "10"));
                addCode("syscall");
            }
        } else
            addCode(ReadWrite(code));
        
        if (isEndOfBlock(qcodes.last_poped_index())) {
            // MUST BE "j label" / "branch $x, label" / "label :"
            if (!isGoingToFuncEndOrCalling())
                releaseAll(false, -1);                      // add Regs Release BEFORE THIS CODE (branch may still need regs)
            // "jal" / "jr $ra" ALREADY handled by FunctionCall()
        }
    }
    if (scope_name != "__main__") {
        for (auto identifier: need_spaces)
            name2symbol.erase(identifier);
        addCode("\n");
        releaseAllGlobals(true);
        addCode(format("addiu", "$sp", "$sp", std::to_string(sp_words * 4)));
        addCode(format("jr", "$ra"));
    } else {
        addCode(format("li", "$v0", "10"));
        addCode("syscall");
    }
}


void Interpreter::save_regs() {
//    addCode("\n# SAVE ENVS START ----------");
    addCode(LwSw('s', "$ra", "$sp", -4));                                       // $sp -= 4 AT THE END [AFFECT sw $ra, 0($sp) -> sw $ra, -4($sp)]
//    addCode("# SAVE ENVS  END  ----------\n");
}

void Interpreter::load_regs() {
//    addCode("\n# LOAD ENVS START ----------");
    addCode(LwSw('l', "$ra", "$sp", 0));
    addCode(format("addiu", "$sp", "$sp", "4"));
//    addCode("# LOAD ENVS  END  ----------\n");
}

void Interpreter::Function_Call() {
    save_regs();
//    addCode("# SAVE ARGS START ----------");
    while (code.op != CALL) {
        int offset = (std::stoi(code.target->name.substr(1)) + 1) * -4 - 4;     // $sp -= 4 AT THE END [AFFECT sw $ai, -4i($sp) -> sw $ai, -4(i+1)($sp)]
        // 可以直接保存，由Parser::valueArgList() 保证这里没有计算了。[但是可能存在加载]
        if (code.first->is_instant) {
            addCode(format("li", "$a0", code.first->toString()));
            addCode(LwSw('s', "$a0", "$sp", offset));
        } else
            addCode(LwSw('s', code.first->name, "$sp", offset));
        code = qcodes.pop();
//        std::cerr << code.toString() << std::endl;
        replaceSymbolToRegs();  // ALWAYS AFTER qcode.pop()!!!
    }
//    addCode("# SAVE ARGS  END  ----------");
    releaseAll(true, 0);                                             // SAVING "ENVS"
    addCode(format("addiu", "$sp", "$sp", "-4"));                                 // $sp -= 4 AT THE END [AFFECT release $regs -= 4]
    addCode(format("jal", code.target->toString()));
    addCode("\n");
    load_regs();
    restoreEnv();
}

std::map<std::string, int> usageCount(std::vector<Quadruple> *const qcodes) {
    std::map<std::string, int> counter;

    for (auto qcode: *qcodes) {
        Operand *target = qcode.target;
        Operand *first = qcode.first;
        Operand *second = qcode.second;

        if (first != nullptr && is_var(first)) {
            mapAddOne(counter, first->toString());
        }
        if (second != nullptr && is_var(second)) {
            mapAddOne(counter, second->toString());
        }
        if (target != nullptr && is_var(target)) {
            mapAddOne(counter, target->toString());
        }
    }
    return counter;
}

void mapAddOne(std::map<std::string, int> &counter, std::string x) {
    if (counter.count(x))
        counter[x]++;
    else
        counter[x] = 1;
}

bool cmpByPairSecond(const std::pair<std::string, int> &l, const std::pair<std::string, int> &r) {
    return l.second > r.second;
}
