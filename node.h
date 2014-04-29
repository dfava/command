#ifndef __NODE_H_
#define __NODE_H_
#include "codegen.h"
#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class Node {
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(Scope* scope) { }
    virtual llvm::Type* typeCheck(Scope* scope) { }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
public:
    long long value;
    NInteger(long long value) : value(value) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NBool : public NExpression {
public:
    std::string value;
    NBool(std::string value) : value(value) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NDouble : public NExpression {
public:
    double value;
    NDouble(double value) : value(value) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NType : public NExpression {
public:
    std::string name;
    NType(const std::string& name) : name(name) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NIdentifier : public NExpression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NIfStatement : public NStatement {
public:
    NExpression& iguard;
    NStatement & ithen;
    NStatement & ielse;
    NIfStatement(NExpression& iguard, NStatement& ithen, NStatement& ielse) :
        iguard(iguard), ithen(ithen), ielse(ielse) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }
    NMethodCall(const NIdentifier& id) : id(id) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
        lhs(lhs), rhs(rhs), op(op) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) : 
        lhs(lhs), rhs(rhs) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) : 
        expression(expression) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NVariableDeclaration : public NStatement {
public:
    const NType& type;
    NIdentifier& id;
    NExpression *assignmentExpr;
    NVariableDeclaration(const NType& type, NIdentifier& id) :
        type(type), id(id) { }
    NVariableDeclaration(const NType& type, NIdentifier& id, NExpression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};

class NFunctionDeclaration : public NStatement {
public:
    const NType& type;
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NType& type, const NIdentifier& id, 
            const VariableList& arguments, NBlock& block) :
        type(type), id(id), arguments(arguments), block(block) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual llvm::Type* typeCheck(Scope* scope);
};
#endif // __NODE_H_
