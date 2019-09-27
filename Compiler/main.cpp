//
//  main.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include <iostream>
#include <cstdio>
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "PeekQueue.cpp"

using namespace std;

PeekQueue<char> readIn() {
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    char character;
    
    PeekQueue<char> data = PeekQueue<char>();
    
    while (scanf("%c", &character) != EOF)
        data.add(character);
    return data;
}

int main() {
    PeekQueue<char> data = readIn();
    cout << "Tokenizer Result:" << endl;
    PeekQueue<Token> tokens = Tokenizer(data);
    cout << "Parser Result:" << endl;
    Parser(tokens);
    return 0;
}
