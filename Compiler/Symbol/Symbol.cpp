//
//  Symbol.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/11/1.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Symbol.hpp"


/* ------------------------------------------------------------
 * Symbol: Base Class for {SymbolVar, SymbolArr, SymbolFunc}
 * ------------------------------------------------------------ */

Symbol::Symbol(std::string id_name, SymbolType id_type) {
    name = id_name;
    type = id_type;
}

bool Symbol::isFunc() {
    return (type == voidFunct || type == intFunct || type == charFunct);
}

bool Symbol::isVar() {
    return (type == intVar || type == charVar);
}

bool Symbol::isArr() {
    return (type == intArr || type == charArr);
}

bool Symbol::isCon() {
    return (type == intCon || type == charCon);
}

bool Symbol::isVoidFunc() {
    return (type == voidFunct);
}

bool Symbol::isNonvoidFunc() {
    return (type == intFunct || type == charFunct);
}

SymbolType Symbol::getType() {
    return type;
}

std::string Symbol::getName() {
    return name;
}

/* ------------------------------------------------------------
* SymbolVar: Derived Class from Symbol
* ------------------------------------------------------------ */

SymbolVar::SymbolVar(std::string id_name, SymbolType id_type): Symbol(id_name, id_type) {
    std::cout << "construction of name = " << name << std::endl;
}

/* ------------------------------------------------------------
* SymbolArr: Derived Class from Symbol
* ------------------------------------------------------------ */

SymbolArr::SymbolArr(std::string id_name, SymbolType id_type, std::string dim): Symbol(id_name, id_type) {
    dimension = dim;
    std::cout << "construction of name = " << name << std::endl;
}

/* ------------------------------------------------------------
* SymbolFunct: Derived Class from Symbol
* ------------------------------------------------------------ */

SymbolFunct::SymbolFunct(std::string id_name, SymbolType id_type, std::vector<std::shared_ptr<SymbolVar> > argList): Symbol(id_name, id_type) {
    args = argList;
    std::cout << "construction of name = " << name << std::endl;
}
