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
    UniqueSymbol(std::string n, int a) {
        name = n;
        addr = a;
    }
};


class Interpreter {
    PeekQueue<Quadruple> qcodes = PeekQueue<Quadruple>();
    std::vector<std::string> mcodes;
    std::map<std::string, UniqueSymbol*> name2symbol;
    std::map<UniqueSymbol*, int> symbol2reg;
    std::queue<int> reg_free, reg_used;
    std::string scope_name;

public:
    Interpreter(std::vector<Quadruple> *qs) {
        for (Quadruple qcode: *qs)
            qcodes.add(qcode);
        run();
        for (int i = 8; i <= 15; ++i)
            reg_free.push(i);
    }
    
    void VarArrStr_Def();
    void Function_Def();
    void run();
    void Function_Call(Quadruple poped_code);
    
    void addCode(std::string code) {
        mcodes.push_back(code);
        std::cerr << code << std::endl;
    }
    
    void save_regs();   // CALL-ER SAVE REGS
    void load_regs();   // CALL-EE LOAD REGS
    
    int allocReg() {
        if (!reg_free.empty()) {
            int r = reg_free.front();
            reg_free.pop();
            reg_used.push(r);
            return r;
        } else {
            int r = reg_used.front();
            reg_used.pop();
            reg_used.push(r);
            return r;
        }
    }
};


#endif /* QuadToMIPS_hpp */
