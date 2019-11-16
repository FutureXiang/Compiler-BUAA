//
//  PeekQueue.cpp
//  Compiler
//
//  Created by Xiang Weilai on 2019/9/27.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "PeekQueue.hpp"

template <class T>
PeekQueue<T>::PeekQueue() {
}

template <class T>
void PeekQueue<T>::add(T in) {
    queue.push_back(in);
}

template <class T>
T PeekQueue<T>::peek() {
    return queue.front();
}

template <class T>
T PeekQueue<T>::pop() {
    T front = queue.front();
    queue.pop_front();
    return front;
}

template <class T>
bool PeekQueue<T>::empty() {
    return queue.empty();
}

template <class T>
T PeekQueue<T>::peek(int n) {
    return queue[n-1];
}

template <class T>
int PeekQueue<T>::size() {
    return queue.size();
}
