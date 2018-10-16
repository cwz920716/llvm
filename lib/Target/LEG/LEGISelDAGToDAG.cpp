//===-- LEGISelDAGToDAG.cpp - A dag to dag inst selector for LEG ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the LEG target.
//
//===----------------------------------------------------------------------===//

#include "LEG.h"
#include "LEGTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include "LEGInstrInfo.h"

using namespace llvm;

/// LEGDAGToDAGISel - LEG specific code to select LEG machine
/// instructions for SelectionDAG operations.
///
namespace {
class LEGDAGToDAGISel : public SelectionDAGISel {
  const LEGSubtarget &Subtarget;

public:
  explicit LEGDAGToDAGISel(LEGTargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(TM, OptLevel), Subtarget(*TM.getSubtargetImpl()) {}

  void Select(SDNode *N) override;

  bool SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);

  virtual StringRef getPassName() const override {
    return "LEG DAG->DAG Pattern Instruction Selection";
  }

private:
  SDNode *SelectMoveImmediate(SDNode *N);
  SDNode *SelectConditionalBranch(SDNode *N);

// Include the pieces autogenerated from the target description.
#include "LEGGenDAGISel.inc"
};
} // end anonymous namespace

bool LEGDAGToDAGISel::SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset) {
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    EVT PtrVT = getTargetLowering()->getPointerTy(CurDAG->getDataLayout());
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), PtrVT);
    Offset = CurDAG->getTargetConstant(0, Addr, MVT::i32);
    return true;
  }
  if (Addr.getOpcode() == ISD::TargetExternalSymbol ||
      Addr.getOpcode() == ISD::TargetGlobalAddress ||
      Addr.getOpcode() == ISD::TargetGlobalTLSAddress) {
    return false; // direct calls.
  }

  Base = Addr;
  Offset = CurDAG->getTargetConstant(0, Addr, MVT::i32);
  return true;
}

SDNode *LEGDAGToDAGISel::SelectMoveImmediate(SDNode *N) {
  // Make sure the immediate size is supported.
  SDLoc DL(N);
  ConstantSDNode *ConstVal = cast<ConstantSDNode>(N);
  uint64_t ImmVal = ConstVal->getZExtValue();
  errs() << "ImmVal = " << ImmVal << "\n";
  uint64_t SupportedMask = 0x0000ffff;
  if ((ImmVal & SupportedMask) == ImmVal) {
    SelectCode(N);
    return nullptr;
  }

  // Select the low part of the immediate move.
  uint64_t LoMask = 0xffff;
  uint64_t HiMask = 0xffff0000;
  uint64_t ImmLo = (ImmVal & LoMask);
  uint64_t ImmHi = (ImmVal & HiMask);
  SDValue ConstLo = CurDAG->getTargetConstant(ImmLo, DL, MVT::i32);
  MachineSDNode *Move =
      CurDAG->getMachineNode(LEG::MOVLOi16, DL, MVT::i32, ConstLo);

  // Select the low part of the immediate move, if needed.
  if (ImmHi) {
    SDValue ConstHi = CurDAG->getTargetConstant(ImmHi >> 16, DL, MVT::i32);
    Move = CurDAG->getMachineNode(LEG::MOVHIi16, DL, MVT::i32, SDValue(Move, 0),
                                  ConstHi);
  }

  ReplaceUses(SDValue(N, 0), SDValue(Move, 0));

  return Move;
}

SDNode *LEGDAGToDAGISel::SelectConditionalBranch(SDNode *N) {
  SDValue Chain = N->getOperand(0);
  SDValue Cond = N->getOperand(1);
  SDValue LHS = N->getOperand(2);
  SDValue RHS = N->getOperand(3);
  SDValue Target = N->getOperand(4);
  
  // Generate a comparison instruction.
  EVT CompareTys[] = { MVT::Other, MVT::Glue };
  SDVTList CompareVT = CurDAG->getVTList(CompareTys);
  SDValue CompareOps[] = {LHS, RHS, Chain};
  SDNode *Compare = CurDAG->getMachineNode(LEG::CMP, N, CompareVT, CompareOps);
  
  // Generate a predicated branch instruction.
  CondCodeSDNode *CC = cast<CondCodeSDNode>(Cond.getNode());
  SDValue CCVal = CurDAG->getTargetConstant(CC->get(), N, MVT::i32);
  SDValue BranchOps[] = {CCVal, Target, SDValue(Compare, 0),
                         SDValue(Compare, 1)};
  return CurDAG->getMachineNode(LEG::Bcc, N, MVT::Other, BranchOps);
}

void LEGDAGToDAGISel::Select(SDNode *N) {
  llvm::SDNode* NewNode = nullptr;
  switch (N->getOpcode()) {
  case ISD::Constant:
    NewNode = SelectMoveImmediate(N);
    return;
  case ISD::BR_CC:
    NewNode = SelectConditionalBranch(N);
    ReplaceNode(N, NewNode);
    return;
  }

  SelectCode(N);
}

/// createLEGISelDag - This pass converts a legalized DAG into a
/// LEG-specific DAG, ready for instruction scheduling.
///
FunctionPass *llvm::createLEGISelDag(LEGTargetMachine &TM,
                                     CodeGenOpt::Level OptLevel) {
  return new LEGDAGToDAGISel(TM, OptLevel);
}
