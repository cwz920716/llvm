#ifndef BULLDOG_H
#define BULLDOG_H

#include <map>
#include <sstream>

#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

using namespace llvm;

namespace bulldog {

class IRPrinter : public FunctionPass {
 public:
  static char ID;
  IRPrinter() : FunctionPass(ID), num_bbs(0), num_values(0) {}

  bool runOnFunction(Function &F) override;

  virtual void getAnalysisUsage(AnalysisUsage& AU) const override;

 private:
  void EmitFunction(Function &func);
  void EmitBasicBlock(BasicBlock &bb);
  void EmitInstruction(Instruction &inst);

  int GetValueId(Value *v) {
    if (value_id_map.find(v) == value_id_map.end()) {
      value_id_map[v] = num_values;
      num_values++;
    }

    return value_id_map[v];
  }

  std::string GetOperand(Value *v) {
    if (ConstantInt *ci = dyn_cast<ConstantInt>(v)) {
      return ci->getValue().toString(10, true);
    } else if (ConstantFP *cf = dyn_cast<ConstantFP>(v)) {
      return std::to_string(cf->getValueAPF().convertToFloat());
    } else {
      int id = GetValueId(v);
      return "t" + std::to_string(id);
    }
  }

  int GetBlockId(BasicBlock *bb) {
    if (bb_id_map.find(bb) == bb_id_map.end()) {
      bb_id_map[bb] = num_bbs;
      num_bbs++;
    }

    return bb_id_map[bb];
  }

  std::stringstream ss;
  int num_bbs;
  int num_values;
  std::map<Value *, int> value_id_map;
  std::map<BasicBlock *, int> bb_id_map;
  std::map<BasicBlock *, BasicBlock *> loop_header_map;
};

}  // namespace bulldog

#endif
