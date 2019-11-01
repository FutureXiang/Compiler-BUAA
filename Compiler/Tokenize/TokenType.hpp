//
//  TokenType.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef TokenType_h
#define TokenType_h

#include <map>
#include <string>

enum TokenType {
    __invalidToken__,
    name,
    intConst, charConst, stringConst,
    constKey, intKey, charKey, voidKey, mainKey,
    ifKey, elseKey, doKey, whileKey, forKey,
    scan, print, returnKey,
    pluss, minuss, multi, divd,
    lesss, leq, great, geq, equalto, neq,
    assign,
    semi, comma,
    lBracket, rBracket,
    lSquare, rSquare,
    lCurly, rCurly
};

extern std::map<TokenType, std::string> type2str;
extern std::map<std::string, TokenType> extractKeys;

#endif /* TokenType_h */
