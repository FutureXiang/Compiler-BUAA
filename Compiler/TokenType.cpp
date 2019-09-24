//
//  TokenType.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "TokenType.hpp"

std::map<TokenType, std::string> type2str = {
    {name, "IDENFR"},
    
    {intConst, "INTCON"},
    {charConst, "CHARCON"},
    {stringConst, "STRCON"},
    
    {constKey, "CONSTTK"},
    {intKey, "INTTK"},
    {charKey, "CHARTK"},
    {voidKey, "VOIDTK"},
    {mainKey, "MAINTK"},
    
    {ifKey, "IFTK"},
    {elseKey, "ELSETK"},
    {doKey, "DOTK"},
    {whileKey, "WHILETK"},
    {forKey, "FORTK"},
    {scan, "SCANFTK"},
    {print, "PRINTFTK"},
    {returnKey, "RETURNTK"},
    
    {pluss, "PLUS"},
    {minuss, "MINU"},
    {multi, "MULT"},
    {divd, "DIV"},
    
    {lesss, "LSS"},
    {leq, "LEQ"},
    {great, "GRE"},
    {geq, "GEQ"},
    {equalto, "EQL"},
    {neq, "NEQ"},
    
    {assign, "ASSIGN"},
    
    {semi, "SEMICN"},
    {comma, "COMMA"},
    
    {lBracket, "LPARENT"},
    {rBracket, "RPARENT"},
    {lSquare, "LBRACK"},
    {rSquare, "RBRACK"},
    {lCurly, "LBRACE"},
    {rCurly, "RBRACE"},
};
