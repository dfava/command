#ifndef __NODE_H_
#define __NODE_H_
#include "scope.h"
#include "visitor.h"
#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

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
    virtual void accept(Visitor &visitor) {
      visitor.visit(this, V_FLAG_ENTER);
      StatementList::const_iterator it;
	    for (it = statements.begin(); it != statements.end(); it++) {
		    (**it).accept(visitor);
	    }
      visitor.visit(this, V_FLAG_EXIT);
    };
};

class NSkip : public NExpression {
public:
    NSkip () { }
    virtual void accept(Visitor &visitor) { visitor.visit(this, V_FLAG_NONE); };
};

class NInteger : public NExpression {
public:
    long long value;
    NInteger(long long value) : value(value) { }
    virtual void accept(Visitor &visitor) { visitor.visit(this, V_FLAG_NONE); };
};

class NBool : public NExpression {
public:
    std::string value;
    NBool(std::string value) : value(value) { }
    virtual void accept(Visitor &visitor) { visitor.visit(this, V_FLAG_NONE); };
};

class NDouble : public NExpression {
public:
    double value;
    NDouble(double value) : value(value) { }
    virtual void accept(Visitor &visitor) { visitor.visit(this, V_FLAG_NONE); };
};

class NType : public NExpression {
public:
    std::string name;
    NType(const std::string& name) : name(name) { }
    virtual void accept(Visitor &visitor) { visitor.visit(this, V_FLAG_NONE); };
};

class NSecurity : public NExpression {
public:
    std::string name;
    NSecurity(const std::string& name) : name(name) { }
    virtual void accept(Visitor &visitor) { visitor.visit(this, V_FLAG_NONE); };
};

class NIdentifier : public NExpression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name) { }
    virtual void accept(Visitor &visitor) { visitor.visit(this, V_FLAG_NONE); };
};

class NIfExpression : public NExpression {
public:
    NExpression& iguard;
    NBlock & ithen;
    NBlock & ielse;
    NIfExpression(NExpression& iguard, NBlock& ithen, NBlock& ielse) :
        iguard(iguard), ithen(ithen), ielse(ielse) { }
    virtual void accept(Visitor &visitor) {
      visitor.visit(this, V_FLAG_ENTER);

      visitor.visit(this, V_FLAG_IF_GUARD | V_FLAG_ENTER);
      iguard.accept(visitor);
      visitor.visit(this, V_FLAG_IF_GUARD | V_FLAG_EXIT);

      visitor.visit(this, V_FLAG_IF_THEN | V_FLAG_ENTER);
      ithen.accept(visitor);
      visitor.visit(this, V_FLAG_IF_THEN | V_FLAG_EXIT);

      visitor.visit(this, V_FLAG_IF_ELSE | V_FLAG_ENTER);
      ielse.accept(visitor);
      visitor.visit(this, V_FLAG_IF_ELSE | V_FLAG_EXIT);

      visitor.visit(this, V_FLAG_EXIT);
    };
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
        lhs(lhs), rhs(rhs), op(op) { }
    virtual void accept(Visitor &visitor) {
      lhs.accept(visitor);
      rhs.accept(visitor);
      visitor.visit(this, V_FLAG_NONE);
    };
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) : 
        lhs(lhs), rhs(rhs) { }
    virtual void accept(Visitor &visitor) {
      rhs.accept(visitor);
      visitor.visit(this, V_FLAG_NONE);
    };
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) : 
        expression(expression) { }
    virtual void accept(Visitor &visitor) {
      expression.accept(visitor);
      visitor.visit(this, V_FLAG_NONE);
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
    virtual void accept(Visitor &visitor) {
      ((NType&)type).accept(visitor);
      security.accept(visitor);
      visitor.visit(this, V_FLAG_NONE); // Must declare the variable before assign
	    if (assignmentExpr != NULL) {
		    NAssignment assign(id, *assignmentExpr);
        assign.accept(visitor);
      }
    };
};
#endif // __NODE_H_
