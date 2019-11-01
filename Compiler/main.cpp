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
#include "PeekQueue.cpp"
#include "Symbol/TestSymbolTable.hpp"

using namespace std;

PeekQueue<char> readIn() {
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    freopen("error.txt", "w", stderr);
    char character;
    
    PeekQueue<char> data = PeekQueue<char>();
    
    while (scanf("%c", &character) != EOF)
        data.add(character);
    return data;
}

int main() {
    // Test();
    set<std::pair<int, std::string> > errorMessages;
    
    Tokenizer tokenizer = Tokenizer(errorMessages, readIn());
    Parser parser = Parser(errorMessages, tokenizer.tokens);
    
    for(auto message: tokenizer.errorMessages)
        cerr << message.first << " " << message.second << endl;
    return 0;
}
