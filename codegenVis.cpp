#include "node.h"
#include "codegenVis.h"
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

static llvm::IRBuilder<> Builder(getGlobalContext()); // TODO: Get rid of this global

void CodeGenVisitor::init()
{
  if (verbose) std::cout << "CodeGenVis::init()" << std::endl;
  Module* m = new Module("main", getGlobalContext());
  Scope* s = new Scope();
  context = new CodeGenContext(s, m);
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
}

void CodeGenVisitor::generateCode()
{
  assert(vals.size() == 0);
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
GenericValue CodeGenVisitor::runCode()
{
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

void CodeGenVisitor::visit(NInteger* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
  vals.push_front(ConstantInt::get(Type::getInt64Ty(getGlobalContext()), element->value, true));
}

void CodeGenVisitor::visit(NBool* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
  if (element->value.compare("true") == 0)
	  vals.push_front(ConstantInt::getTrue(getGlobalContext()));
  assert (element->value.compare("false") == 0);
	vals.push_front(ConstantInt::getFalse(getGlobalContext()));
}

void CodeGenVisitor::visit(NDouble* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
  vals.push_front(ConstantFP::get(Type::getDoubleTy(getGlobalContext()), element->value));
}

void CodeGenVisitor::visit(NType* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
  // Do nothing
}

void CodeGenVisitor::visit(NSecurity* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
  // Do nothing
}

void CodeGenVisitor::visit(NIdentifier* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
	if (context->scope->LookUp(element->name) == NULL) {
		std::cerr << "undeclared variable " << element->name << std::endl;
    assert(0);
    // TODO
		//return NULL;
	}
	vals.push_front(new LoadInst(context->scope->LookUp(element->name)->value, 
                               "", false, Builder.GetInsertBlock()));
}

void CodeGenVisitor::visit(NIfExpression* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
  switch (flag)
  {
    case V_FLAG_IF_GUARD | V_FLAG_EXIT:
      {
        if (verbose) std::cout << "CodeGenVisitor enter " << typeid(element).name() << std::endl;
        Value *CondV = vals.front();
        vals.pop_front(); // Pop guard
        if (CondV == NULL) {
          assert(0);
          // TODO
          //return NULL;
        }
        If* myIf = new If();
        myIf->function = Builder.GetInsertBlock()->getParent();
        // Create blocks for the then and else cases.  Insert the 'then' block at the
        // end of the function.
        myIf->thenBB = BasicBlock::Create(getGlobalContext(), "if.then", myIf->function);
        myIf->elseBB = BasicBlock::Create(getGlobalContext(), "if.else");
        myIf->mergeBB = BasicBlock::Create(getGlobalContext(), "if.end");
        Builder.CreateCondBr(CondV, myIf->thenBB, myIf->elseBB);
        ifs.push_front(myIf);
      }
      break;
    case V_FLAG_IF_THEN | V_FLAG_ENTER:
      if (verbose) std::cout << "CodeGenVisitor then-enter " << typeid(element).name() << std::endl;
      // Emit then block.
      Builder.SetInsertPoint(ifs.front()->thenBB);
      break;
    case V_FLAG_IF_THEN | V_FLAG_EXIT:
      if (verbose) std::cout << "CodeGenVisitor then-exit " << typeid(element).name() << std::endl;
      Builder.CreateBr(ifs.front()->mergeBB);
      break;
    case V_FLAG_IF_ELSE | V_FLAG_ENTER:
      if (verbose) std::cout << "CodeGenVisitor else-enter " << typeid(element).name() << std::endl;
      // Emit else block.
      ifs.front()->function->getBasicBlockList().push_back(ifs.front()->elseBB);
      Builder.SetInsertPoint(ifs.front()->elseBB);
      break;
    case V_FLAG_IF_ELSE | V_FLAG_EXIT:
      if (verbose) std::cout << "CodeGenVisitor else-exit " << typeid(element).name() << std::endl;
      Builder.CreateBr(ifs.front()->mergeBB);
      break;
    case V_FLAG_EXIT:
      if (verbose) std::cout << "CodeGenVisitor exit " << typeid(element).name() << std::endl;
      // Emit merge block.
      ifs.front()->function->getBasicBlockList().push_back(ifs.front()->mergeBB);
      Builder.SetInsertPoint(ifs.front()->mergeBB);
      {
        If* myIf = ifs.front();
        delete myIf;
      }
      ifs.pop_front();
      break;
    default:
      return;
  }
  // No need to add anything to vals
}

void CodeGenVisitor::visit(NBinaryOperator* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
	Instruction::BinaryOps binstr;
	Instruction::OtherOps oinstr;
  CmpInst::Predicate pred;
  Value* rhsv = vals.front();
  vals.pop_front(); 
  Value* lhsv = vals.front();
  vals.pop_front(); 

	switch (element->op) {
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
	  switch (element->op) {
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
	  switch (element->op) {
      case TCEQ:    pred = CmpInst::ICMP_EQ; goto comp;
      case TCNE:    pred = CmpInst::ICMP_NE; goto comp;
      case TCLT:    pred = CmpInst::ICMP_SLT; goto comp;
      case TCLE:    pred = CmpInst::ICMP_SLE; goto comp;
      case TCGT:    pred = CmpInst::ICMP_SGT; goto comp;
      case TCGE:    pred = CmpInst::ICMP_SGE; goto comp;
    }
  }

comp:
	vals.push_front(CmpInst::Create(oinstr, pred, lhsv, rhsv, "", Builder.GetInsertBlock()));
  return;

math:
	vals.push_front(BinaryOperator::Create(binstr, lhsv, rhsv, "", Builder.GetInsertBlock()));
  return;
}

void CodeGenVisitor::visit(NAssignment* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
	if (context->scope->LookUp(element->lhs.name) == NULL) {
		std::cerr << "undeclared variable " << element->lhs.name << std::endl;
    assert(0);
    // TODO
		//return NULL;
	}
  Value* rhsv = vals.front();
  vals.pop_front();
  // No need to add StoreInst to vals
  new StoreInst(rhsv,
                context->scope->LookUp(element->lhs.name)->value, 
                false, Builder.GetInsertBlock());
}

void CodeGenVisitor::visit(NBlock* element, uint64_t flag)
{
  //static int size_on_entering = 0;
  //static int size_on_leaving = 0;
  switch (flag)
  {
    case V_FLAG_ENTER:
      if (verbose) std::cout << "CodeGenVisitor entering " << typeid(element).name() << std::endl;
      //size_on_entering = vals.size();
      //std::cout << "Size on entering: " << size_on_entering << std::endl;;
      context->scope->InitializeScope();
      break;
    case V_FLAG_EXIT:
      if (verbose) std::cout << "CodeGenVisitor leaving " << typeid(element).name() << std::endl;
      //size_on_leaving = vals.size();
      //std::cout << "Size on leaving: " << size_on_leaving << std::endl;;
      context->scope->FinalizeScope();
      break;
    default:
      assert(0);
  }
}

void CodeGenVisitor::visit(NExpressionStatement* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
}

void CodeGenVisitor::visit(NVariableDeclaration* element, uint64_t flag)
{
  if (verbose) std::cout << "CodeGenVisitor " << typeid(element).name() << std::endl;
	AllocaInst *alloc = new AllocaInst( (llvm::Type *) typeOf(element->type), 
                                       element->id.name.c_str(), Builder.GetInsertBlock());
  Symbol* sym = new Symbol(alloc, new SType());
  context->scope->Insert(element->id.name, sym);
  // No need to add alloc to vals
}
