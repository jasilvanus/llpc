#version 450

layout(set = 0, binding = 0) uniform sampler2D      samp2D;
layout(set = 1, binding = 0) uniform sampler2DArray samp2DArray[4];
layout(set = 0, binding = 1) uniform sampler2DRect  samp2DRect;

layout(set = 2, binding = 1) uniform Uniforms
{
    int index;
};

layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 f4 = textureGather(samp2D, vec2(0.1), 2);
    f4 += textureGather(samp2DArray[index], vec3(0.2), 3);
    f4 += textureGather(samp2DRect, vec2(1.0));

    fragColor = f4;
}
// BEGIN_SHADERTEST
/*
; RUN: amdllpc -spvgen-dir=%spvgendir% -v %gfxip %s | FileCheck -check-prefix=SHADERTEST %s
; SHADERTEST-LABEL: {{^// LLPC}} SPIRV-to-LLVM translation results
; SHADERTEST-LABEL: {{^// LLPC}}  SPIR-V lowering results
; SHADERTEST: <8 x i32> addrspace(4)* (...) @lgc.create.get.desc.ptr.p4v8i32(i32 1, i32 0, i32 0)
; SHADERTEST: <4 x i32> addrspace(4)* (...) @lgc.create.get.desc.ptr.p4v4i32(i32 2, i32 0, i32 0)
; SHADERTEST: call reassoc nnan nsz arcp contract afn <4 x float> (...) @lgc.create.image.gather.v4f32(i32 1, i32 512, <8 x {{.*}}, <4 x {{.*}}, i32 37, <2 x float> <float 0x3FB99999A0000000, float 0x3FB99999A0000000>, i32 2, float 0.000000e+00)
; SHADERTEST: <8 x i32> addrspace(4)* (...) @lgc.create.get.desc.ptr.p4v8i32(i32 1, i32 1, i32 0)
; SHADERTEST: <4 x i32> addrspace(4)* (...) @lgc.create.get.desc.ptr.p4v4i32(i32 2, i32 1, i32 0)
; SHADERTEST: call reassoc nnan nsz arcp contract afn <4 x float> (...) @lgc.create.image.gather.v4f32(i32 5, i32 384, <8 x {{.*}}, <4 x {{.*}}, i32 37, <3 x float> <float 0x3FC99999A0000000, float 0x3FC99999A0000000, float 0x3FC99999A0000000>, i32 3, float 0.000000e+00)
; SHADERTEST: <8 x i32> addrspace(4)* (...) @lgc.create.get.desc.ptr.p4v8i32(i32 1, i32 0, i32 1)
; SHADERTEST: <4 x i32> addrspace(4)* (...) @lgc.create.get.desc.ptr.p4v4i32(i32 2, i32 0, i32 1)
; SHADERTEST: call reassoc nnan nsz arcp contract afn <4 x float> (...) @lgc.create.image.gather.v4f32(i32 1, i32 512, <8 x {{.*}}, <4 x {{.*}}, i32 37, <2 x float> <float 1.000000e+00, float 1.000000e+00>, i32 0, float 0.000000e+00)

; SHADERTEST-LABEL: {{^// LLPC}}  pipeline patching results
; SHADERTEST: call {{.*}} <4 x float> @llvm.amdgcn.image.gather4.lz.2d.v4f32.f32(i32 4, float 0x3FB99999A0000000, float 0x3FB99999A0000000, <8 x i32> %{{[-0-9A-Za0z_.]+}}, <4 x i32> %{{[-0-9A-Za0z_.]+}}, i1 false, i32 0, i32 0)
; SHADERTEST: call {{.*}} <4 x float> @llvm.amdgcn.image.gather4.lz.2darray.v4f32.f32(i32 8, float 0x3FC99999A0000000, float 0x3FC99999A0000000, float 0.000000e+00, <8 x i32> %{{[-0-9A-Za0z_.]+}}, <4 x i32> %{{[-0-9A-Za0z_.]+}}, i1 false, i32 0, i32 0)
; SHADERTEST: call {{.*}} <4 x float> @llvm.amdgcn.image.gather4.lz.2d.v4f32.f32(i32 1, float 1.000000e+00, float 1.000000e+00,{{.*}}, i1 false, i32 0, i32 0)
; SHADERTEST: AMDLLPC SUCCESS
*/
// END_SHADERTEST
