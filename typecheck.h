#ifndef __TYPECHECKER_H_
#define __TYPECHECKER_H_
#include "node.h"
#include <map>
#include <string>
class TypeChecker {
private:
  char* filename;
  std::map<int, std::string> fmap;
  bool verbose = false;
public:
  TypeChecker() { };
  bool check(NBlock& root);
  void setFileName(char* filename);
  void printErrorMessage(std::string message, int lineno);
  void setVerbose(bool v) { verbose = v; };
  bool getVerbose() { return verbose; };
};
#endif // __TYPECHECKER_H_
