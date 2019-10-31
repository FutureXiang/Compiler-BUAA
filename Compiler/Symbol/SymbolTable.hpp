//
//  SymbolTable.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/29.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef SymbolTable_hpp
#define SymbolTable_hpp

#include <map>
#include <memory>
#include "Symbol.hpp"


class SymbolTable {
private:
    std::vector<std::shared_ptr<Symbol> > table;
    std::map<std::string, size_t> name2pos;
public:
    SymbolTable() = default;
    ~SymbolTable() = default;

    bool addVar(std::string id_name, SymbolType id_type);
    bool addArr(std::string id_name, SymbolType id_type, std::string shape);
    bool addFunc(std::string id_name, SymbolType id_type, std::vector<std::shared_ptr<SymbolVar> > args);
    
    bool containsByName(std::string id_name);
    SymbolType getTypeByName(std::string id_name);
    std::shared_ptr<Symbol> getSymbolByName(std::string id_name);
    
    void pop();
    size_t size();
};

#endif /* SymbolTable_hpp */
