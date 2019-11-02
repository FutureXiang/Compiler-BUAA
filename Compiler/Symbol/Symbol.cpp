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

Symbol::Symbol(std::string id_name, SymbolType id_type, bool is_global) {
    name = id_name;
    type = id_type;
    global = is_global;
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

SymbolVar::SymbolVar(std::string id_name, SymbolType id_type, bool is_global): Symbol(id_name, id_type, is_global) {
//    std::cout << "construction of name = " << name << std::endl;
}

/* ------------------------------------------------------------
* SymbolArr: Derived Class from Symbol
* ------------------------------------------------------------ */

SymbolArr::SymbolArr(std::string id_name, SymbolType id_type, bool is_global, std::string dim): Symbol(id_name, id_type, is_global) {
    dimension = dim;
//    std::cout << "construction of name = " << name << std::endl;
}

/* ------------------------------------------------------------
* SymbolFunct: Derived Class from Symbol
* ------------------------------------------------------------ */

SymbolFunct::SymbolFunct(std::string id_name, SymbolType id_type, bool is_global, std::vector<std::shared_ptr<SymbolVar> > argList): Symbol(id_name, id_type, is_global) {
    args = argList;
//    std::cout << "construction of name = " << name << std::endl;
}
