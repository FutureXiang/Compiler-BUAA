//
//  Error.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/11/1.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "Error.hpp"

std::string ErrorToString(Error e) {
    return std::string(1, 'a' + e);
}
