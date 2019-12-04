//
//  BlocksDivide.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/12/4.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "BlocksDivide.hpp"


std::vector<std::pair<int, int> > Divider(std::vector<Quadruple> *qcodes) {
    std::vector<std::pair<int, int> > start_ends;
    int start = 0;
    for (int i = 0; i < qcodes->size(); ++i) {
        Operator op = (*qcodes)[i].op;
        if (op == LABEL || (op >= GOTO && op <= BNZ)) {
            // Label belongs to the PREVIOUS Block !!
            start_ends.push_back(std::make_pair(start, i));
            start = i + 1;
        }
    }
    start_ends.push_back(std::make_pair(start, - 1));
    return start_ends;
}
