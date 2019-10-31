//
//  main.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include <iostream>
#include <cstdio>
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
    PeekQueue<char> data = readIn();
    PeekQueue<Token> tokens = Tokenizer(data);
    Parser parser = Parser(tokens);
    return 0;
}
