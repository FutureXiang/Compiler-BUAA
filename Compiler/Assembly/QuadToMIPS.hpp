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
#include "../Quadruple/BlocksDivide.hpp"
#include <algorithm>

// (OperandSymbol)      _global_x:   use:    la $t, _global_x;  lw $x, 0($t)
// (OperandSymbol)      _main_x:     use:    lw $x, addr($sp)
// (OperandSymbolTemp)  t

class UniqueSymbol {
public:
    std::string name;
    int addr = -1;      // -1 for global, non-negative for local ($sp + addr)
    bool is_array;
    bool inited = false;        // only ARGS, they ARE initialized !!!
    bool dirty = false;         // Has it been modified since last lw?
    UniqueSymbol(std::string n, bool array) {
        name = n;
        is_array = array;
    }
    UniqueSymbol(std::string n) {
        name = n;
        is_array = false;
    }
};

std::map<std::string, int> usageCount(std::vector<Quadruple> *const qcodes);
void mapAddOne(std::map<std::string, int> &counter, std::string x);
bool cmpByPairSecond(const std::pair<std::string, int> &l, const std::pair<std::string, int> &r);

class Interpreter {
    PeekQueue<Quadruple> qcodes = PeekQueue<Quadruple>();
    std::vector<CodeBlock *> blocks;
    std::map<int, CodeBlock *> end2block;
    std::vector<std::string> mcodes;
    
    std::map<std::string, UniqueSymbol*> name2symbol;   // string name -> UniqueSymbol *symbol -> int addr

    std::map<UniqueSymbol*, int> symbol2reg;            // UniqueSymbol *symbol -> int reg_using
    std::map<UniqueSymbol*, int> localsymbol2globalreg; // UniqueSymbol *symbol -> int reg_using
    std::map<std::string, int> name2globalreg_assignment;
    std::vector<int> reg_free, reg_used;
    static const std::vector<int> reg_avail;
    static const std::vector<int> global_reg_avail;
    std::string scope_name;
    Quadruple code;                                     // ONGOING qcode
    std::set<int> deadcodes_no;
    
    std::map<UniqueSymbol*, int> envs;
    
public:
    Interpreter(std::vector<Quadruple> *qs) {
        blocks = Divider(qs);
        deadcodes_no = deadCodeElimination(qs, blocks);
        for (auto block: blocks) {
            end2block[block->end] = block;
        }
        for (Quadruple qcode: *qs)
            qcodes.add(qcode);
        reg_free = reg_avail;
        global_regs(qs);
        run();
    }
    
    void VarArrStr_Def();
    void Function_Def();
    void run();
    void Function_Call();
    
    void addCode(std::string code) {
        mcodes.push_back(code);
    }
    void addCode(std::string code, int offset) {
        mcodes.insert(mcodes.end()+offset, code);
    }
    std::vector<std::string> getMIPS() {
        return mcodes;
    }
    
    void save_regs();   // CALL-ER SAVE REGS
    void load_regs();   // CALL-EE LOAD REGS
    
    void replaceSymbolToRegs() {
        Operand *target = code.target, *first = code.first, *second = code.second;
        if (code.first != nullptr && name2symbol.count(code.first->toString())) {
            first = new OperandSymbol("$"+std::to_string(regForThis(code.first->name, true)));
        }
        if (code.second != nullptr && name2symbol.count(code.second->toString())) {
            second = new OperandSymbol("$"+std::to_string(regForThis(code.second->name, true)));
        }
        // FIRST ref(first, second) THEN assign(target)
        if (code.target != nullptr && name2symbol.count(code.target->toString())) {
            if (modify_target_operators.count(code.op))
                name2symbol[code.target->toString()]->dirty = true;
            // except for sw, other $target
            target = new OperandSymbol("$"+std::to_string(regForThis(code.target->name, (ref_target_operators.count(code.op) ? true : false))));
        }
        code = Quadruple(code.op, target, first, second);
    }
    
    int regForThis(std::string name, bool refer) {
        UniqueSymbol* symbol = name2symbol[name];
        
        // $s_x
        if (name2globalreg_assignment.count(name)) {
            if (symbol->addr == -1)
                return name2globalreg_assignment[name];

            if (localsymbol2globalreg.count(symbol) == 0) {
                // INITIALIZE @ first use / RESTORE @ jal_next
                localsymbol2globalreg[symbol] = name2globalreg_assignment[name];
                if (!symbol->is_array) {
                    if (symbol->inited && refer)
                        addCode(LwSw('l', "$"+std::to_string(localsymbol2globalreg[symbol]), "$sp", symbol->addr));
                } else
                    addCode(format("addiu", "$"+std::to_string(localsymbol2globalreg[symbol]), "$sp", std::to_string(symbol->addr)));
            }
            return name2globalreg_assignment[name];
        }
        
        // $t_x
        if (symbol2reg.count(symbol) == 0) {
            symbol2reg[symbol] = allocReg(symbol);
//            std::cout << symbol->name << " is_array:" << symbol->is_array << " addr:" << symbol->addr << std::endl;
            if (!symbol->is_array) {
                // variable -> $x = value   [LOAD var value from .DATA / STACK]
                if ((symbol->addr != -1 && symbol->inited == false) || (!refer))   // [MUST BE LOCAL VARS, NOT GLOBAL !!!] Didn't have meaningful value yet, don't need to load it.
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
                    addCode(format("addiu", "$"+std::to_string(symbol2reg[symbol]), "$sp", std::to_string(symbol->addr)));
                
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
                    symbol->dirty = false;
                }
                symbol2reg.erase(it);
                break;
            }
        }
    }
    void releaseAllGlobals(bool empty_container) {
        // @ RET: only print "sw", don't empty symbol2reg
        // @ END: print "sw" + empty symbol2reg

        // Locals are not saved back. We don't need them anymore.
//        addCode("\n# RELEASE GLOBAL REGS START ----------");
        for (auto pair : symbol2reg) {
            int reg = pair.second;
            UniqueSymbol *symbol = pair.first;
            if (!symbol->is_array) {
                if (symbol->addr == -1) {
                    addCode(format("la", "$a0", symbol->name));
                    addCode(LwSw('s', "$"+std::to_string(reg), "$a0", 0));
                }
                if (empty_container) {
                    symbol->inited = true;
                    symbol->dirty = false;
                }
            }
        }
//        addCode("# RELEASE GLOBAL REGS  END  ----------\n");
        if (empty_container) {
            // CLEAR localsymbol2globalreg & symbol2reg
            localsymbol2globalreg.clear();
            symbol2reg.clear();
            reg_free = reg_avail;
            reg_used = std::vector<int>();
        }
    }
    void releaseAll(bool is_caller_saving_env, int offset) {
//        addCode("\n# RELEASE REGS START ----------"+code.toString(), offset);
        // ONLY RELEASE $s0 [for local vars] @ jal (is_caller_saving_env == true)
        if (is_caller_saving_env) {
            envs.clear();
            for (auto pair: localsymbol2globalreg) {
                int reg = pair.second;
                UniqueSymbol *symbol = pair.first;
                envs[symbol] = reg;
                if (!symbol->is_array && symbol->dirty)
                    addCode(LwSw('s', "$"+std::to_string(reg), "$sp", symbol->addr), offset);
                symbol->inited = true;
                symbol->dirty = false;
            }
            localsymbol2globalreg.clear();
        }

        for (auto pair : symbol2reg) {
            int reg = pair.second;
            UniqueSymbol *symbol = pair.first;
            if (!symbol->is_array) {
                // variable -> value = $x   [SAVE var value to .DATA / STACK]
                if (symbol->addr == -1) {
                    addCode(format("la", "$a0", symbol->name), offset);
                    addCode(LwSw('s', "$"+std::to_string(reg), "$a0", 0), offset);
                } else if (symbol->dirty) {
                    // [INLINE --> SIDE EFFECT] NEED TO SAVE EVERYTHING DIRTY AT THE END OF BLOCKs
                    if (is_caller_saving_env)                                                       // Before CALL: inside a block
                        addCode(LwSw('s', "$"+std::to_string(reg), "$sp", symbol->addr), offset);
                    else if (end2block[qcodes.last_poped_index()]->out.count(symbol->name))         // At the block END: Live Variables Analysis --- only save "OUT"s of the block
                        addCode(LwSw('s', "$"+std::to_string(reg), "$sp", symbol->addr), offset);
                }
                symbol->inited = true;
                symbol->dirty = false;
            }
        }
//        addCode("# RELEASE REGS  END  ----------\n", offset);
        symbol2reg.clear();
        reg_free = reg_avail;
        reg_used = std::vector<int>();
    }
    void restoreEnv() {
        for (auto sym_reg: envs) {
            localsymbol2globalreg[sym_reg.first] = sym_reg.second;
            if (!sym_reg.first->is_array) {
                addCode(LwSw('l', "$"+std::to_string(sym_reg.second), "$sp", sym_reg.first->addr));
            } else
                addCode(format("addiu", "$"+std::to_string(sym_reg.second), "$sp", std::to_string(sym_reg.first->addr)));
        }
        envs.clear();
    }
    bool isEndOfBlock(int index) {
        // LABEL, GOTO, Branch
        return end2block.count(index) != 0;
    }
    bool isGoingToFuncEndOrCalling() {
        return code.op == RET || code.op == CALL;
    }
    
    void global_regs(std::vector<Quadruple> *qcodes) {
        std::map<std::string, int> usage = usageCount(qcodes);
        
        std::vector<std::pair<std::string, int> > usage_vector;
        for (auto pair: usage) usage_vector.push_back(pair);
        std::sort(usage_vector.begin(), usage_vector.end(), cmpByPairSecond);
        for (int i = 0; i < std::min(global_reg_avail.size(), usage_vector.size()); ++i)
            name2globalreg_assignment[usage_vector[i].first] = global_reg_avail[i];
        
        bool OUTPUT_count = false;
        if (OUTPUT_count) {
            for (auto pair: usage)
                std::cerr << pair.first << " :: " << pair.second << std::endl;
            for (auto pair: name2globalreg_assignment)
                std::cerr << pair.first << " --> " << pair.second << std::endl;
        }
    }
};


#endif /* QuadToMIPS_hpp */
