//
//  TestSymbolTable.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/11/1.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include <iostream>
#include <vector>
#include <assert.h>

#include "SymbolTable.hpp"

void printFuncInfo(SymbolTable &table, std::string name) {
    std::shared_ptr<SymbolFunct> func = std::dynamic_pointer_cast<SymbolFunct>(table.getSymbolByName(name));
    std::cout << "shared_ptr<SymbolFunct> -> SymbolType = " << func->getType() << std::endl;
    
    for(auto arg: func->args) {
        std::cout << "arg type: " << arg->getType() << std::endl;
    }
}

void printArrInfo(SymbolTable &table, std::string name) {
    std::shared_ptr<SymbolArr> arr = std::dynamic_pointer_cast<SymbolArr>(table.getSymbolByName(name));
    std::cout << "shared_ptr<SymbolFunct> -> SymbolType = " << arr->getType() << std::endl;
    std::cout << "dimension: " << arr->dimension << std::endl;
}

void makeFuncs(SymbolTable &table) {
    
    std::shared_ptr<SymbolVar> var1 = std::make_shared<SymbolVar>("arg: intcon", intCon);
    std::shared_ptr<SymbolVar> var2 = std::make_shared<SymbolVar>("arg: charcon", charCon);
    std::shared_ptr<SymbolVar> var3 = std::make_shared<SymbolVar>("arg: intvar", intVar);
    std::shared_ptr<SymbolVar> var4 = std::make_shared<SymbolVar>("arg: charvar", charVar);
    std::vector<std::shared_ptr<SymbolVar> >args1, args2, args3;
    args2.push_back(var1);
    args2.push_back(var3);
    args3.push_back(var2);
    args3.push_back(var4);
    table.addFunc("voidfunc", voidFunct, args1);
    table.addFunc("intfunc", intFunct, args2);
    table.addFunc("charfunc", charFunct, args3);
}

void Test() {
    /* Test Code for Symbols = {Symbol, SymbolVar, SymbolArr, SymbolFunct} and SymbolTable
     * Powered by memory.h/std::shared_ptr
     */
    SymbolTable table = SymbolTable();
    table.addVar("var: intcon", intCon);
    table.addVar("var: charcon", charCon);
    table.addVar("var: intvar", intVar);
    table.addVar("var: charvar", charVar);
    table.addArr("arr: int", intArr, "3");
    table.addArr("arr: char", charArr, "33");
    makeFuncs(table);
    
    printFuncInfo(table, "voidfunc");
    printFuncInfo(table, "intfunc");
    printFuncInfo(table, "charfunc");
    table.pop();
    table.pop();
    table.pop();
    /*de-construction of name = arg: charvar
      de-construction of name = arg: charcon
      de-construction of name = charfunc
      de-construction of name = arg: intvar
      de-construction of name = arg: intcon
      de-construction of name = intfunc
      de-construction of name = voidfunc
     */
    
    printArrInfo(table, "arr: int");
    printArrInfo(table, "arr: char");
    table.pop();
    table.pop();
    /*de-construction of name = arr: char
      de-construction of name = arr: int
     */
    
    std::vector<std::string> names = {"__INVALID__", "var: intcon", "var: charcon", "var: intvar", "var: charvar", "arr: int", "arr: char", "voidfunc", "intfunc", "charfunc"};
    for(auto name : names) {
       bool contains = table.containsByName(name);
       std::cout << name << " contains = " << contains << std::endl;
       if (contains)
           std::cout << name << " type = " << table.getTypeByName(name) << std::endl;
    }
    /*de-construction of name = var: charvar
      de-construction of name = var: intvar
      de-construction of name = var: charcon
      de-construction of name = var: intcon
     */
}
