//
//  QuadToMIPS.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/16.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "QuadToMIPS.hpp"
#include <algorithm>

void Interpreter::run() {
    addCode(".data");
    while (qcodes.peek().op != LABEL)
        VarArrStr_Def();
    addCode("\n");
    addCode(".text");
    addCode(format("j", "__main__"));
    addCode("\n");
    while (!qcodes.empty()) {
        scope_name = (code = qcodes.pop()).target->toString();
        Function_Def();
        addCode("\n\n");
    }
}


void Interpreter::VarArrStr_Def() {
    code = qcodes.pop();
    std::string identifier = code.target->name;
    name2symbol[identifier] = new UniqueSymbol(identifier, -1);
    
    if (code.first != nullptr) {                                    // VAR, array, 10
        int size = ((OperandInstant *)code.first)->value;
        addCode(dataLabel(identifier, size).toString());
    } else if (code.target->is_string) {                            // VAR, "this is a string"
        std::string content = ((OperandString *)code.target)->value;
        addCode(dataLabel(identifier, content).toString());
    } else {                                                        // VAR, aword
        addCode(dataLabel(identifier).toString());
    }
}


void Interpreter::Function_Def() {
    addCode(scope_name + ":");
    
    std::vector<std::string> need_spaces = std::vector<std::string>();
    std::map<std::string, int> sp_pre_counter = std::map<std::string, int>();
    int sp_words = 0;
    for (int i = 1; i <= qcodes.size() && qcodes.peek(i).op != LABEL; ++i) {
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
    addCode(format("subu", "$sp", "$sp", std::to_string(sp_words * 4)));
    std::cerr << "==========" << std::endl;
    for (auto identifier: need_spaces) {
        name2symbol[identifier]->addr = (sp_words - sp_pre_counter[identifier])*4;
        std::cerr << identifier << " -> " << name2symbol[identifier]->addr << std::endl;
    }
    addCode("\n");
    
    // WE CAN USE:  "lw  $x,    name2symbol[x]->addr ($sp)" TO FETCH ARGs + VARs + TXs NOW !!!!
    while (!qcodes.empty() && qcodes.peek().op != LABEL) {
        code = qcodes.pop();
//        std::cerr << code.toString() << std::endl;

        if (code.op == PARAM || code.op == VAR)
            continue;
        replaceSymbolToRegs();  // ALWAYS AFTER qcode.pop()!!!

        if (code.op >= ADD && code.op <= SNE)
            addCode(ArithComp(code));
        else if (code.op == GOTO)
            addCode(format("j", code.target->toString()));
        else if (code.op == BEZ || code.op == BNZ)
            addCode(format((code.op == BEZ) ? "beqz" : "bnez", code.target->toString()));
        else if (code.op == LI || code.op == MV) {
            if (code.op == MV && code.target->name[0] == 'a') {          // MV, a0, 5 ----> SAVE ARGS a_x TO -4(x+1)($sp)
                Function_Call();
            } else
                addCode(format((code.first->is_instant) ? "li" : "move", code.target->name, code.first->toString()));
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
            addCode(format("j", scope_name+"END"));
        } else
            addCode(ReadWrite(code));
    }
    if (scope_name != "__main__") {
        for (auto identifier: need_spaces)
            name2symbol.erase(identifier);
        addCode("\n");
        addCode(scope_name+"END:");
        releaseAllGlobals();
        addCode(format("addu", "$sp", "$sp", std::to_string(sp_words * 4)));
        addCode(format("jr", "$ra"));
    } else {
        addCode(format("li", "$v0", "10"));
        addCode("syscall");
    }
}


void Interpreter::save_regs() {
    addCode("\n# SAVE ENVS START ----------");
    addCode(format("subu", "$sp", "$sp", "4"));
    addCode(LwSw('s', "$ra", "$sp", 0));
    addCode("# SAVE ENVS  END  ----------\n");
}

void Interpreter::load_regs() {
    addCode("\n# LOAD ENVS START ----------");
    addCode(LwSw('l', "$ra", "$sp", 0));
    addCode(format("addu", "$sp", "$sp", "4"));
    addCode("# LOAD ENVS  END  ----------\n");
}

void Interpreter::Function_Call() {
    save_regs();
    addCode("# SAVE ARGS START ----------");
    while (code.op != CALL) {
        int offset = (std::stoi(code.target->name.substr(1)) + 1) * -4;
        // 可以直接保存，由Parser::valueArgList() 保证这里没有计算了。
        if (code.first->is_instant) {
            addCode(format("li", "$a0", code.first->toString()));
            addCode(LwSw('s', "$a0", "$sp", offset));
        } else
            addCode(LwSw('s', code.first->name, "$sp", offset));
        code = qcodes.pop();
//        std::cerr << code.toString() << std::endl;
        replaceSymbolToRegs();  // ALWAYS AFTER qcode.pop()!!!
    }
    addCode("# SAVE ARGS  END  ----------");
    releaseAll();
    addCode(format("jal", code.target->toString()));
    addCode("\n");
    load_regs();
}
