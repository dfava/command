#ifndef __VISITOR_H_
#define __VISITOR_H_
#include "node.h"

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
class NFunctionDeclaration;
class NMethodCall;

class Visitor {
public:
    virtual void visit(NInteger* nInteger) = 0;
    virtual void visit(NBool* nBool) = 0;
    virtual void visit(NDouble* nDouble) = 0;
    virtual void visit(NType* nType) = 0;
    virtual void visit(NSecurity* nSecurity) = 0;
    virtual void visit(NIdentifier* nIdentifier) = 0;
    virtual void visit(NIfExpression* nIfExpression) = 0;
    virtual void visit(NBinaryOperator* nBinaryOperator) = 0;
    virtual void visit(NAssignment* nAssignment) = 0;
    virtual void visit(NBlock* nBlock) = 0;
    virtual void visit(NExpression* nExpression) { };
    virtual void visit(NExpressionStatement* nExpressionStatement) = 0;
    virtual void visit(NVariableDeclaration* nVariableDeclaration) = 0;
    virtual void visit(NFunctionDeclaration* nFunctionDeclaration) = 0;
    virtual void visit(NMethodCall* nMethodCall) = 0;
};

class GraphVisitor : public Visitor {
public:
    virtual void visit(NInteger* nInteger);
    virtual void visit(NBool* nBool);
    virtual void visit(NDouble* nDouble);
    virtual void visit(NType* nType);
    virtual void visit(NSecurity* nSecurity);
    virtual void visit(NIdentifier* nIdentifier);
    virtual void visit(NIfExpression* nIfExpression);
    virtual void visit(NBinaryOperator* nBinaryOperator);
    virtual void visit(NAssignment* nAssignment);
    virtual void visit(NBlock* nBlock);
    virtual void visit(NExpressionStatement* nExpressionStatement);
    virtual void visit(NVariableDeclaration* nVariableDeclaration);
    virtual void visit(NFunctionDeclaration* nFunctionDeclaration);
    virtual void visit(NMethodCall* nMethodCall);
};

#endif // __VISITOR_H_
