//
//  SymbolTable.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/29.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "SymbolTable.hpp"
#include <assert.h>


void showContent(std::vector<std::shared_ptr<Symbol> > &table) {
    std::cout << "--------------------" << std::endl;
    for(auto x: table) {
        if (x->isglobal())
            std::cout << x->getName() << " " << x->getType() << std::endl;
        else
            std::cout << "\t" << x->getName() << " " << x->getType() << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

/* ------------------------------------------------------------
* SymbolTable: Polymorphic Container powered by shared_ptr
* ------------------------------------------------------------ */

bool SymbolTable::addVar(std::string id_name, SymbolType id_type, bool is_global) {
    if (containsByName(id_name) && getSymbolByName(id_name)->isglobal() == is_global)
        return false;
    std::shared_ptr<Symbol> sym = std::make_shared<SymbolVar>(id_name, id_type, is_global);
    name2pos[id_name] = table.size();
    table.push_back(sym);
    
    showContent(table);
    
    return true;
}

bool SymbolTable::addArr(std::string id_name, SymbolType id_type, bool is_global, std::string shape) {
    if (containsByName(id_name) && getSymbolByName(id_name)->isglobal() == is_global)
        return false;
    std::shared_ptr<Symbol> sym = std::make_shared<SymbolArr>(id_name, id_type, is_global, shape);
    name2pos[id_name] = table.size();
    table.push_back(sym);
    
    showContent(table);
    
    return true;
}

bool SymbolTable::addFunc(std::string id_name, SymbolType id_type, bool is_global, std::vector<std::shared_ptr<SymbolVar> > args) {
    assert(is_global == true);
    // Function must be global!
    if (containsByName(id_name) && getSymbolByName(id_name)->isglobal() == true)
        return false;
    std::shared_ptr<Symbol> sym = std::make_shared<SymbolFunct>(id_name, id_type, is_global, args);
    name2pos[id_name] = table.size();
    table.push_back(sym);
    
    // Function arguments always local, thus they must be valid !
    for (auto arg: args)
        addVar(arg->getName(), arg->getType(), false);
    
    showContent(table);
    
    return true;
}

bool SymbolTable::containsByName(std::string id_name) {
    return name2pos.count(id_name);
}

std::shared_ptr<Symbol> SymbolTable::getSymbolByName(std::string id_name) {
    assert(name2pos.count(id_name) == 1);
    return table[name2pos[id_name]];
}

SymbolType SymbolTable::getTypeByName(std::string id_name) {
    return getSymbolByName(id_name)->getType();
}

void SymbolTable::pop() {
    std::string name = table.back()->getName();
    name2pos.erase(name);
    table.pop_back();
    for (int pos = table.size() - 1; pos >= 0 ; --pos) {
        if (table[pos]->getName() == name) {
            name2pos[name] = pos;
            break;
        }
    }
    showContent(table);
}

size_t SymbolTable::size() {
    return table.size();
}

