//
//  BlocksDivide.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/12/4.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "BlocksDivide.hpp"
#include <assert.h>

std::vector<CodeBlock *> Divider(std::vector<Quadruple> *qcodes) {
    std::vector<CodeBlock *> codeblocks;
    int start = 0;
    for (int i = 0; i < qcodes->size(); ++i) {
        Operator op = (*qcodes)[i].op;
        if (op == LABEL || op == GOTO || (op >= BEQ && op <= BNEZ)) {
            // THIS INSTRUCTION belongs to the PREVIOUS Block !!
            codeblocks.push_back(new CodeBlock(start, i));
            start = i + 1;
        }
    }
    codeblocks.push_back(new CodeBlock(start, qcodes->size() - 1));
    
    buildGraph(qcodes, codeblocks);
    liveVariableAnalysis(qcodes, codeblocks);
    return codeblocks;
}

void buildGraph(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks) {
    std::map<std::string, int> lineNoForLabel;
    std::map<int, CodeBlock *> lineNoToRange;

    for (auto block: blocks) {
        for (int i = block->start; i <= block->end; ++i)
            lineNoToRange[i] = block;
    }
    for (int i = 0; i < qcodes->size(); ++i) {
        if ((*qcodes)[i].op == LABEL)
            lineNoForLabel[(*qcodes)[i].target->toString()] = i;
    }
    
    for (auto block: blocks) {
        for (int i = block->start; i <= block->end; ++i) {
            Operator op = (*qcodes)[i].op;
            if (op == GOTO || (op >= BEQ && op <= BNEZ) || op == CALL) {
                std::string target = (*qcodes)[i].target->toString();
                assert (lineNoForLabel.count(target));
                int lineNo = lineNoForLabel[target];
                
                if (lineNoToRange.count(lineNo+1))
                    block->addEdge(lineNoToRange[lineNo+1]);                    // branch / jump target
            }
        }
        if ((*qcodes)[block->end].op != GOTO) {
            if (lineNoToRange.count(block->end+1))                              // SEQUENTIAL
                block->addEdge(lineNoToRange[block->end+1]);
        }
    }
}

void liveVariableAnalysis(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks) {
    useDefAnalysis(qcodes, blocks);
    bool finished = false;
    while (!finished) {
        finished = true;
        for (auto block: blocks) {
            std::set<std::string> temp_out, temp_in;
            for (auto next_block: block->next)
                temp_out = set_union(temp_out, next_block->in);
            
            block->out = temp_out;
            temp_in = set_diff(block->out, block->def);
            temp_in = set_union(block->use, temp_in);
            
            if (temp_in != block->in) {
                finished = false;
                block->in = temp_in;
            }
        }
    }
    bool OUTPUT_OUT = false;
    if (OUTPUT_OUT) {
        for (auto block: blocks) {
            if (block->out.size())
                set_toString(block->toString() + " OUT", block->out);
        }
    }
}

void useDefAnalysis(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks) {
    for (auto block: blocks) {
        std::set<std::string> visited;
        for (int i = block->start; i <= block->end; ++i) {
            Operator op = (*qcodes)[i].op;
            Operand *target = (*qcodes)[i].target;
            Operand *first = (*qcodes)[i].first;
            Operand *second = (*qcodes)[i].second;
            if (op == VAR || op == PARAM)
                continue;
            // x=x+y: use
            if (first != nullptr && is_var(first) && !visited.count(first->toString())) {
                visited.insert(first->toString());
                block->use.insert(first->toString());
            }
            if (second != nullptr && is_var(second) && !visited.count(second->toString())) {
                visited.insert(second->toString());
                block->use.insert(second->toString());
            }
            if (target != nullptr && is_var(target) && !visited.count(target->toString())) {
                visited.insert(target->toString());
                if (modify_target_operators.count(op))
                    block->def.insert(target->toString());
                else if (ref_target_operators.count(op))
                    block->use.insert(target->toString());
            }
        }
        assert((block->def.size() + block->use.size()) == visited.size());
        bool OUTPUT_USE_DEF = false;
        if (OUTPUT_USE_DEF) {
            if (block->use.size())
                set_toString(block->toString() + " use", block->use);
            if (block->def.size())
                set_toString(block->toString() + " def", block->def);
        }
    }
}

std::set<std::string> set_diff(std::set<std::string> x, std::set<std::string> y) {
    std::set<std::string> result;
    for (auto element: x) {
        if (!y.count(element))
            result.insert(element);
    }
    return result;
}

std::set<std::string> set_union(std::set<std::string> x, std::set<std::string> y) {
    std::set<std::string> result = x;
    for (auto element: y)
        result.insert(element);
    return result;
}

void set_toString(std::string title, std::set<std::string> &aSet) {
    std::cerr << title << " = { ";
    for (auto e: aSet)
        std::cerr << e << ", ";
    std::cerr << " }" << std::endl;
}

std::set<int> deadCodeElimination(std::vector<Quadruple> *const qcodes, const std::vector<CodeBlock *> &blocks) {
    std::map<int, std::set<std::string> > percode_use;
    std::map<int, std::set<std::string> > percode_def;
    
    // Use-Def Analysis Per Code
    for (auto block: blocks) {
        for (int i = block->start; i <= block->end; ++i) {
            Operator op = (*qcodes)[i].op;
            Operand *target = (*qcodes)[i].target;
            Operand *first = (*qcodes)[i].first;
            Operand *second = (*qcodes)[i].second;
            
            std::set<std::string> visited;
            
            if (op == VAR || op == PARAM)
                continue;
            // x=x+y: use
            if (first != nullptr && is_var(first) && !visited.count(first->toString())) {
                visited.insert(first->toString());
                percode_use[i].insert(first->toString());
            }
            if (second != nullptr && is_var(second) && !visited.count(second->toString())) {
                visited.insert(second->toString());
                percode_use[i].insert(second->toString());
            }
            if (target != nullptr && is_var(target) && !visited.count(target->toString())) {
                visited.insert(target->toString());
                if (modify_target_operators.count(op))
                    percode_def[i].insert(target->toString());
                else if (ref_target_operators.count(op))
                    percode_use[i].insert(target->toString());
            }
        }
    }
    
    std::set<int> deads;
    std::map<int, std::set<std::string> > percode_in;
    std::map<int, std::set<std::string> > percode_out;
    
    // Live Variable Analysis Per Code
    for (auto block: blocks) {
        bool finished = false;
        while (!finished) {
            finished = true;
            for (int i = block->end; i >= block->start; --i) {
                std::set<std::string> temp_out, temp_in;
                if (i == block->end)
                    temp_out = block->out;
                else
                    temp_out = percode_in[i+1];
                
                percode_out[i] = temp_out;
                temp_in = set_diff(percode_out[i], percode_def[i]);
                temp_in = set_union(percode_use[i], temp_in);
                
                if (temp_in != percode_in[i]) {
                    finished = false;
                    percode_in[i] = temp_in;
                }
            }
        }
        for (int i = block->start; i <= block->end; ++i) {
            Operator op = (*qcodes)[i].op;
            Operand *target = (*qcodes)[i].target;
            if (target != nullptr && is_var(target) && modify_target_operators.count(op))
                if (!percode_out[i].count(target->toString()) && op != READ_INT && op != READ_CHAR) {
                    deads.insert(i);
                    std::cerr << "Dead Code Elimination :: " << (*qcodes)[i].toString() << std::endl;
                }
        }
    }
    return deads;
}
