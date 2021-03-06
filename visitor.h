// Copyright (C) 2014, Daniel S. Fava
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
#ifndef __VISITOR_H_
#define __VISITOR_H_
#include "node.h"
#include <stdint.h>

class NSkip;
class NInteger;
class NBool;
class NDouble;
class NType;
class NSecurity;
class NIdentifier;
class NIfExpression;
class NWhileExpression;
class NBinaryOperator;
class NAssignment;
class NBlock;
class NExpression;
class NExpressionStatement;
class NVariableDeclaration;

// Visitor Flags
enum {
  V_FLAG_NONE = -1,
  V_FLAG_ENTER = 0,
  V_FLAG_EXIT = 1,
  V_FLAG_GUARD = 8,
  V_FLAG_THEN = 0xc,
  V_FLAG_ELSE = 0xe,
};

class Visitor {
public:
    virtual void visit(NSkip* nSkip, uint64_t flag) = 0;
    virtual void visit(NInteger* nInteger, uint64_t flag) = 0;
    virtual void visit(NBool* nBool, uint64_t flag) = 0;
    virtual void visit(NDouble* nDouble, uint64_t flag) = 0;
    virtual void visit(NType* nType, uint64_t flag) = 0;
    virtual void visit(NSecurity* nSecurity, uint64_t flag) = 0;
    virtual void visit(NIdentifier* nIdentifier, uint64_t flag) = 0;
    virtual void visit(NIfExpression* nIfExpression, uint64_t flag) = 0;
    virtual void visit(NWhileExpression* nWhileExpression, uint64_t flag) = 0;
    virtual void visit(NBinaryOperator* nBinaryOperator, uint64_t flag) = 0;
    virtual void visit(NAssignment* nAssignment, uint64_t flag) = 0;
    virtual void visit(NBlock* nBlock, uint64_t flag) = 0;
    virtual void visit(NExpression* nExpression, uint64_t flag) { };
    virtual void visit(NExpressionStatement* nExpressionStatement, uint64_t flag) = 0;
    virtual void visit(NVariableDeclaration* nVariableDeclaration, uint64_t flag) = 0;
};
#endif // __VISITOR_H_
