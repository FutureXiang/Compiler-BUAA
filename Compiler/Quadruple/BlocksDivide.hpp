//
//  BlocksDivide.hpp
//  Compiler
//
//  Created by xiangweilai on 2019/12/4.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#ifndef BlocksDivide_hpp
#define BlocksDivide_hpp

#include <vector>
#include "Quadruple.hpp"


class CodeBlock {
private:

public:
    int start;
    int end;
    std::vector<CodeBlock *> next;
    std::set<std::string> use;
    std::set<std::string> def;
    std::set<std::string> in;
    std::set<std::string> out;
    
    CodeBlock(int s, int e) {
        start = s;
        end = e;
    }

    std::pair<int, int> getRange() {
        return std::make_pair(start, end);
    }

    void addEdge(CodeBlock *another) {
        next.push_back(another);
//        std::cerr << this->toString() << "--->" << another->toString() << std::endl;
    }
    
    std::string toString() {
        return "(" + std::to_string(start+1) + ", " + std::to_string(end+1) + ")";
    }
};

std::vector<CodeBlock *> Divider(std::vector<Quadruple> *qcodes);
void buildGraph(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks);
void liveVariableAnalysis(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks);
void useDefAnalysis(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks);

std::set<std::string> set_diff(std::set<std::string> x, std::set<std::string> y);
std::set<std::string> set_union(std::set<std::string> x, std::set<std::string> y);
void set_toString(std::string title, std::set<std::string> &aSet);

std::set<int> deadCodeElimination(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks);

#endif /* BlocksDivide_hpp */
