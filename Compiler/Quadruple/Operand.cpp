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
    {BEZ, "BEZ"},
    {BNZ, "BNZ"},
    
    {LI, "LI"},
    {MV, "MV"},
    
    {LARR, "LARR"},
    {SARR, "SARR"},
    
    {LABEL, "LABEL"},
    
    {CALL, "CALL"},
    {RET, "RET"},
    
    {VAR, "VAR"},
};
