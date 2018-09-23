#include "Bulldog.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Analysis/LoopInfo.h"

namespace bulldog {

void IRPrinter::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.setPreservesCFG();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool IRPrinter::runOnFunction(Function &F) {
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  for (auto &bb : F) {
    auto loop = LI.getLoopFor(&bb);
    if (loop != nullptr && loop->getLoopDepth() > 0) {
      auto header = loop->getHeader();
      loop_header_map[&bb] = header;
      errs() << bb.getName() << " inside loop " << header->getName() << "\n";
    }
  }
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
  // int id = GetBlockId(&bb);
  auto bbp = &bb;
  if (loop_header_map[bbp] != nullptr) {
    ss << "%" << loop_header_map[bbp]->getName().str() << " ";
  }
  ss << bb.getName().str() << ":\n";
  for (auto &inst : bb) {
    EmitInstruction(inst);
  }
}

void IRPrinter::EmitInstruction(Instruction &inst) {
  Instruction *ip = &inst;
  if (inst.isBinaryOp()) {
    ss << GetOperand(ip) << " <- ";
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
    ss << GetOperand(ip->getOperand(0)) << ", ";
    ss << GetOperand(ip->getOperand(1));
  } else if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(ip)) {
    ss << GetOperand(ip) << " <- ";
    ss << "vaddr ";
    ss << GetOperand(gep->getPointerOperand()) << "";
    for (int i = 0; i < gep->getNumIndices(); i++) {
      ss << "[" << GetOperand(ip->getOperand(i + 1)) << "]";
    }
  } else if (AllocaInst *alloca = dyn_cast<AllocaInst>(ip)) {
    ss << GetOperand(ip) << " <- ";
    ss << "alloca ";
    // errs() << *alloca->getType()->getElementType() << "\n";
    auto type = alloca->getType()->getElementType();
    ss << "[" << GetOperand(alloca->getArraySize()) << "]";
    if (type->isArrayTy()) {
      ss << "[" << type->getArrayNumElements() << "]";
    }
    // ss << alloca->getType()->getElementType()->getVectorNumElements();
  } else if (StoreInst *st = dyn_cast<StoreInst>(ip)) {
    ss << "store ";
    ss << GetOperand(st->getPointerOperand()) << ", ";
    ss << GetOperand(st->getValueOperand());
  } else if (LoadInst *ld = dyn_cast<LoadInst>(ip)) {
    ss << GetOperand(ip) << " <- ";
    ss << "load ";
    ss << GetOperand(ld->getPointerOperand()) << ", ";
  } else if (PHINode *phi = dyn_cast<PHINode>(ip)) {
    ss << GetOperand(ip) << " <- ";
    ss << "phi { ";
    for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
      ss << phi->getIncomingBlock(i)->getName().str() << " : ";
      ss << GetOperand(phi->getIncomingValue(i)) << " , ";
    }
    ss << "}";
  } else if (CmpInst *cmp = dyn_cast<CmpInst>(ip)) {
    ss << GetOperand(ip) << " <- ";
    if (FCmpInst *fcmp = dyn_cast<FCmpInst>(ip)) {
      assert(fcmp != nullptr);
      ss << "fcmp ";
    } else {
      // ss << "cmp ";
    }
    switch (cmp->getPredicate()) {
      case ICmpInst::ICMP_NE:
        ss << "ne ";
        break;
      case ICmpInst::ICMP_SLT:
        ss << "slt ";
        break;
      default:
        ss << " <? ";
    }
    ss << GetOperand(ip->getOperand(0)) << ", ";
    ss << GetOperand(ip->getOperand(1));
  } else if (CastInst *cast = dyn_cast<CastInst>(ip)) {
    ss << GetOperand(ip) << " <- ";
    if (cast->isIntegerCast()) {
      ss << "mov ";
    } else {
      if (cast->getSrcTy()->isFloatingPointTy()) {
        ss << "fcast ";
      } else {
        ss << "icast ";
      }
    }
    ss << GetOperand(ip->getOperand(0));
  } else if (inst.isTerminator()) {
    if (BranchInst *br = dyn_cast<BranchInst>(ip)) {
      if (br->isConditional()) {
        ss << "if <- " << GetOperand(br->getCondition()) << ", " << br->getSuccessor(0)->getName().str();
        ss << ", " << br->getSuccessor(1)->getName().str();
      } else {
        ss << "goto " << br->getSuccessor(0)->getName().str();
      }
    } else if (ReturnInst *ret = dyn_cast<ReturnInst>(ip)) {
      ss << "ret " << GetOperand(ret->getReturnValue());
    } else {
      ss << "err>terminator";
    }
  } else {
    ss << "err>";
  }
  ss << ";\n";
  // errs() << inst << "\n";
}

}  // namespace bulldog

char bulldog::IRPrinter::ID = 0;
static RegisterPass<bulldog::IRPrinter> X("print-bulldog", "Bulldog frontend Pass", /*CFGOnly=*/false , /*is_analysis*/false);
