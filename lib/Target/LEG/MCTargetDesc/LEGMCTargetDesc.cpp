//===-- LEGMCTargetDesc.cpp - LEG Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides LEG specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "LEGMCTargetDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "LEGGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LEGGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "LEGGenRegisterInfo.inc"

using namespace llvm;

// Force static initialization.
extern "C" void LLVMInitializeLEGTargetMC() {
}
