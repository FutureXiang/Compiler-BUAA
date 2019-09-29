//
//  SymbolTable.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/29.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef SymbolTable_hpp
#define SymbolTable_hpp

#include <string>

enum SymbolType {
    voidFunct, intFunct, charFunct,
    intVar, charVar,
    intArr, charArr,
    intCon, charCon,
};

class Symbol {
private:
    std::string name;
    SymbolType type;
public:
    Symbol(std::string id_name, SymbolType id_type);
    Symbol();
    bool isFunc();
    bool isVar();
    bool isArr();
    bool isCon();
    bool isVoidFunc();
    bool isNonvoidFunc();
    SymbolType getType();
};

#endif /* SymbolTable_hpp */
