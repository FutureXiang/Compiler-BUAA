//
//  QuadToMIPS.hpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/16.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef QuadToMIPS_hpp
#define QuadToMIPS_hpp

#include "../Quadruple/Quadruple.hpp"
#include "MIPS.hpp"
#include "../PeekQueue.cpp"
#include <iostream>
#include <queue>
#include <set>

// (OperandSymbol)      _global_x:   use:    la $t, _global_x;  lw $x, 0($t)
// (OperandSymbol)      _main_x:     use:    lw $x, addr($sp)
// (OperandSymbolTemp)  t

class UniqueSymbol {
public:
    std::string name;
    int addr = -1;      // -1 for global, non-negative for local ($sp + addr)
    bool is_array;
    bool inited = false;        // only ARGS, they ARE initialized !!!
    UniqueSymbol(std::string n, bool array) {
        name = n;
        is_array = array;
    }
    UniqueSymbol(std::string n) {
        name = n;
        is_array = false;
    }
};


class Interpreter {
    PeekQueue<Quadruple> qcodes = PeekQueue<Quadruple>();
    std::vector<std::string> mcodes;
    
    std::map<std::string, UniqueSymbol*> name2symbol;   // string name -> UniqueSymbol *symbol -> int addr

    std::map<UniqueSymbol*, int> symbol2reg;            // UniqueSymbol *symbol -> int reg_using
    std::vector<int> reg_free, reg_used;
    const std::vector<int> reg_avail{8,9,10,11,12,13,14,15};
    std::string scope_name;
    Quadruple code;                                     // ONGOING qcode

public:
    Interpreter(std::vector<Quadruple> *qs) {
        for (Quadruple qcode: *qs)
            qcodes.add(qcode);
        reg_free = reg_avail;
        run();
    }
    
    void VarArrStr_Def();
    void Function_Def();
    void run();
    void Function_Call();
    
    void addCode(std::string code) {
        mcodes.push_back(code);
        std::cout << code << std::endl;
    }
    
    void save_regs();   // CALL-ER SAVE REGS
    void load_regs();   // CALL-EE LOAD REGS
    
    void replaceSymbolToRegs() {
        Operand *target = code.target, *first = code.first, *second = code.second;
        if (code.target != nullptr && name2symbol.count(code.target->toString())) {
            target = new OperandSymbol("$"+std::to_string(regForThis(code.target->name)));
        }
        if (code.first != nullptr && name2symbol.count(code.first->toString())) {
            first = new OperandSymbol("$"+std::to_string(regForThis(code.first->name)));
        }
        if (code.second != nullptr && name2symbol.count(code.second->toString())) {
            second = new OperandSymbol("$"+std::to_string(regForThis(code.second->name)));
        }
        code = Quadruple(code.op, target, first, second);
    }
    
    int regForThis(std::string name) {
        UniqueSymbol* symbol = name2symbol[name];
        if (symbol2reg.count(symbol) == 0) {
            symbol2reg[symbol] = allocReg(symbol);
            if (!symbol->is_array) {
                // variable -> $x = value   [LOAD var value from .DATA / STACK]
                if (symbol->inited == false)            // Didn't have meaningful value yet, don't need to load it.
                    return symbol2reg[symbol];
                if (symbol->addr == -1) {
                    addCode(format("la", "$a0", symbol->name));
                    addCode(LwSw('l', "$"+std::to_string(symbol2reg[symbol]), "$a0", 0));
                } else
                    addCode(LwSw('l', "$"+std::to_string(symbol2reg[symbol]), "$sp", symbol->addr));
            } else {
                // array -> $x = addr       [LOAD arr addr from .DATA / STACK]
                if (symbol->addr == -1) {
                    addCode(format("la", "$"+std::to_string(symbol2reg[symbol]), symbol->name));
                } else
                    addCode(format("addu", "$"+std::to_string(symbol2reg[symbol]), "$sp", std::to_string(symbol->addr)));
                
            }
        }
        return symbol2reg[symbol];
    }
    int allocReg(UniqueSymbol* symbol) {
        if (!reg_free.empty()) {
            int r = reg_free.front();
            reg_free.erase(reg_free.begin());
            reg_used.push_back(r);
            return r;
        } else {
            for (auto it = reg_used.begin(); it != reg_used.end(); ++it) {
                bool check = true;
                if (code.first != nullptr) {
                    UniqueSymbol *first = name2symbol[code.first->name];
                    if (symbol2reg.count(first) != 0 && symbol2reg[first] == *it)
                        check = false;
                }
                if (code.second != nullptr) {
                    UniqueSymbol *second = name2symbol[code.second->name];
                    if (symbol2reg.count(second) != 0 && symbol2reg[second] == *it)
                        check = false;
                }
                if (check) {
                    int r = *it;
                    reg_used.erase(it);     // after ERASE, *it will CHANGE (to next element) !!!
                    releaseReg(r);
                    reg_used.push_back(r);
                    return r;
                }
            }
        }
        return 0;
    }
    void releaseReg(int reg) {
        for (auto it = symbol2reg.begin(); it != symbol2reg.end(); ++it) {
            UniqueSymbol *symbol = it->first;
            if (it->second == reg) {
                if (!symbol->is_array) {
                    // variable -> value = $x   [SAVE var value to .DATA / STACK]
                    if (symbol->addr == -1) {
                        addCode(format("la", "$a0", symbol->name));
                        addCode(LwSw('s', "$"+std::to_string(reg), "$a0", 0));
                    } else
                        addCode(LwSw('s', "$"+std::to_string(reg), "$sp", symbol->addr));
                    symbol->inited = true;
                }
                symbol2reg.erase(it);
                break;
            }
        }
    }
    void releaseAllGlobals() {
        // Locals are not saved back. We don't need them anymore.
        for (auto pair : symbol2reg) {
            int reg = pair.second;
            UniqueSymbol *symbol = pair.first;
            if (symbol->addr == -1) {
                addCode(format("la", "$a0", symbol->name));
                addCode(LwSw('s', "$"+std::to_string(reg), "$a0", 0));
            }
            symbol->inited = true;
        }
        symbol2reg.clear();
        reg_free = reg_avail;
        reg_used = std::vector<int>();
    }
    void releaseAll() {
        addCode("\n# RELEASE REGS START ----------");
        for (auto pair : symbol2reg) {
            int reg = pair.second;
            UniqueSymbol *symbol = pair.first;
            if (!symbol->is_array) {
                // variable -> value = $x   [SAVE var value to .DATA / STACK]
                if (symbol->addr == -1) {
                    addCode(format("la", "$a0", symbol->name));
                    addCode(LwSw('s', "$"+std::to_string(reg), "$a0", 0));
                } else
                    addCode(LwSw('s', "$"+std::to_string(reg), "$sp", symbol->addr + 4)); // $sp has EARLY -4ed for saving $ra !!!
                symbol->inited = true;
            }
        }
        addCode("\n# RELEASE REGS  END  ----------");
        symbol2reg.clear();
        reg_free = reg_avail;
        reg_used = std::vector<int>();
    }
};


#endif /* QuadToMIPS_hpp */
