//
//  SymbolTable.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/29.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "SymbolTable.hpp"

/* ------------------------------------------------------------
* SymbolTable: Polymorphic Container powered by shared_ptr
* ------------------------------------------------------------ */

bool SymbolTable::addVar(std::string id_name, SymbolType id_type) {
    if (name2pos.count(id_name) != 0)
        return false;
    std::shared_ptr<Symbol> sym = std::make_shared<SymbolVar>(id_name, id_type);
    name2pos[id_name] = table.size();
    table.push_back(sym);
    return true;
}

bool SymbolTable::addArr(std::string id_name, SymbolType id_type, std::string shape) {
    if (name2pos.count(id_name) != 0)
        return false;
    std::shared_ptr<Symbol> sym = std::make_shared<SymbolArr>(id_name, id_type, shape);
    name2pos[id_name] = table.size();
    table.push_back(sym);
    return true;
}

bool SymbolTable::addFunc(std::string id_name, SymbolType id_type, std::vector<std::shared_ptr<SymbolVar> > args) {
    if (name2pos.count(id_name) != 0)
        return false;
    std::shared_ptr<Symbol> sym = std::make_shared<SymbolFunct>(id_name, id_type, args);
    name2pos[id_name] = table.size();
    table.push_back(sym);
    return true;
}

bool SymbolTable::containsByName(std::string id_name) {
    return name2pos.count(id_name);
}

std::shared_ptr<Symbol> SymbolTable::getSymbolByName(std::string id_name) {
    return table[name2pos[id_name]];
}

SymbolType SymbolTable::getTypeByName(std::string id_name) {
    return getSymbolByName(id_name)->getType();
}

void SymbolTable::pop() {
    name2pos.erase(table.back()->getName());
    table.pop_back();
}

size_t SymbolTable::size() {
    return table.size();
}
