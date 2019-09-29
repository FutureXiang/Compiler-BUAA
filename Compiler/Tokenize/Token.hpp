//
//  Token.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Token_hpp
#define Token_hpp

#include <string>
#include "TokenType.hpp"

class Token {
private:
    std::string text;
    TokenType type;
public:
    Token();
    Token(std::string text, TokenType type);
    std::string toString();
    TokenType getType();
    std::string getText();
};

#endif /* Token_hpp */
