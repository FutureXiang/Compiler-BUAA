//
//  PeekQueue.hpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/28.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef PeekQueue_hpp
#define PeekQueue_hpp

#include <deque>

template <class T>
class PeekQueue {
private:
    std::deque<T> queue;
public:
    PeekQueue();
    void add(T in);
    T peek();
    T pop();
    bool empty();
};

#endif /* PeekQueue_hpp */
