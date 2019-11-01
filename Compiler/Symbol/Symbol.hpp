//
//  Symbol.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/11/1.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Symbol_hpp
#define Symbol_hpp

#include <string>
#include <vector>
#include <iostream>

enum ExprType {
    intType, charType
};

enum SymbolType {
    voidFunct, intFunct, charFunct,
    intVar, charVar,
    intArr, charArr,
    intCon, charCon,
};

class Symbol {
protected:
    std::string name;
    SymbolType type;
    bool global;
public:
    Symbol(std::string id_name, SymbolType id_type, bool is_global);
    Symbol() = default;
    virtual ~Symbol() {
        std::cout << "de-construction of name = " << name << std::endl;
    };
    bool isFunc();
    bool isVar();
    bool isArr();
    bool isCon();
    bool isVoidFunc();
    bool isNonvoidFunc();
    bool isglobal() {return global;}
    SymbolType getType();
    std::string getName();
};


class SymbolVar: public Symbol {
public:
    SymbolVar(std::string id_name, SymbolType id_type, bool is_global);
};

class SymbolArr: public Symbol {
public:
    std::string dimension;
    SymbolArr(std::string id_name, SymbolType id_type, bool is_global, std::string dim);
};

class SymbolFunct: public Symbol {
public:
    std::vector<std::shared_ptr<SymbolVar> > args;
    SymbolFunct(std::string id_name, SymbolType id_type, bool is_global, std::vector<std::shared_ptr<SymbolVar> > args);
};

#endif /* Symbol_hpp */
