/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2022 Google LLC. All Rights Reserved.
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

#include "vkgcPipelineDumper.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>

using namespace Vkgc;
using namespace llvm;

namespace Llpc {
namespace {

using PipelineDumperTest = ::testing::Test;

// The parameters to run a pipeline options test.
struct GenerateHashParams {
  bool isCacheHash;
  bool isRelocatableShader;
  UnlinkedShaderStage unlinkedShaderStage;
};

// The function type used to pass the function that given the expected result from a pipeline options tests.
using HashModifiedFunc = std::function<bool(const GenerateHashParams &params)>;

// The function type used to modify the compute pipeline build info for a pipeline options test.
using ModifyComputeBuildInfo = std::function<void(ComputePipelineBuildInfo *)>;

// The function type used to modify the graphics pipeline build info for a pipeline options test.
using ModifyGraphicsBuildInfo = std::function<void(GraphicsPipelineBuildInfo *)>;

// =====================================================================================================================
// Runs a graphics pipeline options hash test.  The test will follow these steps:
//
// 1) Get the hash for a default graphics pipeline build info using `params`.
// 2) Modify the build info using `modifyBuildInfoFunc`.
// 3) Get the hash for the modified build info using `params`.
// 4) The test passes if hashes are equal exactly when expected, and unequal otherwise.
//
// @param params : The parameters other than the build info to pass to
// `PipelineDumper::generateHashForGraphicsPipeline`.
// @param modifyBuildInfoFunc : A function that will modify the build info in some way.
// @param expectHashesToBeEqual : The expected result of comparing the hashes of the original buildinfo and the modified
// one.
void runGraphicsPipelineOptionsHashTest(GenerateHashParams params, const ModifyGraphicsBuildInfo &modifyBuildInfoFunc,
                                        bool expectHashesToBeEqual) {
  auto buildInfo = std::make_unique<GraphicsPipelineBuildInfo>();
  auto originalHash = PipelineDumper::generateHashForGraphicsPipeline(
      buildInfo.get(), params.isCacheHash, params.isRelocatableShader, params.unlinkedShaderStage);

  modifyBuildInfoFunc(buildInfo.get());
  auto modifiedHash = PipelineDumper::generateHashForGraphicsPipeline(
      buildInfo.get(), params.isCacheHash, params.isRelocatableShader, params.unlinkedShaderStage);
  if (expectHashesToBeEqual) {
    EXPECT_EQ(originalHash, modifiedHash);
  } else {
    EXPECT_NE(originalHash, modifiedHash);
  }
}

// =====================================================================================================================
// Runs a graphics pipeline options hash test on all possible values for the GenerateHashParams.
//
// @param modifyBuildInfoFunc : A function that will modify the build info in some way.
// @param expectHashToBeEqual : A function that returns whether or not the change make by `modifyBuildInfoFunc` will
// modify the hash for the given set of GenerateHashParameters.
void runGraphicsPipelineVariations(const ModifyGraphicsBuildInfo &modifyBuildInfo,
                                   const HashModifiedFunc &expectHashToBeEqual) {
  for (auto unlinkedShaderStage : {UnlinkedStageVertexProcess, UnlinkedStageFragment, UnlinkedStageCount}) {
    for (bool isCacheHash : {false, true}) {
      for (bool isRelocatableShader : {false, true}) {
        GenerateHashParams params = {isCacheHash, isRelocatableShader, unlinkedShaderStage};
        runGraphicsPipelineOptionsHashTest(params, modifyBuildInfo, expectHashToBeEqual(params));
      }
    }
  }
}

// =====================================================================================================================
// Runs a compute pipeline options hash test.  The test will follow these steps:
//
// 1) Get the hash for a default compute pipeline build info using `params`.
// 2) Modify the build info using `modifyBuildInfoFunc`.
// 3) Get the hash for the modified build info using `params`.
// 4) The test passes if hashes are equal exactly when expected, and unequal otherwise.
//
// @param params : The parameters other than the build info to pass to
// `PipelineDumper::generateHashForGraphicsPipeline`.
// @param modifyBuildInfoFunc : A function that will modify the build info in some way.
// @param expectHashesToBeEqual : The expected result of comparing the hashes of the original buildinfo and the modified
// one.
void runComputePipelineOptionsHashTest(GenerateHashParams params, const ModifyComputeBuildInfo &modifyBuildInfoFunc,
                                       bool expectHashesToBeEqual) {
  auto buildInfo = std::make_unique<ComputePipelineBuildInfo>();
  auto originalHash =
      PipelineDumper::generateHashForComputePipeline(buildInfo.get(), params.isCacheHash, params.isRelocatableShader);

  modifyBuildInfoFunc(buildInfo.get());
  auto modifiedHash =
      PipelineDumper::generateHashForComputePipeline(buildInfo.get(), params.isCacheHash, params.isRelocatableShader);
  if (expectHashesToBeEqual) {
    EXPECT_EQ(originalHash, modifiedHash);
  } else {
    EXPECT_NE(originalHash, modifiedHash);
  }
}

// =====================================================================================================================
// Runs a compute pipeline options hash test on all possible values for the GenerateHashParams.
//
// @param modifyBuildInfoFunc : A function that will modify the build info in some way.
// @param expectHashToBeEqual : A function that returns whether or not the change make by `modifyBuildInfoFunc` will
// modify the hash for the given set of GenerateHashParameters.
void runComputePipelineVariations(const ModifyComputeBuildInfo &modifyBuildInfo,
                                  const HashModifiedFunc &expectHashToBeEqual) {
  UnlinkedShaderStage unlinkedShaderStage = UnlinkedStageCount;
  for (bool isCacheHash : {false, true}) {
    for (bool isRelocatableShader : {false, true}) {
      GenerateHashParams params = {isCacheHash, isRelocatableShader, unlinkedShaderStage};
      runComputePipelineOptionsHashTest(params, modifyBuildInfo, expectHashToBeEqual(params));
    }
  }
}

// =====================================================================================================================
// Test the robustBufferAccess option.

// cppcheck-suppress syntaxError
TEST(PipelineDumperTest, TestRobustBufferAccessOptionGraphics) {
  ModifyGraphicsBuildInfo modifyBuildInfo = [](GraphicsPipelineBuildInfo *buildInfo) {
    buildInfo->options.robustBufferAccess = true;
  };
  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &) { return false; };
  runGraphicsPipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

TEST(PipelineDumperTest, TestRobustBufferAccessOptionCompute) {
  ModifyComputeBuildInfo modifyBuildInfo = [](ComputePipelineBuildInfo *buildInfo) {
    buildInfo->options.robustBufferAccess = true;
  };
  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &) { return false; };
  runComputePipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

// =====================================================================================================================
// Test the includeDisassembly option.

TEST(PipelineDumperTest, TestIncludeDisassemblyOptionGraphics) {
  ModifyGraphicsBuildInfo modifyBuildInfo = [](GraphicsPipelineBuildInfo *buildInfo) {
    buildInfo->options.includeDisassembly = true;
  };
  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) { return false; };
  runGraphicsPipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

TEST(PipelineDumperTest, TestIncludeDisassemblyOptionCompute) {
  ModifyComputeBuildInfo modifyBuildInfo = [](ComputePipelineBuildInfo *buildInfo) {
    buildInfo->options.includeDisassembly = true;
  };
  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) { return false; };
  runComputePipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

// =====================================================================================================================
// Test the enableInterpModePatch option.

TEST(PipelineDumperTest, TestEnableInterpModePatchOptionGraphics) {
  ModifyGraphicsBuildInfo modifyBuildInfo = [](GraphicsPipelineBuildInfo *buildInfo) {
    buildInfo->options.enableInterpModePatch = true;
  };

  // This should only modify the fragment shader.
  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) {
    return params.unlinkedShaderStage == UnlinkedStageVertexProcess;
  };
  runGraphicsPipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

TEST(PipelineDumperTest, TestEnableInterpModePatchOptionCompute) {
  ModifyComputeBuildInfo modifyBuildInfo = [](ComputePipelineBuildInfo *buildInfo) {
    buildInfo->options.enableInterpModePatch = true;
  };
  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) { return true; };
  runComputePipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

// =====================================================================================================================
// Test the shadowDescriptorTableUsage option.

TEST(PipelineDumperTest, TestShadowDescriptorTableUsageGraphics) {
  ModifyGraphicsBuildInfo modifyBuildInfo = [](GraphicsPipelineBuildInfo *buildInfo) {
    buildInfo->options.shadowDescriptorTableUsage = ShadowDescriptorTableUsage::Enable;
  };

  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) { return params.isRelocatableShader; };
  runGraphicsPipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

TEST(PipelineDumperTest, TestShadowDescriptorTableUsageCompute) {
  ModifyComputeBuildInfo modifyBuildInfo = [](ComputePipelineBuildInfo *buildInfo) {
    buildInfo->options.shadowDescriptorTableUsage = ShadowDescriptorTableUsage::Enable;
  };
  HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) { return params.isRelocatableShader; };
  runComputePipelineVariations(modifyBuildInfo, expectHashToBeEqual);
}

#if LLPC_CLIENT_INTERFACE_MAJOR_VERSION >= 53
// =====================================================================================================================
// Test the optimization level option.  The default value for the optimization level in the pipeline build info is 0.
// All tess will be compared with that.

TEST(PipelineDumperTest, TestOptimizationLevelGraphics) {

  for (uint32_t optLevel = 1; optLevel <= 3; ++optLevel) {
    ModifyGraphicsBuildInfo modifyBuildInfo = [optLevel](GraphicsPipelineBuildInfo *buildInfo) {
      buildInfo->options.optimizationLevel = optLevel;
    };

    // Even if LGC will internally bump the optimization level to 1 if it is 0, that is not reflected in the hash
    // because the hash is computed first.
    HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) { return false; };
    runGraphicsPipelineVariations(modifyBuildInfo, expectHashToBeEqual);
  }
}

TEST(PipelineDumperTest, TestOptimizationLevel1Compute) {
  for (uint32_t optLevel = 1; optLevel <= 3; ++optLevel) {
    ModifyComputeBuildInfo modifyBuildInfo = [optLevel](ComputePipelineBuildInfo *buildInfo) {
      buildInfo->options.optimizationLevel = optLevel;
    };

    // Even if LGC will internally bump the optimization level to 1 if it is 0, that is not reflected in the hash
    // because the hash is computed first.
    HashModifiedFunc expectHashToBeEqual = [](const GenerateHashParams &params) { return false; };
    runComputePipelineVariations(modifyBuildInfo, expectHashToBeEqual);
  }
}
#endif

} // namespace
} // namespace Llpc