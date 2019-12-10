//
//  Operand.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/13.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Operand.hpp"

std::map<Operator, std::string> op2str = {
    {ADD, "ADD"},
    {SUB, "SUB"},
    {MULT, "MULT"},
    {DIV, "DIV"},
    {SLT, "SLT"},
    {SLEQ, "SLEQ"},
    {SGT, "SGT"},
    {SGEQ, "SGEQ"},
    {SEQ, "SEQ"},
    {SNE, "SNE"},
    
    {GOTO, "GOTO"},
    
    {LI, "LI"},
    {MV, "MV"},
    
    {LARR, "LARR"},
    {SARR, "SARR"},
    
    {LABEL, "LABEL"},
    
    {CALL, "CALL"},
    {RET, "RET"},
    
    {VAR, "VAR"},
    {PARAM, "PARAM"},
    
    {READ_INT, "READ_INT"},
    {READ_CHAR, "READ_CHAR"},
    {WRITE_INT, "WRITE_INT"},
    {WRITE_CHAR, "WRITE_CHAR"},
    {WRITE_STR, "WRITE_STR"},
    
    {BEQ, "BEQ"},
    {BNE, "BNE"},
    {BGT, "BGT"},
    {BGE, "BGE"},
    {BLT, "BLT"},
    {BLE, "BLE"},
    {BEQZ, "BEQZ"},
    {BNEZ, "BNEZ"},
};
