#ifndef __SCOPE_H_
#define __SCOPE_H_
#include <list>
#include <map>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

class SType {
public:
  llvm::Type* type;
  std::string sec;
  SType(llvm::Type* type, std::string sec) : type(type), sec(sec) { }
  SType() : type(NULL), sec("") { }
};

class Symbol {
public:
  llvm::Value* value;
  //llvm::Type* type;
  SType* stype;
  //Symbol(llvm::Value* value, llvm::Type* type) : value(value), type(type) { }
  Symbol(llvm::Value* value, SType* stype) : value(value), stype(stype) { }
};

class SymbolTable {
private:
  std::string name;
  std::map<std::string, Symbol*> locals;

public:
  SymbolTable(std::string name) : name(name) { }
  ~SymbolTable() {
    for (std::map<std::string, Symbol*>::iterator it=locals.begin(); it != locals.end(); ++it) {
      delete it->second;
    }
  }
  void Insert(std::string name, Symbol* sym) { locals[name] = sym; }
  Symbol* LookUp(std::string name) {
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
  void Insert(std::string name, Symbol* sym) { scope.front()->Insert(name, sym); }
  Symbol* LookUp(std::string name) {
    for (std::list<SymbolTable*>::iterator it=scope.begin(); it != scope.end(); ++it)
    {
      Symbol* sym = (*it)->LookUp(name);
      if (sym != NULL) return sym;
    }
    return NULL;
  }
};
#endif // __SCOPE_H_
