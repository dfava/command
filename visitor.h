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
  V_FLAG_IF_GUARD = 8,
  V_FLAG_IF_THEN = 0xc,
  V_FLAG_IF_ELSE = 0xe,
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
    virtual void visit(NBinaryOperator* nBinaryOperator, uint64_t flag) = 0;
    virtual void visit(NAssignment* nAssignment, uint64_t flag) = 0;
    virtual void visit(NBlock* nBlock, uint64_t flag) = 0;
    virtual void visit(NExpression* nExpression, uint64_t flag) { };
    virtual void visit(NExpressionStatement* nExpressionStatement, uint64_t flag) = 0;
    virtual void visit(NVariableDeclaration* nVariableDeclaration, uint64_t flag) = 0;
};
#endif // __VISITOR_H_
