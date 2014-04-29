#ifndef __SCOPE_H_
#define __SCOPE_H_
#include <list>
#include <map>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

class SymbolTable {
private:
  std::string name;
  std::map<std::string, llvm::Value*> locals;

public:
  SymbolTable(std::string name) : name(name) { }
  void Insert(std::string name, llvm::Value* value) { locals[name] = value; }
  llvm::Value* LookUp(std::string name) {
    if (locals.find(name) == locals.end()) {
      return NULL;
    }
    return locals[name];
  }
};

class Scope {
private:
  std::list<SymbolTable*> scope;

public:
  Scope() { }
  int depth() { return scope.size(); }
  void InitializeScope(std::string name) {
    scope.push_front(new SymbolTable(name));
  }
  void FinalizeScope() {
    SymbolTable* toDelete = scope.front();
    scope.pop_front();
    delete toDelete;
  }
  void Insert(std::string name, llvm::Value* value) { scope.front()->Insert(name, value); }
  llvm::Value* LookUp(std::string name) {
    for (std::list<SymbolTable*>::iterator it=scope.begin(); it != scope.end(); ++it)
    {
      llvm::Value* val = (*it)->LookUp(name);
      if (val != NULL) return val;
    }
    return NULL;
  }
};
#endif // __SCOPE_H_
