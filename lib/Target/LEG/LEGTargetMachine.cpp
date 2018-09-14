//===-- LEGTargetMachine.cpp - Define TargetMachine for LEG -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "LEGTargetMachine.h"
#include "LEG.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

// Force static initialization.
extern "C" void LLVMInitializeLEGTarget() {
}
