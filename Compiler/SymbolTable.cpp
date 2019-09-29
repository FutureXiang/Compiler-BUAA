//
//  SymbolTable.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/29.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "SymbolTable.hpp"

Symbol::Symbol(std::string id_name, SymbolType id_type) {
    name = id_name;
    type = id_type;
}

Symbol::Symbol() {
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
