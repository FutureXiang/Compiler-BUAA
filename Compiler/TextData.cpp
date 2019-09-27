//
//  TextData.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/24.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "TextData.hpp"
TextData::TextData() {
}

void TextData::add(char in) {
    text.push_back(in);
}

char TextData::peek() {
    return text.front();
}

char TextData::pop() {
    char front = text.front();
    text.pop_front();
    return front;
}

bool TextData::empty() {
    return text.empty();
}
