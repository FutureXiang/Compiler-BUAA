//
//  MIPS.hpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/16.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef MIPS_hpp
#define MIPS_hpp

#include <string>
#include "../Quadruple/Quadruple.hpp"


enum DataType {
    word, space, asciiz,
};


class dataLabel {
    // int/char, int/char[], string
public:
    std::string label;
    DataType type;
    
    int arrayShape;
    std::string stringContent;
    
    dataLabel() = default;
    ~dataLabel() = default;
    
    dataLabel(std::string name);                    // int/char
    dataLabel(std::string name, int size);          // int/char[]
    dataLabel(std::string name, std::string quote); // string
    
    std::string toString();
};


std::string Arith(Quadruple qcode);
std::string CompBranch(Quadruple qcode);

std::string ReadWrite(Quadruple qcode);

std::string LwSw(char mode, std::string target, std::string label, int offset);


std::string format(std::string op, std::string rd);
std::string format(std::string op, std::string rd, std::string rs);
std::string format(std::string op, std::string rd, std::string rs, std::string rt);

#endif /* MIPS_hpp */
