//
//  main.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include <iostream>
#include <cstdio>
#include <vector>
#include "TextData.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"

using namespace std;

TextData readIn() {
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    char character;
    
    TextData data = TextData();
    
    while (scanf("%c", &character) != EOF)
        data.add(character);
    return data;
}

int main() {
    TextData data = readIn();
    vector<Token> tokens = Tokenizer(data);
    return 0;
}
