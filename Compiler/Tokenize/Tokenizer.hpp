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

PeekQueue<Token> Tokenizer(PeekQueue<char> data);

#endif /* Tokenizer_hpp */
