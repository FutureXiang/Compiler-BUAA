//
//  main.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include <iostream>
#include <cstdio>
#include <set>
#include "Tokenize/Token.hpp"
#include "Tokenize/Tokenizer.hpp"
#include "Parser.hpp"
#include "Assembly/QuadToMIPS.hpp"

using namespace std;

PeekQueue<char> readIn() {
    freopen("testfile.txt", "r", stdin);
    freopen("mips.txt", "w", stdout);
    char character;
    
    PeekQueue<char> data = PeekQueue<char>();
    
    while (scanf("%c", &character) != EOF)
        data.add(character);
    return data;
}

int main() {
    set<std::pair<int, std::string> > errorMessages;
    
    Tokenizer tokenizer = Tokenizer(errorMessages, readIn());
    Parser parser = Parser(errorMessages, tokenizer.tokens);
    
//    for(auto message: tokenizer.errorMessages)
//        cerr << message.first << " " << message.second << endl;
    
    std::vector<Quadruple> *qcodes_total = parser.getQcodes();
    Interpreter convert = Interpreter(qcodes_total);
    for (std::string mips: convert.getMIPS())
        cout << mips << endl;
    return 0;
}
