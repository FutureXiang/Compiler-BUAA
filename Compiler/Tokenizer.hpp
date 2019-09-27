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
#include <vector>
#include "TextData.hpp"
#include "Token.hpp"

std::vector<Token> Tokenizer(TextData data);

#endif /* Tokenizer_hpp */
