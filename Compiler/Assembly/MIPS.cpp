//
//  MIPS.cpp
//  Compiler
//
//  Created by xiangweilai on 2019/11/16.
//  Copyright © 2019 Xiang Weilai. All rights reserved.
//

#include "MIPS.hpp"
#include <utility>
#include <algorithm>


// VAR, x;  VAR, arr, 5;  VAR, "content" (name)
dataLabel::dataLabel(std::string name) {
    label = name;
    type = word;
}
dataLabel::dataLabel(std::string name, int size) {
    label = name;
    type = space;
    arrayShape = size;
}
dataLabel::dataLabel(std::string name, std::string quote) {
    label = name;
    type = asciiz;
    stringContent = quote;
}
std::string dataLabel::toString() {
    std::string code = label + " : ";
    switch (type) {
        case word:
            return code + ".word 0";
            break;
        case space:
            return code + ".space " + std::to_string(arrayShape * 4);
            break;
        default:
            return code + ".asciiz " + "\"" + stringContent + "\"";
            break;
    }
}


// WRITE_INT, WRITE_CHAR, WRITE_STR, READ_INT, READ_CHAR
std::string ReadWrite(Quadruple qcode) {
    switch (qcode.op) {
        case READ_INT:
            return format("li", "$v0", "5") + "\nsyscall\n" +
                   format("move", qcode.target->name, "$v0");
            break;
        case READ_CHAR:
            return format("li", "$v0", "12") + "\nsyscall\n" +
                   format("move", qcode.target->name, "$v0");
            break;
        case WRITE_INT:
            if (qcode.target->is_instant)
                return format("li", "$a0", qcode.target->toString()) +"\n"+ format("li", "$v0", "1") + "\nsyscall";
            else
                return format("move", "$a0", qcode.target->name) +"\n"+ format("li", "$v0", "1") + "\nsyscall";
            break;
        case WRITE_CHAR:
            if (qcode.target->is_instant)
                return format("li", "$a0", qcode.target->toString()) +"\n"+ format("li", "$v0", "11") + "\nsyscall";
            else
                return format("move", "$a0", qcode.target->name) +"\n"+ format("li", "$v0", "11") + "\nsyscall";
            break;
        default:
            return format("la", "$a0", qcode.target->name) +"\n"+ format("li", "$v0", "4") + "\nsyscall";
            break;
    }
}


// LARR, SARR
// OperandSymbol->isTemp() == false
std::string LwSw(char mode, std::string target, std::string label, int offset) {
    std::string head = "lw";
    if (mode == 's')
        head = "sw";
    return format(head, target, label, std::to_string(offset));
}


std::string format(std::string op, std::string rd) {
    std::string space( std::max(8 - (int)op.length(),1), ' ');
    return op + space + rd;
}
std::string format(std::string op, std::string rd, std::string rs) {
    std::string space( std::max(8 - (int)rd.length(),1), ' ');
    return format(op, rd) + "," + space + rs;
}
std::string format(std::string op, std::string rd, std::string rs, std::string rt) {
    if (op == "lw" || op == "sw")
        return format(op, rd, rt) + "("+ rs +")";
    else {
        std::string space(std::max(8 - (int)rs.length(),1), ' ');
        return format(op, rd, rs) + "," + space + rt;
    }
}

std::string ArithComp(Quadruple qcode) {
    if (qcode.first->is_instant && qcode.second->is_instant) {              // ADD, t0, -1, -2
        int x = ((OperandInstant *)qcode.first)->value;
        int y = ((OperandInstant *)qcode.second)->value;
        int res = 0;
        switch (qcode.op) {
            case ADD:
                res = x+y;
                break;
            case SUB:
                res = x-y;
                break;
            case MULT:
                res = x*y;
                break;
            case DIV:
                res = x/y;
                break;
            case SLT:
                res = x<y;
                break;
            case SLEQ:
                res = x<=y;
                break;
            case SGT:
                res = x>y;
                break;
            case SGEQ:
                res = x>=y;
                break;
            case SEQ:
                res = x==y;
                break;
            case SNE:
                res = x!=y;
                break;
            default:
                break;
        }
        return format("li", qcode.target->name, std::to_string(res));
    } else if (qcode.first->is_instant || qcode.second->is_instant) {
        if (qcode.first->is_instant) {                                      // SUB, t0, -1, t1
            std::string x = std::to_string(((OperandInstant *)qcode.first)->value);
            std::string y = qcode.second->name;
            std::string z = qcode.target->name;
            std::string temp = "a0";
            switch (qcode.op) {
                case ADD:
                    return format("addu", z, y, x);
                    break;
                case SUB:       // subu t0, t1, -1;    neg  t0, t0
                    return format("subu", z, y, x) + "\n" + format("neg", z, z);
                    break;
                case MULT:
                    return format("mul", z, y, x);
                    break;
                case DIV:       // li   a0, -1;        div  t0, a0, t1
                    return format("li", temp, x) + "\n" + format("div", z, temp, y);
                    break;
                case SLT:       // sgt  t0, t1, -1
                    return format("sgt", z, y, x);
                    break;
                case SLEQ:      // sge  t0, t1, -1
                    return format("sge", z, y, x);
                    break;
                case SGT:       // li   a0, -1;        sgt  t0, a0, t1
                    return format("li", temp, x) + "\n" + format("sgt", z, temp, y);
                    break;
                case SGEQ:      // li   a0, -1;        sge  t0, a0, t1
                    return format("li", temp, x) + "\n" + format("sge", z, temp, y);
                    break;
                case SEQ:
                    return format("seq", z, y, x);
                    break;
                case SNE:
                    return format("sne", z, y, x);
                    break;
                default:
                    return "";
                    break;
            }
        } else {                                                            // SUB, t0, t1, -2
            std::string x = qcode.first->name;
            std::string y = std::to_string(((OperandInstant *)qcode.second)->value);
            std::string z = qcode.target->name;
            switch (qcode.op) {
                case ADD:
                    return format("addu", z, x, y);
                    break;
                case SUB:
                    return format("subu", z, x, y);
                    break;
                case MULT:
                    return format("mul", z, x, y);
                    break;
                case DIV:
                    return format("div", z, x, y);
                    break;
                case SLT:       // sge  t0, t1, -2;     subu t0, 1, t0
                    return format("sge", z, x, y) + "\n" + format("subu", z, "1", z);
                    break;
                case SLEQ:
                    return format("sle", z, x, y);
                    break;
                case SGT:
                    return format("sgt", z, x, y);
                    break;
                case SGEQ:
                    return format("sge", z, x, y);
                    break;
                case SEQ:
                    return format("seq", z, x, y);
                    break;
                case SNE:
                    return format("sne", z, x, y);
                    break;
                default:
                    return "";
                    break;
            }
        }
    } else {                                                                // MULT, t2, t0, t1
        std::string x = qcode.first->name;
        std::string y = qcode.second->name;
        std::string z = qcode.target->name;
        switch (qcode.op) {
            case ADD:
                return format("addu", z, x, y);
                break;
            case SUB:
                return format("subu", z, x, y);
                break;
            case MULT:
                return format("mul", z, x, y);
                break;
            case DIV:
                return format("div", z, x, y);
                break;
            case SLT:
                return format("slt", z, x, y);
                break;
            case SLEQ:
                return format("sle", z, x, y);
                break;
            case SGT:
                return format("sgt", z, x, y);
                break;
            case SGEQ:
                return format("sge", z, x, y);
                break;
            case SEQ:
                return format("seq", z, x, y);
                break;
            case SNE:
                return format("sne", z, x, y);
                break;
            default:
                return "";
                break;
        }
    }
}
