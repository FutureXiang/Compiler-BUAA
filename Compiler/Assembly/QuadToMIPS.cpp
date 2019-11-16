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
    for (int i = 1; i <= qcodes.size() && qcodes.peek(i).op != LABEL; ++i) {
        Quadruple code = qcodes.peek(i);

        if (code.op == PARAM || code.op == VAR)
            if (std::find(need_spaces.begin(), need_spaces.end(), code.target->name) == need_spaces.end())
                need_spaces.push_back(code.target->name);
        if (code.target != nullptr && code.target->isTemp())
            if (std::find(need_spaces.begin(), need_spaces.end(), code.target->name) == need_spaces.end())
                need_spaces.push_back(code.target->name);
        if (code.first != nullptr && code.first->isTemp())
            if (std::find(need_spaces.begin(), need_spaces.end(), code.first->name) == need_spaces.end())
                need_spaces.push_back(code.first->name);
        if (code.second != nullptr && code.second->isTemp())
            if (std::find(need_spaces.begin(), need_spaces.end(), code.second->name) == need_spaces.end())
                need_spaces.push_back(code.second->name);
    }
    // ARGs + VARs + TXs: count numbers
    int temp = need_spaces.size();
    addCode(format("subu", "$sp", "$sp", std::to_string(need_spaces.size() * 4)));
    
    for (auto identifier: need_spaces) {
        name2symbol[identifier] = new UniqueSymbol(identifier, (--temp)*4);
//        std::cerr << identifier << " -> " << name2symbol[identifier]->addr << std::endl;
    }
    addCode("\n");
    
    // WE CAN USE:  "lw  $x,    name2symbol[x]->addr ($sp)" TO FETCH ARGs + VARs + TXs NOW !!!!
    while (!qcodes.empty() && qcodes.peek().op != LABEL) {
        code = qcodes.pop();
//        std::cout << code.toString() << std::endl;

        if (code.op == PARAM || code.op == VAR)
            continue;
        replaceSymbolToRegs();

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
        else if (code.op == LARR)
            addCode(format("lw", code.target->name, code.first->name, code.second->toString()));
        else if (code.op == SARR) {
            if (code.target->is_instant) {
                addCode(format("li", "a0", code.target->toString()));
                addCode(format("sw", "a0", code.first->name, code.second->toString()));
            } else
                addCode(format("sw", code.target->name, code.first->name, code.second->toString()));
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
        releaseAll();
        addCode(format("addu", "$sp", "$sp", std::to_string(need_spaces.size() * 4)));
        load_regs();
        addCode(format("jr", "$ra"));
    } else {
        addCode(format("li", "$v0", "10"));
        addCode("syscall");
    }
}


void Interpreter::save_regs() {
    addCode(format("subu", "$sp", "$sp", "36"));
    addCode(LwSw('s', "$ra", "$sp", 0));
    for (int i: reg_avail)
        addCode(LwSw('s', "$"+std::to_string(i), "$sp", (i - 7)*4));
}

void Interpreter::load_regs() {
    addCode(LwSw('l', "$ra", "$sp", 0));
    for (int i: reg_avail)
        addCode(LwSw('l', "$"+std::to_string(i), "$sp", (i - 7)*4));
    addCode(format("addu", "$sp", "$sp", "36"));
}

void Interpreter::Function_Call() {
    save_regs();
    while (code.op != CALL) {
        int offset = (std::stoi(code.target->name.substr(1)) + 1) * -4;
        if (code.first->is_instant) {
            addCode(format("li", "a0", code.first->toString()));
            addCode(LwSw('s', "a0", "$sp", offset));
        } else
            addCode(LwSw('s', code.first->name, "$sp", offset));
        code = qcodes.pop();
    }
    addCode(format("jal", code.target->toString()));
    addCode("\n");
}
