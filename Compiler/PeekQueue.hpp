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
    int poped_index = -1;
public:
    PeekQueue();
    void add(T in);
    T peek();
    T peek(int n);
    T pop();
    bool empty();
    int size();
    int last_poped_index();
};

#endif /* PeekQueue_hpp */
