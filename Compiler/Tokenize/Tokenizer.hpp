//
//  Tokenizer.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/27.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Tokenizer_hpp
#define Tokenizer_hpp

#include <iostream>
#include <cstdio>
#include "../PeekQueue.hpp"
#include "Token.hpp"
#include <set>

class Tokenizer {
public:
    std::set<std::pair<int, std::string> > &errorMessages;
    void error(int line, std::string x);
    
    PeekQueue<Token> tokens;
    Tokenizer(std::set<std::pair<int, std::string> > &errorMessages, PeekQueue<char> data); // Input a copy of Char List, the reference of Global Message Contrainer
};

#endif /* Tokenizer_hpp */
