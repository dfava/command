#ifndef __TYPECHECKER_H_
#define __TYPECHECKER_H_
#include "node.h"
class TypeChecker {
public:
  TypeChecker() { };
  bool check(NBlock& root);
};
#endif // __TYPECHECKER_H_
