//
//  Error.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/11/1.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef Error_hpp
#define Error_hpp

#include <string>

enum Error {
    token_invalid,
    id_redef, id_nodef,
    args_lenmis, args_typemis,
    cond_invalid,
    void_misret, nonvoid_misret,
    index_notint,
    const_assign,
    semi_mis, bracket_mis, square_mis,
    while_mis,
    const_value,
    __othererror__,
};

std::string ErrorToString(Error e);

#endif /* Error_hpp */
