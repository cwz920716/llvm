#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct LLVMHello : public FunctionPass {
  static char ID;
  LLVMHello() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "LLVM Hello: ";
    errs().write_escaped(F.getName()) << '\n';
    return false;
  }
}; // end of struct LLVMHello
}  // end of anonymous namespace

char LLVMHello::ID = 0;
static RegisterPass<LLVMHello> X("llvm-hello", "Hello World Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
