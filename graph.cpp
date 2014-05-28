#include "node.h"
#include "visitor.h"

void GraphVisitor::visit(NInteger* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NBool* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NDouble* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NType* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NSecurity* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NIdentifier* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NIfExpression* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NBinaryOperator* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NAssignment* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NBlock* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NExpressionStatement* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NVariableDeclaration* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NFunctionDeclaration* element)
{
  std::cout << typeid(element).name() << std::endl;
}

void GraphVisitor::visit(NMethodCall* element)
{
  std::cout << typeid(element).name() << std::endl;
}
