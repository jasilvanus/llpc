/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2017-2022 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 **********************************************************************************************************************/
/**
 ***********************************************************************************************************************
 * @file  PatchImageOpCollect.h
 * @brief LLPC header file: contains declaration of class lgc::PatchImageOpCollect.
 ***********************************************************************************************************************
 */
#pragma once

#include "lgc/state/PipelineState.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace lgc {

// =====================================================================================================================
// Represents the pass of LLVM patching operations for image operations
class PatchImageOpCollect : public llvm::PassInfoMixin<PatchImageOpCollect> {
public:
  llvm::PreservedAnalyses run(llvm::Module &module, llvm::ModuleAnalysisManager &analysisManager);

  bool runImpl(llvm::Module &module, PipelineState *pipelineState);

  static llvm::StringRef name() { return "Patch LLVM for image operation collecting"; }
};

// =====================================================================================================================
// Represents the pass of LLVM patching operations for image operations
class LegacyPatchImageOpCollect : public llvm::ModulePass {
public:
  LegacyPatchImageOpCollect();

  void getAnalysisUsage(llvm::AnalysisUsage &analysisUsage) const override;

  bool runOnModule(llvm::Module &module) override;

  static char ID; // NOLINT: ID of this pass

private:
  LegacyPatchImageOpCollect(const LegacyPatchImageOpCollect &) = delete;
  LegacyPatchImageOpCollect &operator=(const LegacyPatchImageOpCollect &) = delete;

  PatchImageOpCollect m_impl;
};

} // namespace lgc
