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
            return ".align 2\n" + code + ".space " + std::to_string(arrayShape * 4);
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
            if (qcode.target->name == "newline")
                return format("li", "$a0", "10") +"\n"+ format("li", "$v0", "11") + "\nsyscall";
            else
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

std::string Arith(Quadruple qcode) {
    if (qcode.first->is_instant && qcode.second->is_instant) {                  // ADD, t0, -1, -2
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
            default:
                break;
        }
        return format("li", qcode.target->name, std::to_string(res));
    } else if (qcode.first->is_instant) {                                       // SUB, t0, -1, t1
        int x_value = ((OperandInstant *)qcode.first)->value;
        std::string y = qcode.second->toString();
        std::string z = qcode.target->toString();
        std::string temp = "$a0";
        switch (qcode.op) {
            case ADD:
                return format("addiu", z, y, std::to_string(x_value));
                break;
            case SUB:       // subu t0, t1, -1;    neg  t0, t0
                return format("addiu", z, y, std::to_string(-x_value)) + "\n" + format("neg", z, z);
                break;
            case MULT:
                return format("mul", z, y, std::to_string(x_value));
                break;
            case DIV:       // li   a0, -1;        div  t0, a0, t1
                return format("li", temp, std::to_string(x_value)) + "\n" + format("div", z, temp, y);
                break;
            default:
                return "";
                break;
        }
    } else if (qcode.second->is_instant) {                                      // MULT, t2, t0, 5
        std::string x = qcode.first->toString();
        int y_value = ((OperandInstant *)qcode.second)->value;
        std::string z = qcode.target->toString();
        switch (qcode.op) {
            case ADD:
                return format("addiu", z, x, std::to_string(y_value));          // addiu: [-32768, 32767]->ONE instuction, others->THREE
                break;
            case SUB:
                return format("addiu", z, x, std::to_string(-y_value));         // addiu: [-32768, 32767]->ONE instuction, others->THREE
                break;
            case MULT:
                return format("mul", z, x, std::to_string(y_value));
                break;
            case DIV:
                return format("div", z, x, std::to_string(y_value));
                break;
            default:
                return "";
                break;
        }
    } else {                                                                    // MULT, t2, t0, t1
        std::string x = qcode.first->toString();
        std::string y = qcode.second->toString();
        std::string z = qcode.target->toString();
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
            default:
                return "";
                break;
        }
    }
}

std::string CompBranch(Quadruple qcode) {
    if (qcode.op == BEQZ) {
        if (qcode.first->is_instant) {
            if (((OperandInstant *)qcode.first)->value == 0)
                return format("j", qcode.target->toString());
            else
                return "";
        }
        else
            return format("beqz", qcode.first->toString(), qcode.target->toString());
    }
    if (qcode.op == BNEZ) {
        if (qcode.first->is_instant) {
            if (((OperandInstant *)qcode.first)->value != 0)
                return format("j", qcode.target->toString());
            else
                return "";
        }
        else
            return format("bnez", qcode.first->toString(), qcode.target->toString());
    }
    std::map<Operator, std::string> sixQop2sixMop = {
        {BEQ, "beq"},
        {BNE, "bne"},
        {BGT, "bgt"},
        {BGE, "bge"},
        {BLT, "blt"},
        {BLE, "ble"},
    };
    return format(sixQop2sixMop[qcode.op], qcode.first->toString(), qcode.second->toString(), qcode.target->toString());
}
