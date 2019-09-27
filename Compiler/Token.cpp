//
//  Token.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Token.hpp"

Token::Token(){
}

Token::Token(std::string text, TokenType type) {
    this->text = text;
    this->type = type;
}

std::string Token::toString() {
    return type2str[type] + " " + text;
}
