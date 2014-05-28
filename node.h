#ifndef __NODE_H_
#define __NODE_H_
#include "visitor.h"
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

class Visitor;

class Node {
public:
    int lineno;
    virtual ~Node() {}
    virtual llvm::Value* codeGen(Scope* scope) { }
    virtual SType* typeCheck(Scope* scope) { }
    virtual void accept(class Visitor &visitor) { }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) {
      StatementList::const_iterator it;
	    for (it = statements.begin(); it != statements.end(); it++) {
		    (**it).accept(visitor);
	    }
      visitor.visit(this);
    };
};

class NInteger : public NExpression {
public:
    long long value;
    NInteger(long long value) : value(value) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};

class NBool : public NExpression {
public:
    std::string value;
    NBool(std::string value) : value(value) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};

class NDouble : public NExpression {
public:
    double value;
    NDouble(double value) : value(value) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};

class NType : public NExpression {
public:
    std::string name;
    NType(const std::string& name) : name(name) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};

class NSecurity : public NExpression {
public:
    std::string name;
    NSecurity(const std::string& name) : name(name) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};

class NIdentifier : public NExpression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};

class NIfExpression : public NExpression {
public:
    NExpression& iguard;
    NBlock & ithen;
    NBlock & ielse;
    NIfExpression(NExpression& iguard, NBlock& ithen, NBlock& ielse) :
        iguard(iguard), ithen(ithen), ielse(ielse) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) {
      visitor.visit(this);
      iguard.accept(visitor);
      ithen.accept(visitor);
      ielse.accept(visitor);
    };
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
        lhs(lhs), rhs(rhs), op(op) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) {
      visitor.visit(this);
      lhs.accept(visitor);
      rhs.accept(visitor);
    };
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) : 
        lhs(lhs), rhs(rhs) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) {
      visitor.visit(this);
      rhs.accept(visitor);
    };
};

// TODO: Why even have NExpressionStatement?!
class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) : 
        expression(expression) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) {
      visitor.visit(this);
      expression.accept(visitor);
    };
};

class NVariableDeclaration : public NStatement {
public:
    const NType& type;
    NSecurity& security;
    NIdentifier& id;
    NExpression *assignmentExpr;
    NVariableDeclaration(const NType& type, NIdentifier& id, NSecurity& sec) :
        type(type), id(id), security(sec) { }
    NVariableDeclaration(const NType& type, NIdentifier& id, NExpression *assignmentExpr, NSecurity& sec) :
        type(type), id(id), assignmentExpr(assignmentExpr), security(sec) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) {
      visitor.visit(this);
      ((NType&)type).accept(visitor);
      security.accept(visitor);
	    if (assignmentExpr != NULL) {
		    NAssignment assign(id, *assignmentExpr);
        assign.accept(visitor);
      }
    };
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
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }
    NMethodCall(const NIdentifier& id) : id(id) { }
    virtual llvm::Value* codeGen(Scope* scope);
    virtual SType* typeCheck(Scope* scope);
    virtual void accept(Visitor &visitor) { visitor.visit(this); };
};
#endif // __NODE_H_
