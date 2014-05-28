#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm-c/BitWriter.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

extern CodeGen codegen;

static IRBuilder<> Builder(getGlobalContext());
void CodeGen::init()
{
  Module* m = new Module("main", getGlobalContext());
  Scope* s = new Scope();
  context = new CodeGenContext(s, m);
}

/* Compile the AST into a module */
void CodeGen::generateCode(NBlock& root)
{
  if (verbose) std::cout << "Generating code...\n";
  // Create a global scope
  context->scope->InitializeScope("global");
  // Create a main function and add the first basic block to it
  ArrayRef<llvm::Type *> argTypes;
	//FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
	FunctionType *ftype = FunctionType::get(Type::getInt32Ty(getGlobalContext()), argTypes, false);
	mainFunction = Function::Create(ftype, GlobalValue::ExternalLinkage, "main", context->module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
  // Set the Builder's basic block to this first block from the main function
  Builder.SetInsertPoint(bblock);
  // Traverse AST and generate code
  root.codeGen(context->scope);
  //Builder.CreateRetVoid();
  Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0, true));
  // Cleanup scopes
  assert(context->scope->depth() == 1);
  context->scope->FinalizeScope();
  // Validate the generated code, checking for consistency.
  verifyFunction(*mainFunction);
  // Dump IR to screen
	if (verbose) std::cout << "Code is generated." << std::endl;
  context->module->dump();
  if (filename != NULL) {
    LLVMWriteBitcodeToFile((LLVMOpaqueModule *)context->module, filename);
  }
}

/* Executes the AST by running the main function */
GenericValue CodeGen::runCode() {
	if (verbose) std::cout << "Running code...\n";
  InitializeNativeTarget();
  ExecutionEngine* ee = EngineBuilder(context->module).create();
  assert(ee != 0);
	std::vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	if (verbose) std::cout << "Code was run.\n";
  return v;
}

/* Returns an LLVM type based on the identifier */
static const Type *typeOf(const NType& type) 
{
	if (type.name.compare("int") == 0) {
		return Type::getInt64Ty(getGlobalContext());
	}
	else if (type.name.compare("double") == 0) {
		return Type::getDoubleTy(getGlobalContext());
	}
  else if (type.name.compare("bool") == 0) {
		return Type::getInt1Ty(getGlobalContext());
  }
	return Type::getVoidTy(getGlobalContext());
}

/* -- Code Generation -- */

Value* NInteger::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating integer: " << value << std::endl;
	return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

Value* NDouble::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating double: " << value << std::endl;
	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* NBool::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating boolean: " << value << std::endl;
  if (value.compare("true") == 0)
	  return ConstantInt::getTrue(getGlobalContext());
  assert (value.compare("false") == 0);
	return ConstantInt::getFalse(getGlobalContext());
}

Value* NType::codeGen(Scope* scope)
{
  assert(0);
}

Value* NSecurity::codeGen(Scope* scope)
{
  assert(0);
}

Value* NIdentifier::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating identifier reference: " << name << std::endl;
	if (scope->LookUp(name) == NULL) {
		std::cerr << "undeclared variable " << name << std::endl;
		return NULL;
	}
	return new LoadInst(scope->LookUp(name)->value, "", false, Builder.GetInsertBlock());
}

Value* NAssignment::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating assignment for " << lhs.name << std::endl;
	if (scope->LookUp(lhs.name) == NULL) {
		std::cerr << "undeclared variable " << lhs.name << std::endl;
		return NULL;
	}
	return new StoreInst(rhs.codeGen(scope), scope->LookUp(lhs.name)->value, false, Builder.GetInsertBlock());
}

Value* NVariableDeclaration::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating variable declaration " << type.name << " " << id.name << std::endl;
	AllocaInst *alloc = new AllocaInst( (llvm::Type *) typeOf(type), id.name.c_str(), Builder.GetInsertBlock());
  Symbol* sym = new Symbol(alloc, new SType());
  scope->Insert(id.name, sym);
	if (assignmentExpr != NULL) {
		NAssignment assign(id, *assignmentExpr);
		assign.codeGen(scope);
  }
	return alloc;
}

Value* NBinaryOperator::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating binary operation " << op << std::endl;
	Instruction::BinaryOps binstr;
	Instruction::OtherOps oinstr;
  CmpInst::Predicate pred;
  Value* lhsv = lhs.codeGen(scope);

	switch (op) {
    // Arith instructions
		case TPLUS: 	binstr = Instruction::Add; goto math;
		case TMINUS: 	binstr = Instruction::Sub; goto math;
		case TMUL: 		binstr = Instruction::Mul; goto math;
		case TDIV: 		binstr = Instruction::SDiv; goto math;
  }
  
  // For now, we assume that if the type checker passed,
  // both lhs and rhs have the exact same value.
  // So we don't promote the int 1 to a float 1.0,
  // though we probably should
  if (lhsv->getType() == Type::getDoubleTy(getGlobalContext())) {
    // Comparision instructions, doubles
    oinstr = Instruction::FCmp; 
	  switch (op) {
      case TCEQ:    pred = CmpInst::FCMP_UEQ; goto comp;
      case TCNE:    pred = CmpInst::FCMP_UNE; goto comp;
      case TCLT:    pred = CmpInst::FCMP_ULT; goto comp;
      case TCLE:    pred = CmpInst::FCMP_ULE; goto comp;
      case TCGT:    pred = CmpInst::FCMP_UGT; goto comp;
      case TCGE:    pred = CmpInst::FCMP_UGE; goto comp;
    }
  } else {
    // Comparison instructions, int
    oinstr = Instruction::ICmp; 
	  switch (op) {
      case TCEQ:    pred = CmpInst::ICMP_EQ; goto comp;
      case TCNE:    pred = CmpInst::ICMP_NE; goto comp;
      case TCLT:    pred = CmpInst::ICMP_SLT; goto comp;
      case TCLE:    pred = CmpInst::ICMP_SLE; goto comp;
      case TCGT:    pred = CmpInst::ICMP_SGT; goto comp;
      case TCGE:    pred = CmpInst::ICMP_SGE; goto comp;
    }
  }

comp:
	return CmpInst::Create(oinstr, pred, lhsv,
    rhs.codeGen(scope), "", Builder.GetInsertBlock());

math:
	return BinaryOperator::Create(binstr, lhsv,
		rhs.codeGen(scope), "", Builder.GetInsertBlock());
}

Value* NIfExpression::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Generating code for if-then-else" << std::endl;

  Value *CondV = iguard.codeGen(scope);
  if (CondV == NULL) return NULL;
  
  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  
  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "if.then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "if.else");
  BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "if.end");
  
  Builder.CreateCondBr(CondV, ThenBB, ElseBB);
  
  // Emit then value.
  Builder.SetInsertPoint(ThenBB);
  
  scope->InitializeScope("branch", "");
  Value *ThenV = ithen.codeGen(scope);
  scope->FinalizeScope();
  if (ThenV == NULL) return NULL;
  
  Builder.CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = Builder.GetInsertBlock();
  
  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);
  
  scope->InitializeScope("branch", "");
  Value *ElseV = ielse.codeGen(scope);
  scope->FinalizeScope();
  if (ElseV == NULL) return NULL;
  
  Builder.CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = Builder.GetInsertBlock();
  
  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);
  return ElseV;
}

Value* NExpressionStatement::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Generating code for " << typeid(expression).name() << std::endl;
	return expression.codeGen(scope);
}

Value* NBlock::codeGen(Scope* scope)
{
	if (codegen.getVerbose()) std::cout << "Creating block" << std::endl;
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		if (codegen.getVerbose()) std::cout << "Generating code for " << typeid(**it).name() << std::endl;
		last = (**it).codeGen(scope);
	}
  // Since we are using NULL for error conditions,
  // And since a block can be empty,
  // Then we need to distinguish the NULL for error condition
  // from the NULL for an empty block
  // Therefore, for empty blocks we return nop instead
	if (statements.begin() == statements.end()) {
    return new BitCastInst(Constant::getNullValue(
        Type::getInt1Ty(getGlobalContext())), 
        Type::getInt1Ty(getGlobalContext()), "", Builder.GetInsertBlock());
  }
	return last;
}

Value* NMethodCall::codeGen(Scope* scope)
{
  // TODO: CONTEXT
  /*
	Function *function = TheModule->getFunction(id.name.c_str());
	if (function == NULL) {
		std::cerr << "no such function " << id.name << std::endl;
	}
	std::vector<Value*> args;
	ExpressionList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		args.push_back((**it).codeGen(st));
	}
	CallInst *call = CallInst::Create(function, args, "", context.currentBlock());
	if (codegen.getVerbose()) std::cout << "Creating method call: " << id.name << std::endl;
	return call;
  //*/
  return NULL;
}

// TODO: Treat argument list as "NType NIdentifier, ...." 
// as opposed to "NTIdentifier NIdentifier, ...."
Value* NFunctionDeclaration::codeGen(Scope* scope)
{
  // TODO: CONTEXT
  /*
  SmallVector<llvm::Type *, 100> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		argTypes.push_back((llvm::Type*)typeOf((**it).type));
	}
	FunctionType *ftype = FunctionType::get((llvm::Type *)typeOf(type), argTypes, false);
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), TheModule);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);

	context.pushBlock(bblock);

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).codeGen(st);
	}
	
	block.codeGen(st);
	ReturnInst::Create(getGlobalContext(), bblock);

	context.popBlock();
	if (codegen.getVerbose()) std::cout << "Creating function: " << id.name << std::endl;
	return function;
  //*/
  return NULL;
}
