//
//  TextData.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef TextData_hpp
#define TextData_hpp

#include <deque>

class TextData {
private:
    std::deque<char> text;
public:
    TextData();
    void add(char in);
    char peek();
    char pop();
    bool empty();
};

#endif /* TextData_hpp */
