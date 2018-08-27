#include "Bulldog.h"

namespace bulldog {

bool IRPrinter::runOnFunction(Function &F) {
  errs() << "Emit assembly for ";
  errs().write_escaped(F.getName()) << '\n';
  EmitFunction(F);
  errs() << ss.str() << '\n';
  return false;
}

void IRPrinter::EmitFunction(Function &func) {
  for (auto &bb : func) {
    EmitBasicBlock(bb);
  }
}

void IRPrinter::EmitBasicBlock(BasicBlock &bb) {
  int id = GetBlockId(&bb);
  ss << bb.getName().str() << ":\n";
  for (auto &inst : bb) {
    EmitInstruction(inst);
  }
}

void IRPrinter::EmitInstruction(Instruction &inst) {
  Instruction *ip = &inst;
  if (inst.isBinaryOp()) {
    switch(inst.getOpcode()) {
      case Instruction::Add:
        ss << "add ";
        break;
      case Instruction::Mul:
        ss << "mul ";
        break;
      case Instruction::FAdd:
        ss << "fadd ";
        break;
      default:
        ss << "unknown ";
    }
  } else if (PHINode *phi = dyn_cast<PHINode>(ip)) {
    ss << "phi ";
  } else if (inst.isTerminator()) {
  }
  ss << ";\n";
  errs() << inst << "\n";
}

}  // namespace bulldog

char bulldog::IRPrinter::ID = 0;
static RegisterPass<bulldog::IRPrinter> X("print-bulldog", "Hello World Pass", /*CFGOnly=*/false , /*is_analysis*/false);
