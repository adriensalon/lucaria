//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) Guillaume Blanc                                              //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

// 25/11/21 initial commit by adriensalon

#ifndef OZZ_OZZ_BASE_MATHS_INTERNAL_SIMD_MATH_NEON_INL_H_
#define OZZ_OZZ_BASE_MATHS_INTERNAL_SIMD_MATH_NEON_INL_H_

// NEON implementation, based on scalar floats.

#include <stdint.h>

#include <cassert>

// Temporarly needed while trigonometric functions aren't implemented.
#include <cmath>

#include "ozz/base/maths/math_constant.h"

namespace ozz {
namespace math {

// Internal macros.

#define OZZ_NEON_SPLAT_F(_v, _i) vdupq_n_f32(vgetq_lane_f32((_v), (_i)))

#define OZZ_NEON_SPLAT_I(_v, _i) vdupq_n_s32(vgetq_lane_s32((_v), (_i)))

#if defined(__aarch64__)
#define OZZ_NEON_UNPACKLO_PS(a, b) vzip1q_f32((a), (b))
#define OZZ_NEON_UNPACKHI_PS(a, b) vzip2q_f32((a), (b))
#else
OZZ_INLINE float32x4_t OZZ_NEON_UNPACKLO_PS(float32x4_t a, float32x4_t b) {
  float32x4x2_t vec = vzipq_f32(a, b);
  return vec.val[0];
}
OZZ_INLINE float32x4_t OZZ_NEON_UNPACKHI_PS(float32x4_t a, float32x4_t b) {
  float32x4x2_t vec = vzipq_f32(a, b);
  return vec.val[1];
}
#endif

#define OZZ_NEON_MOVELH_PS(a, b) \
  vcombine_f32(vget_low_f32((a)), vget_low_f32((b)))

#define OZZ_NEON_MOVEHL_PS(a, b) \
  vcombine_f32(vget_high_f32((b)), vget_high_f32((a)))

#define OZZ_NEON_MOVE_SS(a, b) vsetq_lane_f32(vgetq_lane_f32((b), 0), (a), 0)

#define OZZ_NEON_SELECT_F(m, t, f)                          \
  vreinterpretq_f32_u32(vbslq_u32(vreinterpretq_u32_s32(m), \
                                  vreinterpretq_u32_f32(t), \
                                  vreinterpretq_u32_f32(f)))

OZZ_INLINE SimdInt4 OZZ_NEON_SELECT_I(_SimdInt4 m, _SimdInt4 t, _SimdInt4 f) {
  return vreinterpretq_s32_u32(vbslq_u32(vreinterpretq_u32_s32(m),
                                         vreinterpretq_u32_s32(t),
                                         vreinterpretq_u32_s32(f)));
}

namespace simd_float4 {

OZZ_INLINE SimdFloat4 zero() { return vdupq_n_f32(0.0f); }

OZZ_INLINE SimdFloat4 one() { return vdupq_n_f32(1.0f); }

OZZ_INLINE SimdFloat4 x_axis() {
  float32x4_t vec = vdupq_n_f32(0.0f);
  return vsetq_lane_f32(1.0f, vec, 0);
}

OZZ_INLINE SimdFloat4 y_axis() {
  float32x4_t vec = vdupq_n_f32(0.0f);
  return vsetq_lane_f32(1.0f, vec, 1);
}

OZZ_INLINE SimdFloat4 z_axis() {
  float32x4_t vec = vdupq_n_f32(0.0f);
  return vsetq_lane_f32(1.0f, vec, 2);
}

OZZ_INLINE SimdFloat4 w_axis() {
  float32x4_t vec = vdupq_n_f32(0.0f);
  return vsetq_lane_f32(1.0f, vec, 3);
}

OZZ_INLINE SimdFloat4 Load(float _x, float _y, float _z, float _w) {
  float32x4_t vec = vdupq_n_f32(0.0f);
  vec = vsetq_lane_f32(_x, vec, 0);
  vec = vsetq_lane_f32(_y, vec, 1);
  vec = vsetq_lane_f32(_z, vec, 2);
  vec = vsetq_lane_f32(_w, vec, 3);
  return vec;
}

OZZ_INLINE SimdFloat4 LoadX(float _x) {
  float32x4_t vec = vdupq_n_f32(0.0f);
  return vsetq_lane_f32(_x, vec, 0);
}

OZZ_INLINE SimdFloat4 Load1(float _x) { return vdupq_n_f32(_x); }

OZZ_INLINE SimdFloat4 LoadPtr(const float* _f) {
  assert(!(reinterpret_cast<uintptr_t>(_f) & 0xF) && "Invalid alignment");
  return vld1q_f32(_f);
}

OZZ_INLINE SimdFloat4 LoadPtrU(const float* _f) { return vld1q_f32(_f); }

OZZ_INLINE SimdFloat4 LoadXPtrU(const float* _f) {
  float32x4_t vec = vdupq_n_f32(0.0f);
  return vsetq_lane_f32(_f[0], vec, 0);
}

OZZ_INLINE SimdFloat4 Load1PtrU(const float* _f) { return vdupq_n_f32(_f[0]); }

OZZ_INLINE SimdFloat4 Load2PtrU(const float* _f) {
  float32x4_t vec = vdupq_n_f32(0.0f);
  vec = vsetq_lane_f32(_f[0], vec, 0);
  vec = vsetq_lane_f32(_f[1], vec, 1);
  return vec;
}

OZZ_INLINE SimdFloat4 Load3PtrU(const float* _f) {
  float32x4_t vec = vdupq_n_f32(0.0f);
  vec = vsetq_lane_f32(_f[0], vec, 0);
  vec = vsetq_lane_f32(_f[1], vec, 1);
  vec = vsetq_lane_f32(_f[2], vec, 2);
  return vec;
}

OZZ_INLINE SimdFloat4 FromInt(_SimdInt4 _i) {
  return vcvtq_f32_s32(_i);
}

}  // namespace simd_float4

OZZ_INLINE float GetX(_SimdFloat4 _v) { return vgetq_lane_f32(_v, 0); }

OZZ_INLINE float GetY(_SimdFloat4 _v) { return vgetq_lane_f32(_v, 1); }

OZZ_INLINE float GetZ(_SimdFloat4 _v) { return vgetq_lane_f32(_v, 2); }

OZZ_INLINE float GetW(_SimdFloat4 _v) { return vgetq_lane_f32(_v, 3); }

OZZ_INLINE SimdFloat4 SetX(_SimdFloat4 _v, _SimdFloat4 _f) {
  return vsetq_lane_f32(GetX(_f), _v, 0);
}

OZZ_INLINE SimdFloat4 SetY(_SimdFloat4 _v, _SimdFloat4 _f) {
  return vsetq_lane_f32(GetX(_f), _v, 1);
}

OZZ_INLINE SimdFloat4 SetZ(_SimdFloat4 _v, _SimdFloat4 _f) {
  return vsetq_lane_f32(GetX(_f), _v, 2);
}

OZZ_INLINE SimdFloat4 SetW(_SimdFloat4 _v, _SimdFloat4 _f) {
  return vsetq_lane_f32(GetX(_f), _v, 3);
}

OZZ_INLINE SimdFloat4 SetI(_SimdFloat4 _v, _SimdFloat4 _f, int _ith) {
  assert(_ith >= 0 && _ith <= 3 && "Invalid index, out of range.");
  const float value = GetX(_f);
  switch (_ith) {
    case 0:
      return vsetq_lane_f32(value, _v, 0);
    case 1:
      return vsetq_lane_f32(value, _v, 1);
    case 2:
      return vsetq_lane_f32(value, _v, 2);
    default:
      return vsetq_lane_f32(value, _v, 3);
  }
}

OZZ_INLINE void StorePtr(_SimdFloat4 _v, float* _f) {
  assert(!(reinterpret_cast<uintptr_t>(_f) & 0xF) && "Invalid alignment");
  vst1q_f32(_f, _v);
}

OZZ_INLINE void Store1Ptr(_SimdFloat4 _v, float* _f) {
  assert(!(reinterpret_cast<uintptr_t>(_f) & 0xF) && "Invalid alignment");
  _f[0] = GetX(_v);
}

OZZ_INLINE void Store2Ptr(_SimdFloat4 _v, float* _f) {
  assert(!(reinterpret_cast<uintptr_t>(_f) & 0xF) && "Invalid alignment");
  _f[0] = GetX(_v);
  _f[1] = GetY(_v);
}

OZZ_INLINE void Store3Ptr(_SimdFloat4 _v, float* _f) {
  assert(!(reinterpret_cast<uintptr_t>(_f) & 0xF) && "Invalid alignment");
  _f[0] = GetX(_v);
  _f[1] = GetY(_v);
  _f[2] = GetZ(_v);
}

OZZ_INLINE void StorePtrU(_SimdFloat4 _v, float* _f) { vst1q_f32(_f, _v); }

OZZ_INLINE void Store1PtrU(_SimdFloat4 _v, float* _f) { _f[0] = GetX(_v); }

OZZ_INLINE void Store2PtrU(_SimdFloat4 _v, float* _f) {
  _f[0] = GetX(_v);
  _f[1] = GetY(_v);
}

OZZ_INLINE void Store3PtrU(_SimdFloat4 _v, float* _f) {
  _f[0] = GetX(_v);
  _f[1] = GetY(_v);
  _f[2] = GetZ(_v);
}

OZZ_INLINE SimdFloat4 SplatX(_SimdFloat4 _v) { return OZZ_NEON_SPLAT_F(_v, 0); }

OZZ_INLINE SimdFloat4 SplatY(_SimdFloat4 _v) { return OZZ_NEON_SPLAT_F(_v, 1); }

OZZ_INLINE SimdFloat4 SplatZ(_SimdFloat4 _v) { return OZZ_NEON_SPLAT_F(_v, 2); }

OZZ_INLINE SimdFloat4 SplatW(_SimdFloat4 _v) { return OZZ_NEON_SPLAT_F(_v, 3); }

template <size_t _X, size_t _Y, size_t _Z, size_t _W>
OZZ_INLINE SimdFloat4 Swizzle(_SimdFloat4 _v) {
  static_assert(_X <= 3 && _Y <= 3 && _Z <= 3 && _W <= 3,
                "Indices must be between 0 and 3");
  float32x4_t vec = vdupq_n_f32(vgetq_lane_f32(_v, _X));  // lane 0
  vec = vsetq_lane_f32(vgetq_lane_f32(_v, _Y), vec, 1);
  vec = vsetq_lane_f32(vgetq_lane_f32(_v, _Z), vec, 2);
  vec = vsetq_lane_f32(vgetq_lane_f32(_v, _W), vec, 3);
  return vec;
}

OZZ_INLINE void Transpose4x1(const SimdFloat4 _in[4], SimdFloat4 _out[1]) {
  const float32x4_t xz = OZZ_NEON_UNPACKLO_PS(_in[0], _in[2]);
  const float32x4_t yw = OZZ_NEON_UNPACKLO_PS(_in[1], _in[3]);
  _out[0] = OZZ_NEON_UNPACKLO_PS(xz, yw);
}

OZZ_INLINE void Transpose1x4(const SimdFloat4 _in[1], SimdFloat4 _out[4]) {
  const float32x4_t zwzw = OZZ_NEON_MOVEHL_PS(_in[0], _in[0]);
  const float32x4_t yyyy = OZZ_NEON_SPLAT_F(_in[0], 1);
  const float32x4_t wwww = OZZ_NEON_SPLAT_F(_in[0], 3);
  const float32x4_t zero = vdupq_n_f32(0.0f);
  _out[0] = OZZ_NEON_MOVE_SS(zero, _in[0]);
  _out[1] = OZZ_NEON_MOVE_SS(zero, yyyy);
  _out[2] = OZZ_NEON_MOVE_SS(zero, zwzw);
  _out[3] = OZZ_NEON_MOVE_SS(zero, wwww);
}

OZZ_INLINE void Transpose4x2(const SimdFloat4 _in[4], SimdFloat4 _out[2]) {
  const float32x4_t tmp0 = OZZ_NEON_UNPACKLO_PS(_in[0], _in[2]);
  const float32x4_t tmp1 = OZZ_NEON_UNPACKLO_PS(_in[1], _in[3]);
  _out[0] = OZZ_NEON_UNPACKLO_PS(tmp0, tmp1);
  _out[1] = OZZ_NEON_UNPACKHI_PS(tmp0, tmp1);
}

OZZ_INLINE void Transpose2x4(const SimdFloat4 _in[2], SimdFloat4 _out[4]) {
  const float32x4_t tmp0 = OZZ_NEON_UNPACKLO_PS(_in[0], _in[1]);
  const float32x4_t tmp1 = OZZ_NEON_UNPACKHI_PS(_in[0], _in[1]);
  const float32x4_t zero = vdupq_n_f32(0.0f);
  _out[0] = OZZ_NEON_MOVELH_PS(tmp0, zero);
  _out[1] = OZZ_NEON_MOVEHL_PS(zero, tmp0);
  _out[2] = OZZ_NEON_MOVELH_PS(tmp1, zero);
  _out[3] = OZZ_NEON_MOVEHL_PS(zero, tmp1);
}

OZZ_INLINE void Transpose4x3(const SimdFloat4 _in[4], SimdFloat4 _out[3]) {
  const float32x4_t tmp0 = OZZ_NEON_UNPACKLO_PS(_in[0], _in[2]);
  const float32x4_t tmp1 = OZZ_NEON_UNPACKLO_PS(_in[1], _in[3]);
  const float32x4_t tmp2 = OZZ_NEON_UNPACKHI_PS(_in[0], _in[2]);
  const float32x4_t tmp3 = OZZ_NEON_UNPACKHI_PS(_in[1], _in[3]);
  _out[0] = OZZ_NEON_UNPACKLO_PS(tmp0, tmp1);
  _out[1] = OZZ_NEON_UNPACKHI_PS(tmp0, tmp1);
  _out[2] = OZZ_NEON_UNPACKLO_PS(tmp2, tmp3);
}

OZZ_INLINE void Transpose3x4(const SimdFloat4 _in[3], SimdFloat4 _out[4]) {
  const float32x4_t zero = vdupq_n_f32(0.0f);
  const float32x4_t temp0 = OZZ_NEON_UNPACKLO_PS(_in[0], _in[1]);
  const float32x4_t temp1 = OZZ_NEON_UNPACKLO_PS(_in[2], zero);
  const float32x4_t temp2 = OZZ_NEON_UNPACKHI_PS(_in[0], _in[1]);
  const float32x4_t temp3 = OZZ_NEON_UNPACKHI_PS(_in[2], zero);
  _out[0] = OZZ_NEON_MOVELH_PS(temp0, temp1);
  _out[1] = OZZ_NEON_MOVEHL_PS(temp1, temp0);
  _out[2] = OZZ_NEON_MOVELH_PS(temp2, temp3);
  _out[3] = OZZ_NEON_MOVEHL_PS(temp3, temp2);
}

OZZ_INLINE void Transpose4x4(const SimdFloat4 _in[4], SimdFloat4 _out[4]) {
  const float32x4_t tmp0 = OZZ_NEON_UNPACKLO_PS(_in[0], _in[2]);
  const float32x4_t tmp1 = OZZ_NEON_UNPACKLO_PS(_in[1], _in[3]);
  const float32x4_t tmp2 = OZZ_NEON_UNPACKHI_PS(_in[0], _in[2]);
  const float32x4_t tmp3 = OZZ_NEON_UNPACKHI_PS(_in[1], _in[3]);
  _out[0] = OZZ_NEON_UNPACKLO_PS(tmp0, tmp1);
  _out[1] = OZZ_NEON_UNPACKHI_PS(tmp0, tmp1);
  _out[2] = OZZ_NEON_UNPACKLO_PS(tmp2, tmp3);
  _out[3] = OZZ_NEON_UNPACKHI_PS(tmp2, tmp3);
}

OZZ_INLINE void Transpose16x16(const SimdFloat4 _in[16], SimdFloat4 _out[16]) {
  const float32x4_t tmp0 = OZZ_NEON_UNPACKLO_PS(_in[0], _in[2]);
  const float32x4_t tmp1 = OZZ_NEON_UNPACKLO_PS(_in[1], _in[3]);
  _out[0] = OZZ_NEON_UNPACKLO_PS(tmp0, tmp1);
  _out[4] = OZZ_NEON_UNPACKHI_PS(tmp0, tmp1);
  const float32x4_t tmp2 = OZZ_NEON_UNPACKHI_PS(_in[0], _in[2]);
  const float32x4_t tmp3 = OZZ_NEON_UNPACKHI_PS(_in[1], _in[3]);
  _out[8] = OZZ_NEON_UNPACKLO_PS(tmp2, tmp3);
  _out[12] = OZZ_NEON_UNPACKHI_PS(tmp2, tmp3);
  const float32x4_t tmp4 = OZZ_NEON_UNPACKLO_PS(_in[4], _in[6]);
  const float32x4_t tmp5 = OZZ_NEON_UNPACKLO_PS(_in[5], _in[7]);
  _out[1] = OZZ_NEON_UNPACKLO_PS(tmp4, tmp5);
  _out[5] = OZZ_NEON_UNPACKHI_PS(tmp4, tmp5);
  const float32x4_t tmp6 = OZZ_NEON_UNPACKHI_PS(_in[4], _in[6]);
  const float32x4_t tmp7 = OZZ_NEON_UNPACKHI_PS(_in[5], _in[7]);
  _out[9] = OZZ_NEON_UNPACKLO_PS(tmp6, tmp7);
  _out[13] = OZZ_NEON_UNPACKHI_PS(tmp6, tmp7);
  const float32x4_t tmp8 = OZZ_NEON_UNPACKLO_PS(_in[8], _in[10]);
  const float32x4_t tmp9 = OZZ_NEON_UNPACKLO_PS(_in[9], _in[11]);
  _out[2] = OZZ_NEON_UNPACKLO_PS(tmp8, tmp9);
  _out[6] = OZZ_NEON_UNPACKHI_PS(tmp8, tmp9);
  const float32x4_t tmp10 = OZZ_NEON_UNPACKHI_PS(_in[8], _in[10]);
  const float32x4_t tmp11 = OZZ_NEON_UNPACKHI_PS(_in[9], _in[11]);
  _out[10] = OZZ_NEON_UNPACKLO_PS(tmp10, tmp11);
  _out[14] = OZZ_NEON_UNPACKHI_PS(tmp10, tmp11);
  const float32x4_t tmp12 = OZZ_NEON_UNPACKLO_PS(_in[12], _in[14]);
  const float32x4_t tmp13 = OZZ_NEON_UNPACKLO_PS(_in[13], _in[15]);
  _out[3] = OZZ_NEON_UNPACKLO_PS(tmp12, tmp13);
  _out[7] = OZZ_NEON_UNPACKHI_PS(tmp12, tmp13);
  const float32x4_t tmp14 = OZZ_NEON_UNPACKHI_PS(_in[12], _in[14]);
  const float32x4_t tmp15 = OZZ_NEON_UNPACKHI_PS(_in[13], _in[15]);
  _out[11] = OZZ_NEON_UNPACKLO_PS(tmp14, tmp15);
  _out[15] = OZZ_NEON_UNPACKHI_PS(tmp14, tmp15);
}

OZZ_INLINE SimdFloat4 MAdd(_SimdFloat4 _a, _SimdFloat4 _b, _SimdFloat4 _c) {
  return vmlaq_f32(_c, _a, _b);
}

OZZ_INLINE SimdFloat4 MSub(_SimdFloat4 _a, _SimdFloat4 _b, _SimdFloat4 _c) {
  return vsubq_f32(vmulq_f32(_a, _b), _c);
}

OZZ_INLINE SimdFloat4 NMAdd(_SimdFloat4 _a, _SimdFloat4 _b, _SimdFloat4 _c) {
  return vsubq_f32(_c, vmulq_f32(_a, _b));
}

OZZ_INLINE SimdFloat4 NMSub(_SimdFloat4 _a, _SimdFloat4 _b, _SimdFloat4 _c) {
  return vnegq_f32(vaddq_f32(vmulq_f32(_a, _b), _c));
}

OZZ_INLINE SimdFloat4 DivX(_SimdFloat4 _a, _SimdFloat4 _b) {
  float32x4_t vec = _a;
  const float ax = vgetq_lane_f32(_a, 0);
  const float bx = vgetq_lane_f32(_b, 0);
  vec = vsetq_lane_f32(ax / bx, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 HAdd2(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  vec = vsetq_lane_f32(x + y, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 HAdd3(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  vec = vsetq_lane_f32(x + y + z, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 HAdd4(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float w = vgetq_lane_f32(_v, 3);
  vec = vsetq_lane_f32(x + y + z + w, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Dot2(_SimdFloat4 _a, _SimdFloat4 _b) {
  const float32x4_t ab = vmulq_f32(_a, _b);
  const float s = vgetq_lane_f32(ab, 0) + vgetq_lane_f32(ab, 1);
  float32x4_t vec = ab;
  vec = vsetq_lane_f32(s, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Dot3(_SimdFloat4 _a, _SimdFloat4 _b) {
  const float32x4_t ab = vmulq_f32(_a, _b);
  const float s =
      vgetq_lane_f32(ab, 0) + vgetq_lane_f32(ab, 1) + vgetq_lane_f32(ab, 2);
  float32x4_t vec = ab;
  vec = vsetq_lane_f32(s, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Dot4(_SimdFloat4 _a, _SimdFloat4 _b) {
  const float32x4_t ab = vmulq_f32(_a, _b);
  const float s = vgetq_lane_f32(ab, 0) + vgetq_lane_f32(ab, 1) +
                  vgetq_lane_f32(ab, 2) + vgetq_lane_f32(ab, 3);
  float32x4_t vec = ab;
  vec = vsetq_lane_f32(s, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Cross3(_SimdFloat4 _a, _SimdFloat4 _b) {
  const float ax = vgetq_lane_f32(_a, 0);
  const float ay = vgetq_lane_f32(_a, 1);
  const float az = vgetq_lane_f32(_a, 2);
  const float bx = vgetq_lane_f32(_b, 0);
  const float by = vgetq_lane_f32(_b, 1);
  const float bz = vgetq_lane_f32(_b, 2);
  float32x4_t vec = vdupq_n_f32(0.0f);
  vec = vsetq_lane_f32(ay * bz - az * by, vec, 0);
  vec = vsetq_lane_f32(az * bx - ax * bz, vec, 1);
  vec = vsetq_lane_f32(ax * by - ay * bx, vec, 2);
  return vec;
}

OZZ_INLINE SimdFloat4 RcpEst(_SimdFloat4 _v) { return vrecpeq_f32(_v); }

OZZ_INLINE SimdFloat4 RcpEstNR(_SimdFloat4 _v) {
  float32x4_t vec = vrecpeq_f32(_v);
  vec = vmulq_f32(vec, vrecpsq_f32(_v, vec));
  vec = vmulq_f32(vec, vrecpsq_f32(_v, vec));
  return vec;
}

OZZ_INLINE SimdFloat4 RcpEstX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  const float x = vgetq_lane_f32(_v, 0);
  vec = vsetq_lane_f32(1.0f / x, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 RcpEstXNR(_SimdFloat4 _v) { return RcpEstX(_v); }

OZZ_INLINE SimdFloat4 Sqrt(_SimdFloat4 _v) { return vsqrtq_f32(_v); }

OZZ_INLINE SimdFloat4 SqrtX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  const float x = vgetq_lane_f32(_v, 0);
  vec = vsetq_lane_f32(std::sqrt(x), vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 RSqrtEst(_SimdFloat4 _v) { return vrsqrteq_f32(_v); }

OZZ_INLINE SimdFloat4 RSqrtEstNR(_SimdFloat4 _v) {
  float32x4_t vec = vrsqrteq_f32(_v);
  vec = vmulq_f32(vec, vrsqrtsq_f32(vmulq_f32(_v, vec), vec));
  vec = vmulq_f32(vec, vrsqrtsq_f32(vmulq_f32(_v, vec), vec));
  return vec;
}

OZZ_INLINE SimdFloat4 RSqrtEstX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  const float x = vgetq_lane_f32(_v, 0);
  vec = vsetq_lane_f32(1.0f / std::sqrt(x), vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 RSqrtEstXNR(_SimdFloat4 _v) { return RSqrtEstX(_v); }

OZZ_INLINE SimdFloat4 Abs(_SimdFloat4 _v) { return vabsq_f32(_v); }

OZZ_INLINE SimdInt4 Sign(_SimdFloat4 _v) {
  const uint32x4_t bits = vreinterpretq_u32_f32(_v);
  const uint32x4_t s1 = vshrq_n_u32(bits, 31);
  const uint32x4_t s2 = vshlq_n_u32(s1, 31);
  return vreinterpretq_s32_u32(s2);
}

OZZ_INLINE SimdFloat4 Length2Sqr(_SimdFloat4 _v) { return Dot2(_v, _v); }

OZZ_INLINE SimdFloat4 Length3Sqr(_SimdFloat4 _v) { return Dot3(_v, _v); }

OZZ_INLINE SimdFloat4 Length4Sqr(_SimdFloat4 _v) { return Dot4(_v, _v); }

OZZ_INLINE SimdFloat4 Length2(_SimdFloat4 _v) {
  const SimdFloat4 sq = Length2Sqr(_v);
  float32x4_t vec = vdupq_n_f32(0.0f);
  const float s = std::sqrt(vgetq_lane_f32(sq, 0));
  vec = vsetq_lane_f32(s, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Length3(_SimdFloat4 _v) {
  const SimdFloat4 sq = Length3Sqr(_v);
  float32x4_t vec = vdupq_n_f32(0.0f);
  const float s = std::sqrt(vgetq_lane_f32(sq, 0));
  vec = vsetq_lane_f32(s, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Length4(_SimdFloat4 _v) {
  const SimdFloat4 sq = Length4Sqr(_v);
  float32x4_t vec = vdupq_n_f32(0.0f);
  const float s = std::sqrt(vgetq_lane_f32(sq, 0));
  vec = vsetq_lane_f32(s, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Normalize2(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float len2 = x * x + y * y;
  assert(len2 != 0.f && "_v is not normalizable");
  const float inv_len = 1.0f / std::sqrt(len2);
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(x * inv_len, vec, 0);
  vec = vsetq_lane_f32(y * inv_len, vec, 1);
  return vec;
}

OZZ_INLINE SimdFloat4 Normalize3(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float len2 = x * x + y * y + z * z;
  assert(len2 != 0.f && "_v is not normalizable");
  const float inv_len = 1.0f / std::sqrt(len2);
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(x * inv_len, vec, 0);
  vec = vsetq_lane_f32(y * inv_len, vec, 1);
  vec = vsetq_lane_f32(z * inv_len, vec, 2);
  return vec;
}

OZZ_INLINE SimdFloat4 Normalize4(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float w = vgetq_lane_f32(_v, 3);
  const float len2 = x * x + y * y + z * z + w * w;
  assert(len2 != 0.f && "_v is not normalizable");
  const float inv_len = 1.0f / std::sqrt(len2);
  float32x4_t vec = vdupq_n_f32(0.0f);
  vec = vsetq_lane_f32(x * inv_len, vec, 0);
  vec = vsetq_lane_f32(y * inv_len, vec, 1);
  vec = vsetq_lane_f32(z * inv_len, vec, 2);
  vec = vsetq_lane_f32(w * inv_len, vec, 3);
  return vec;
}

OZZ_INLINE SimdFloat4 NormalizeEst2(_SimdFloat4 _v) { return Normalize2(_v); }

OZZ_INLINE SimdFloat4 NormalizeEst3(_SimdFloat4 _v) { return Normalize3(_v); }

OZZ_INLINE SimdFloat4 NormalizeEst4(_SimdFloat4 _v) { return Normalize4(_v); }

OZZ_INLINE SimdInt4 IsNormalized2(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float len2 = x * x + y * y;
  const float max = 1.f + kNormalizationToleranceSq;
  const float min = 1.f - kNormalizationToleranceSq;
  const uint32_t m = (len2 < max && len2 > min) ? 0xFFFFFFFFu : 0u;
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32((int32_t)m, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 IsNormalized3(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float len2 = x * x + y * y + z * z;
  const float max = 1.f + kNormalizationToleranceSq;
  const float min = 1.f - kNormalizationToleranceSq;
  const uint32_t m = (len2 < max && len2 > min) ? 0xFFFFFFFFu : 0u;
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32((int32_t)m, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 IsNormalized4(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float w = vgetq_lane_f32(_v, 3);
  const float len2 = x * x + y * y + z * z + w * w;
  const float max = 1.f + kNormalizationToleranceSq;
  const float min = 1.f - kNormalizationToleranceSq;
  const uint32_t m = (len2 < max && len2 > min) ? 0xFFFFFFFFu : 0u;
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32((int32_t)m, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 IsNormalizedEst2(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float len2 = x * x + y * y;
  const float max = 1.f + kNormalizationToleranceEstSq;
  const float min = 1.f - kNormalizationToleranceEstSq;
  const uint32_t m = (len2 < max && len2 > min) ? 0xFFFFFFFFu : 0u;
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32((int32_t)m, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 IsNormalizedEst3(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float len2 = x * x + y * y + z * z;
  const float max = 1.f + kNormalizationToleranceEstSq;
  const float min = 1.f - kNormalizationToleranceEstSq;
  const uint32_t m = (len2 < max && len2 > min) ? 0xFFFFFFFFu : 0u;
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32((int32_t)m, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 IsNormalizedEst4(_SimdFloat4 _v) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float w = vgetq_lane_f32(_v, 3);
  const float len2 = x * x + y * y + z * z + w * w;
  const float max = 1.f + kNormalizationToleranceEstSq;
  const float min = 1.f - kNormalizationToleranceEstSq;
  const uint32_t m = (len2 < max && len2 > min) ? 0xFFFFFFFFu : 0u;
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32((int32_t)m, vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 NormalizeSafe2(_SimdFloat4 _v, _SimdFloat4 _safe) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float len2 = x * x + y * y;
  if (len2 <= 0.f) {
    return _safe;
  }
  const float inv_len = 1.0f / std::sqrt(len2);
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(x * inv_len, vec, 0);
  vec = vsetq_lane_f32(y * inv_len, vec, 1);
  return vec;
}

OZZ_INLINE SimdFloat4 NormalizeSafe3(_SimdFloat4 _v, _SimdFloat4 _safe) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float len2 = x * x + y * y + z * z;
  if (len2 <= 0.f) {
    return _safe;
  }
  const float inv_len = 1.0f / std::sqrt(len2);
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(x * inv_len, vec, 0);
  vec = vsetq_lane_f32(y * inv_len, vec, 1);
  vec = vsetq_lane_f32(z * inv_len, vec, 2);
  return vec;
}

OZZ_INLINE SimdFloat4 NormalizeSafe4(_SimdFloat4 _v, _SimdFloat4 _safe) {
  const float x = vgetq_lane_f32(_v, 0);
  const float y = vgetq_lane_f32(_v, 1);
  const float z = vgetq_lane_f32(_v, 2);
  const float w = vgetq_lane_f32(_v, 3);
  const float len2 = x * x + y * y + z * z + w * w;
  if (len2 <= 0.f) {
    return _safe;
  }
  const float inv_len = 1.0f / std::sqrt(len2);
  float32x4_t vec = vdupq_n_f32(0.0f);
  vec = vsetq_lane_f32(x * inv_len, vec, 0);
  vec = vsetq_lane_f32(y * inv_len, vec, 1);
  vec = vsetq_lane_f32(z * inv_len, vec, 2);
  vec = vsetq_lane_f32(w * inv_len, vec, 3);
  return vec;
}

OZZ_INLINE SimdFloat4 NormalizeSafeEst2(_SimdFloat4 _v, _SimdFloat4 _safe) {
  return NormalizeSafe2(_v, _safe);
}

OZZ_INLINE SimdFloat4 NormalizeSafeEst3(_SimdFloat4 _v, _SimdFloat4 _safe) {
  return NormalizeSafe3(_v, _safe);
}

OZZ_INLINE SimdFloat4 NormalizeSafeEst4(_SimdFloat4 _v, _SimdFloat4 _safe) {
  return NormalizeSafe4(_v, _safe);
}

OZZ_INLINE SimdFloat4 Lerp(_SimdFloat4 _a, _SimdFloat4 _b, _SimdFloat4 _alpha) {
  return MAdd(_alpha, vsubq_f32(_b, _a), _a);
}

OZZ_INLINE SimdFloat4 Min(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vminq_f32(_a, _b);
}

OZZ_INLINE SimdFloat4 Max(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vmaxq_f32(_a, _b);
}

OZZ_INLINE SimdFloat4 Min0(_SimdFloat4 _v) {
  const float32x4_t zero = vdupq_n_f32(0.0f);
  return vminq_f32(zero, _v);
}

OZZ_INLINE SimdFloat4 Max0(_SimdFloat4 _v) {
  const float32x4_t zero = vdupq_n_f32(0.0f);
  return vmaxq_f32(zero, _v);
}

OZZ_INLINE SimdFloat4 Clamp(_SimdFloat4 _a, _SimdFloat4 _v, _SimdFloat4 _b) {
  return vmaxq_f32(_a, vminq_f32(_v, _b));
}

OZZ_INLINE SimdFloat4 Select(_SimdInt4 _b, _SimdFloat4 _true,
                             _SimdFloat4 _false) {
  return OZZ_NEON_SELECT_F(_b, _true, _false);
}

OZZ_INLINE SimdInt4 CmpEq(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_s32_u32(vceqq_f32(_a, _b));
}

OZZ_INLINE SimdInt4 CmpNe(_SimdFloat4 _a, _SimdFloat4 _b) {
  uint32x4_t eq = vceqq_f32(_a, _b);
  return vreinterpretq_s32_u32(vmvnq_u32(eq));
}

OZZ_INLINE SimdInt4 CmpLt(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_s32_u32(vcltq_f32(_a, _b));
}

OZZ_INLINE SimdInt4 CmpLe(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_s32_u32(vcleq_f32(_a, _b));
}

OZZ_INLINE SimdInt4 CmpGt(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_s32_u32(vcgtq_f32(_a, _b));
}

OZZ_INLINE SimdInt4 CmpGe(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_s32_u32(vcgeq_f32(_a, _b));
}

OZZ_INLINE SimdFloat4 And(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_f32_u32(
      vandq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
}

OZZ_INLINE SimdFloat4 Or(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_f32_u32(
      vorrq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
}

OZZ_INLINE SimdFloat4 Xor(_SimdFloat4 _a, _SimdFloat4 _b) {
  return vreinterpretq_f32_u32(
      veorq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_f32(_b)));
}

OZZ_INLINE SimdFloat4 And(_SimdFloat4 _a, _SimdInt4 _b) {
  return vreinterpretq_f32_u32(
      vandq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_s32(_b)));
}

OZZ_INLINE SimdFloat4 AndNot(_SimdFloat4 _a, _SimdInt4 _b) {
  return vreinterpretq_f32_u32(
      vbicq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_s32(_b)));
}

OZZ_INLINE SimdFloat4 Or(_SimdFloat4 _a, _SimdInt4 _b) {
  return vreinterpretq_f32_u32(
      vorrq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_s32(_b)));
}

OZZ_INLINE SimdFloat4 Xor(_SimdFloat4 _a, _SimdInt4 _b) {
  return vreinterpretq_f32_u32(
      veorq_u32(vreinterpretq_u32_f32(_a), vreinterpretq_u32_s32(_b)));
}

OZZ_INLINE SimdFloat4 Cos(_SimdFloat4 _v) {
  return simd_float4::Load(std::cos(GetX(_v)), std::cos(GetY(_v)),
                           std::cos(GetZ(_v)), std::cos(GetW(_v)));
}

OZZ_INLINE SimdFloat4 CosX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(std::cos(GetX(_v)), vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 ACos(_SimdFloat4 _v) {
  return simd_float4::Load(std::acos(GetX(_v)), std::acos(GetY(_v)),
                           std::acos(GetZ(_v)), std::acos(GetW(_v)));
}

OZZ_INLINE SimdFloat4 ACosX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(std::acos(GetX(_v)), vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Sin(_SimdFloat4 _v) {
  return simd_float4::Load(std::sin(GetX(_v)), std::sin(GetY(_v)),
                           std::sin(GetZ(_v)), std::sin(GetW(_v)));
}

OZZ_INLINE SimdFloat4 SinX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(std::sin(GetX(_v)), vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 ASin(_SimdFloat4 _v) {
  return simd_float4::Load(std::asin(GetX(_v)), std::asin(GetY(_v)),
                           std::asin(GetZ(_v)), std::asin(GetW(_v)));
}

OZZ_INLINE SimdFloat4 ASinX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(std::asin(GetX(_v)), vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 Tan(_SimdFloat4 _v) {
  return simd_float4::Load(std::tan(GetX(_v)), std::tan(GetY(_v)),
                           std::tan(GetZ(_v)), std::tan(GetW(_v)));
}

OZZ_INLINE SimdFloat4 TanX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(std::tan(GetX(_v)), vec, 0);
  return vec;
}

OZZ_INLINE SimdFloat4 ATan(_SimdFloat4 _v) {
  return simd_float4::Load(std::atan(GetX(_v)), std::atan(GetY(_v)),
                           std::atan(GetZ(_v)), std::atan(GetW(_v)));
}

OZZ_INLINE SimdFloat4 ATanX(_SimdFloat4 _v) {
  float32x4_t vec = _v;
  vec = vsetq_lane_f32(std::atan(GetX(_v)), vec, 0);
  return vec;
}

namespace simd_int4 {

OZZ_INLINE SimdInt4 zero() { return vdupq_n_s32(0); }

OZZ_INLINE SimdInt4 one() {
  // {1,1,1,1}
  return vdupq_n_s32(1);
}

OZZ_INLINE SimdInt4 x_axis() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(1, vec, 0);
  return vec;  // {1,0,0,0}
}

OZZ_INLINE SimdInt4 y_axis() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(1, vec, 1);
  return vec;  // {0,1,0,0}
}

OZZ_INLINE SimdInt4 z_axis() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(1, vec, 2);
  return vec;  // {0,0,1,0}
}

OZZ_INLINE SimdInt4 w_axis() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(1, vec, 3);
  return vec;  // {0,0,0,1}
}

OZZ_INLINE SimdInt4 all_true() {
  return vdupq_n_s32(-1);  // 0xFFFFFFFF in all lanes
}

OZZ_INLINE SimdInt4 all_false() { return vdupq_n_s32(0); }

OZZ_INLINE SimdInt4 mask_sign() {
  return vdupq_n_s32(0x80000000u);  // sign bit in all lanes
}

OZZ_INLINE SimdInt4 mask_sign_xyz() {
  int32x4_t vec = vdupq_n_s32(0x80000000u);
  vec = vsetq_lane_s32(0, vec, 3);  // {sign,sign,sign,0}
  return vec;
}

OZZ_INLINE SimdInt4 mask_sign_w() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(0x80000000u, vec, 3);  // {0,0,0,sign}
  return vec;
}

OZZ_INLINE SimdInt4 mask_not_sign() {
  return vdupq_n_s32(0x7FFFFFFF);  // ~sign bit
}

OZZ_INLINE SimdInt4 mask_ffff() {
  return vdupq_n_s32(-1);  // all bits set
}

OZZ_INLINE SimdInt4 mask_0000() { return vdupq_n_s32(0); }

OZZ_INLINE SimdInt4 mask_fff0() {
  int32x4_t vec = vdupq_n_s32(-1);
  vec = vsetq_lane_s32(0, vec, 3);  // {FFFF,FFFF,FFFF,0000}
  return vec;
}

OZZ_INLINE SimdInt4 mask_f000() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(-1, vec, 0);  // {FFFF,0000,0000,0000}
  return vec;
}

OZZ_INLINE SimdInt4 mask_0f00() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(-1, vec, 1);  // {0000,FFFF,0000,0000}
  return vec;
}

OZZ_INLINE SimdInt4 mask_00f0() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(-1, vec, 2);  // {0000,0000,FFFF,0000}
  return vec;
}

OZZ_INLINE SimdInt4 mask_000f() {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(-1, vec, 3);  // {0000,0000,0000,FFFF}
  return vec;
}

OZZ_INLINE SimdInt4 Load(int _x, int _y, int _z, int _w) {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_x, vec, 0);
  vec = vsetq_lane_s32(_y, vec, 1);
  vec = vsetq_lane_s32(_z, vec, 2);
  vec = vsetq_lane_s32(_w, vec, 3);
  return vec;
}

OZZ_INLINE SimdInt4 LoadX(int _x) {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_x, vec, 0);  // {_x,0,0,0}
  return vec;
}

OZZ_INLINE SimdInt4 Load1(int _x) {
  return vdupq_n_s32(_x);  // {_x,_x,_x,_x}
}

// bool -> mask (0 or 0xFFFFFFFF)
OZZ_INLINE SimdInt4 Load(bool _x, bool _y, bool _z, bool _w) {
  int32_t vals[4] = {
      _x ? -1 : 0,
      _y ? -1 : 0,
      _z ? -1 : 0,
      _w ? -1 : 0,
  };
  return vld1q_s32(vals);
}

OZZ_INLINE SimdInt4 LoadX(bool _x) {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_x ? -1 : 0, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 Load1(bool _x) {
  int val = _x ? -1 : 0;
  return vdupq_n_s32(val);
}

OZZ_INLINE SimdInt4 LoadPtr(const int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  return vld1q_s32(reinterpret_cast<const int32_t*>(_i));
}

OZZ_INLINE SimdInt4 LoadXPtr(const int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_i[0], vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 Load1Ptr(const int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  return vdupq_n_s32(_i[0]);
}

OZZ_INLINE SimdInt4 Load2Ptr(const int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_i[0], vec, 0);
  vec = vsetq_lane_s32(_i[1], vec, 1);
  return vec;  // {i0, i1, 0, 0}
}

OZZ_INLINE SimdInt4 Load3Ptr(const int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_i[0], vec, 0);
  vec = vsetq_lane_s32(_i[1], vec, 1);
  vec = vsetq_lane_s32(_i[2], vec, 2);
  return vec;  // {i0, i1, i2, 0}
}

OZZ_INLINE SimdInt4 LoadPtrU(const int* _i) {
  return vld1q_s32(reinterpret_cast<const int32_t*>(_i));
}

OZZ_INLINE SimdInt4 LoadXPtrU(const int* _i) {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_i[0], vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 Load1PtrU(const int* _i) { return vdupq_n_s32(_i[0]); }

OZZ_INLINE SimdInt4 Load2PtrU(const int* _i) {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_i[0], vec, 0);
  vec = vsetq_lane_s32(_i[1], vec, 1);
  return vec;  // {i0, i1, 0, 0}
}

OZZ_INLINE SimdInt4 Load3PtrU(const int* _i) {
  int32x4_t vec = vdupq_n_s32(0);
  vec = vsetq_lane_s32(_i[0], vec, 0);
  vec = vsetq_lane_s32(_i[1], vec, 1);
  vec = vsetq_lane_s32(_i[2], vec, 2);
  return vec;  // {i0, i1, i2, 0}
}

OZZ_INLINE SimdInt4 FromFloatRound(_SimdFloat4 _f) { return vcvtq_s32_f32(_f); }

OZZ_INLINE SimdInt4 FromFloatTrunc(_SimdFloat4 _f) {
  float tmp[4];
  vst1q_f32(tmp, _f);
  int32_t out[4];
  out[0] = static_cast<int32_t>(tmp[0]);
  out[1] = static_cast<int32_t>(tmp[1]);
  out[2] = static_cast<int32_t>(tmp[2]);
  out[3] = static_cast<int32_t>(tmp[3]);
  return vld1q_s32(out);
}

}  // namespace simd_int4

OZZ_INLINE int GetX(_SimdInt4 _v) { return vgetq_lane_s32(_v, 0); }

OZZ_INLINE int GetY(_SimdInt4 _v) { return vgetq_lane_s32(_v, 1); }

OZZ_INLINE int GetZ(_SimdInt4 _v) { return vgetq_lane_s32(_v, 2); }

OZZ_INLINE int GetW(_SimdInt4 _v) { return vgetq_lane_s32(_v, 3); }

OZZ_INLINE SimdInt4 SetX(_SimdInt4 _v, _SimdInt4 _i) {
  return vsetq_lane_s32(GetX(_i), _v, 0);
}

OZZ_INLINE SimdInt4 SetY(_SimdInt4 _v, _SimdInt4 _i) {
  return vsetq_lane_s32(GetX(_i), _v, 1);
}

OZZ_INLINE SimdInt4 SetZ(_SimdInt4 _v, _SimdInt4 _i) {
  return vsetq_lane_s32(GetX(_i), _v, 2);
}

OZZ_INLINE SimdInt4 SetW(_SimdInt4 _v, _SimdInt4 _i) {
  return vsetq_lane_s32(GetX(_i), _v, 3);
}

OZZ_INLINE SimdInt4 SetI(_SimdInt4 _v, _SimdInt4 _i, int _ith) {
  assert(_ith >= 0 && _ith <= 3 && "Invalid index, out of range.");
  alignas(16) int32_t tmp[4];
  vst1q_s32(tmp, _v);
  tmp[_ith] = GetX(_i);
  return vld1q_s32(tmp);
}

OZZ_INLINE void StorePtr(_SimdInt4 _v, int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  vst1q_s32(reinterpret_cast<int32_t*>(_i), _v);
}

OZZ_INLINE void Store1Ptr(_SimdInt4 _v, int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  _i[0] = GetX(_v);
}

OZZ_INLINE void Store2Ptr(_SimdInt4 _v, int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  _i[0] = GetX(_v);
  _i[1] = GetY(_v);
}

OZZ_INLINE void Store3Ptr(_SimdInt4 _v, int* _i) {
  assert(!(reinterpret_cast<uintptr_t>(_i) & 0xf) && "Invalid alignment");
  _i[0] = GetX(_v);
  _i[1] = GetY(_v);
  _i[2] = GetZ(_v);
}

OZZ_INLINE void StorePtrU(_SimdInt4 _v, int* _i) {
  vst1q_s32(reinterpret_cast<int32_t*>(_i), _v);
}

OZZ_INLINE void Store1PtrU(_SimdInt4 _v, int* _i) { _i[0] = GetX(_v); }

OZZ_INLINE void Store2PtrU(_SimdInt4 _v, int* _i) {
  _i[0] = GetX(_v);
  _i[1] = GetY(_v);
}

OZZ_INLINE void Store3PtrU(_SimdInt4 _v, int* _i) {
  _i[0] = GetX(_v);
  _i[1] = GetY(_v);
  _i[2] = GetZ(_v);
}

OZZ_INLINE SimdInt4 SplatX(_SimdInt4 _a) { return vdupq_n_s32(GetX(_a)); }

OZZ_INLINE SimdInt4 SplatY(_SimdInt4 _a) { return vdupq_n_s32(GetY(_a)); }

OZZ_INLINE SimdInt4 SplatZ(_SimdInt4 _a) { return vdupq_n_s32(GetZ(_a)); }

OZZ_INLINE SimdInt4 SplatW(_SimdInt4 _a) { return vdupq_n_s32(GetW(_a)); }

template <size_t _X, size_t _Y, size_t _Z, size_t _W>
OZZ_INLINE SimdInt4 Swizzle(_SimdInt4 _v) {
  static_assert(_X <= 3 && _Y <= 3 && _Z <= 3 && _W <= 3,
                "Indices must be between 0 and 3");
  alignas(16) int32_t in[4];
  alignas(16) int32_t out[4];
  vst1q_s32(in, _v);
  out[0] = in[_X];
  out[1] = in[_Y];
  out[2] = in[_Z];
  out[3] = in[_W];
  return vld1q_s32(out);
}

template <>
OZZ_INLINE SimdInt4 Swizzle<0, 1, 2, 3>(_SimdInt4 _v) {
  return _v;
}

OZZ_INLINE int MoveMask(_SimdInt4 _v) {
  // Take sign bit from each lane and pack into 4-bit mask
  uint32x4_t u = vreinterpretq_u32_s32(_v);
  uint32_t x = vgetq_lane_u32(u, 0);
  uint32_t y = vgetq_lane_u32(u, 1);
  uint32_t z = vgetq_lane_u32(u, 2);
  uint32_t w = vgetq_lane_u32(u, 3);
  int mask = ((x >> 31) & 1) | (((y >> 31) & 1) << 1) | (((z >> 31) & 1) << 2) |
             (((w >> 31) & 1) << 3);
  return mask;
}

OZZ_INLINE bool AreAllTrue(_SimdInt4 _v) { return MoveMask(_v) == 0xf; }

OZZ_INLINE bool AreAllTrue3(_SimdInt4 _v) {
  return (MoveMask(_v) & 0x7) == 0x7;
}

OZZ_INLINE bool AreAllTrue2(_SimdInt4 _v) {
  return (MoveMask(_v) & 0x3) == 0x3;
}

OZZ_INLINE bool AreAllTrue1(_SimdInt4 _v) {
  return (MoveMask(_v) & 0x1) == 0x1;
}

OZZ_INLINE bool AreAllFalse(_SimdInt4 _v) { return MoveMask(_v) == 0; }

OZZ_INLINE bool AreAllFalse3(_SimdInt4 _v) { return (MoveMask(_v) & 0x7) == 0; }

OZZ_INLINE bool AreAllFalse2(_SimdInt4 _v) { return (MoveMask(_v) & 0x3) == 0; }

OZZ_INLINE bool AreAllFalse1(_SimdInt4 _v) { return (MoveMask(_v) & 0x1) == 0; }

OZZ_INLINE SimdInt4 HAdd2(_SimdInt4 _v) {
  int32_t sum = GetX(_v) + GetY(_v);
  SimdInt4 vec = _v;
  vec = vsetq_lane_s32(sum, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 HAdd3(_SimdInt4 _v) {
  int32_t sum = GetX(_v) + GetY(_v) + GetZ(_v);
  SimdInt4 vec = _v;
  vec = vsetq_lane_s32(sum, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 HAdd4(_SimdInt4 _v) {
  int32_t sum = GetX(_v) + GetY(_v) + GetZ(_v) + GetW(_v);
  SimdInt4 vec = _v;
  vec = vsetq_lane_s32(sum, vec, 0);
  return vec;
}

OZZ_INLINE SimdInt4 Abs(_SimdInt4 _v) { return vabsq_s32(_v); }

OZZ_INLINE SimdInt4 Sign(_SimdInt4 _v) {
  SimdInt4 neg_mask = vcltq_s32(_v, vdupq_n_s32(0));
  SimdInt4 sign_bit = vdupq_n_s32(0x80000000u);
  return vandq_s32(neg_mask, sign_bit);
}

OZZ_INLINE SimdInt4 Min(_SimdInt4 _a, _SimdInt4 _b) {
  return vminq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 Max(_SimdInt4 _a, _SimdInt4 _b) {
  return vmaxq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 Min0(_SimdInt4 _v) {
  SimdInt4 zero = vdupq_n_s32(0);
  return vminq_s32(zero, _v);
}

OZZ_INLINE SimdInt4 Max0(_SimdInt4 _v) {
  SimdInt4 zero = vdupq_n_s32(0);
  return vmaxq_s32(zero, _v);
}

OZZ_INLINE SimdInt4 Clamp(_SimdInt4 _a, _SimdInt4 _v, _SimdInt4 _b) {
  SimdInt4 tmp = vmaxq_s32(_a, _v);
  return vminq_s32(tmp, _b);
}

OZZ_INLINE SimdInt4 Select(_SimdInt4 _b, _SimdInt4 _true, _SimdInt4 _false) {
  return OZZ_NEON_SELECT_I(_b, _true, _false);
}

OZZ_INLINE SimdInt4 And(_SimdInt4 _a, _SimdInt4 _b) {
  return vandq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 AndNot(_SimdInt4 _a, _SimdInt4 _b) {
  return vbicq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 Or(_SimdInt4 _a, _SimdInt4 _b) { return vorrq_s32(_a, _b); }

OZZ_INLINE SimdInt4 Xor(_SimdInt4 _a, _SimdInt4 _b) {
  return veorq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 Not(_SimdInt4 _v) { return vmvnq_s32(_v); }

OZZ_INLINE SimdInt4 ShiftL(_SimdInt4 _v, int _bits) {
  return vshlq_s32(_v, vdupq_n_s32(_bits));
}

OZZ_INLINE SimdInt4 ShiftR(_SimdInt4 _v, int _bits) {
  return vshlq_s32(_v, vdupq_n_s32(-_bits));
}

OZZ_INLINE SimdInt4 ShiftRu(_SimdInt4 _v, int _bits) {
  uint32x4_t uv = vreinterpretq_u32_s32(_v);
  uv = vshlq_u32(uv, vdupq_n_s32(-_bits));
  return vreinterpretq_s32_u32(uv);
}

OZZ_INLINE SimdInt4 CmpEq(_SimdInt4 _a, _SimdInt4 _b) {
  return vceqq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 CmpNe(_SimdInt4 _a, _SimdInt4 _b) {
  SimdInt4 eq = vceqq_s32(_a, _b);
  return vmvnq_s32(eq);
}

OZZ_INLINE SimdInt4 CmpLt(_SimdInt4 _a, _SimdInt4 _b) {
  return vcltq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 CmpLe(_SimdInt4 _a, _SimdInt4 _b) {
  return vcleq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 CmpGt(_SimdInt4 _a, _SimdInt4 _b) {
  return vcgtq_s32(_a, _b);
}

OZZ_INLINE SimdInt4 CmpGe(_SimdInt4 _a, _SimdInt4 _b) {
  return vcgeq_s32(_a, _b);
}

OZZ_INLINE Float4x4 Float4x4::identity() {
  Float4x4 vec;
  vec.cols[0] = simd_float4::x_axis();
  vec.cols[1] = simd_float4::y_axis();
  vec.cols[2] = simd_float4::z_axis();
  vec.cols[3] = simd_float4::w_axis();
  return vec;
}

OZZ_INLINE Float4x4 Transpose(const Float4x4& _m) {
  SimdFloat4 in[4] = {_m.cols[0], _m.cols[1], _m.cols[2], _m.cols[3]};
  SimdFloat4 out[4];
  Transpose4x4(in, out);
  Float4x4 vec;
  vec.cols[0] = out[0];
  vec.cols[1] = out[1];
  vec.cols[2] = out[2];
  vec.cols[3] = out[3];
  return vec;
}

OZZ_INLINE Float4x4 Invert(const Float4x4& _m, SimdInt4* _invertible) {
  float m[16];
  for (int c = 0; c < 4; ++c) {
    float32x4_t col = _m.cols[c];
    m[0 + c] = vgetq_lane_f32(col, 0);
    m[4 + c] = vgetq_lane_f32(col, 1);
    m[8 + c] = vgetq_lane_f32(col, 2);
    m[12 + c] = vgetq_lane_f32(col, 3);
  }
  float inv[16];

  inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
           m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

  inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
           m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

  inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
           m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

  inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
            m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

  inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
           m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

  inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
           m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

  inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
           m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

  inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
            m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

  inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
           m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

  inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
           m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

  inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
            m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

  inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
            m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
           m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
           m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
            m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

  float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
  const float eps = 1e-8f;
  bool ok = std::fabs(det) > eps;
  SimdInt4 mask = vdupq_n_s32(0);
  if (ok) {
    mask = vsetq_lane_s32(-1, mask, 0);
    float inv_det = 1.0f / det;
    for (int i = 0; i < 16; ++i) {
      inv[i] *= inv_det;
    }
  } else {
    for (int i = 0; i < 16; ++i) inv[i] = 0.0f;
  }
  if (_invertible) {
    *_invertible = mask;
  } else {
    assert(ok && "Matrix is not invertible");
  }
  Float4x4 out;
  for (int c = 0; c < 4; ++c) {
    float32x4_t col = vdupq_n_f32(0.0f);
    col = vsetq_lane_f32(inv[0 + c], col, 0);
    col = vsetq_lane_f32(inv[4 + c], col, 1);
    col = vsetq_lane_f32(inv[8 + c], col, 2);
    col = vsetq_lane_f32(inv[12 + c], col, 3);
    out.cols[c] = col;
  }
  return out;
}

OZZ_INLINE Float4x4 Float4x4::Translation(_SimdFloat4 _v) {
  Float4x4 vec = Float4x4::identity();
  vec.cols[3] = SetW(_v, simd_float4::one());
  return vec;
}

OZZ_INLINE Float4x4 Float4x4::Scaling(_SimdFloat4 _v) {
  Float4x4 vec;
  vec.cols[0] = vmulq_f32(simd_float4::x_axis(), SplatX(_v));
  vec.cols[1] = vmulq_f32(simd_float4::y_axis(), SplatY(_v));
  vec.cols[2] = vmulq_f32(simd_float4::z_axis(), SplatZ(_v));
  vec.cols[3] = simd_float4::w_axis();
  return vec;
}

OZZ_INLINE Float4x4 Translate(const Float4x4& _m, _SimdFloat4 _v) {
  const SimdFloat4 a01 =
      MAdd(_m.cols[0], SplatX(_v), vmulq_f32(_m.cols[1], SplatY(_v)));
  const SimdFloat4 m3 = MAdd(_m.cols[2], SplatZ(_v), _m.cols[3]);
  Float4x4 vec;
  vec.cols[0] = _m.cols[0];
  vec.cols[1] = _m.cols[1];
  vec.cols[2] = _m.cols[2];
  vec.cols[3] = vaddq_f32(a01, m3);
  return vec;
}

OZZ_INLINE Float4x4 Scale(const Float4x4& _m, _SimdFloat4 _v) {
  Float4x4 vec;
  vec.cols[0] = vmulq_f32(_m.cols[0], SplatX(_v));
  vec.cols[1] = vmulq_f32(_m.cols[1], SplatY(_v));
  vec.cols[2] = vmulq_f32(_m.cols[2], SplatZ(_v));
  vec.cols[3] = _m.cols[3];
  return vec;
}

OZZ_INLINE Float4x4 ColumnMultiply(const Float4x4& _m, _SimdFloat4 _v) {
  Float4x4 vec;
  vec.cols[0] = vmulq_f32(_m.cols[0], _v);
  vec.cols[1] = vmulq_f32(_m.cols[1], _v);
  vec.cols[2] = vmulq_f32(_m.cols[2], _v);
  vec.cols[3] = vmulq_f32(_m.cols[3], _v);
  return vec;
}

OZZ_INLINE SimdInt4 IsNormalized(const Float4x4& _m) {
  const SimdInt4 n0 = IsNormalized3(_m.cols[0]);
  const SimdInt4 n1 = IsNormalized3(_m.cols[1]);
  const SimdInt4 n2 = IsNormalized3(_m.cols[2]);
  const bool ok = AreAllTrue1(n0) && AreAllTrue1(n1) && AreAllTrue1(n2);
  if (ok) {
    return simd_int4::mask_fff0();
  } else {
    return vdupq_n_s32(0);
  }
}

OZZ_INLINE SimdInt4 IsNormalizedEst(const Float4x4& _m) {
  const SimdInt4 n0 = IsNormalizedEst3(_m.cols[0]);
  const SimdInt4 n1 = IsNormalizedEst3(_m.cols[1]);
  const SimdInt4 n2 = IsNormalizedEst3(_m.cols[2]);
  const bool ok = AreAllTrue1(n0) && AreAllTrue1(n1) && AreAllTrue1(n2);
  if (ok) {
    return simd_int4::mask_fff0();
  } else {
    return vdupq_n_s32(0);
  }
}

OZZ_INLINE SimdInt4 IsOrthogonal(const Float4x4& _m) {
  const float max_v = 1.f + kNormalizationToleranceSq;
  const float min_v = 1.f - kNormalizationToleranceSq;
  const SimdFloat4 zero = simd_float4::zero();
  const SimdFloat4 cross = NormalizeSafe3(Cross3(_m.cols[0], _m.cols[1]), zero);
  const SimdFloat4 at = NormalizeSafe3(_m.cols[2], zero);
  const SimdFloat4 dot = Dot3(cross, at);
  const float d = GetX(dot);
  const bool ok = (d < max_v) && (d > min_v);
  SimdInt4 mask = vdupq_n_s32(0);
  if (ok) {
    mask = vsetq_lane_s32(-1, mask, 0);  // lane 0 = true
  }
  return mask;
}

OZZ_INLINE SimdFloat4 ToQuaternion(const Float4x4& _m) {
  float m00 = vgetq_lane_f32(_m.cols[0], 0);
  float m01 = vgetq_lane_f32(_m.cols[1], 0);
  float m02 = vgetq_lane_f32(_m.cols[2], 0);
  float m10 = vgetq_lane_f32(_m.cols[0], 1);
  float m11 = vgetq_lane_f32(_m.cols[1], 1);
  float m12 = vgetq_lane_f32(_m.cols[2], 1);
  float m20 = vgetq_lane_f32(_m.cols[0], 2);
  float m21 = vgetq_lane_f32(_m.cols[1], 2);
  float m22 = vgetq_lane_f32(_m.cols[2], 2);
  float qw, qx, qy, qz;
  float trace = m00 + m11 + m22;
  if (trace > 0.0f) {
    float s = std::sqrt(trace + 1.0f) * 2.0f;
    qw = 0.25f * s;
    qx = (m21 - m12) / s;
    qy = (m02 - m20) / s;
    qz = (m10 - m01) / s;
  } else if (m00 > m11 && m00 > m22) {
    float s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
    qw = (m21 - m12) / s;
    qx = 0.25f * s;
    qy = (m01 + m10) / s;
    qz = (m02 + m20) / s;
  } else if (m11 > m22) {
    float s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
    qw = (m02 - m20) / s;
    qx = (m01 + m10) / s;
    qy = 0.25f * s;
    qz = (m12 + m21) / s;
  } else {
    float s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
    qw = (m10 - m01) / s;
    qx = (m02 + m20) / s;
    qy = (m12 + m21) / s;
    qz = 0.25f * s;
  }
  SimdFloat4 q = simd_float4::zero();
  q = vsetq_lane_f32(qx, q, 0);
  q = vsetq_lane_f32(qy, q, 1);
  q = vsetq_lane_f32(qz, q, 2);
  q = vsetq_lane_f32(qw, q, 3);
  assert(AreAllTrue1(IsNormalizedEst4(q)));
  return q;
}

OZZ_INLINE bool ToAffine(const Float4x4& _m, SimdFloat4* _translation,
                         SimdFloat4* _quaternion, SimdFloat4* _scale) {
  const float ortho_tol = kOrthogonalisationToleranceSq;
  {
    float32x4_t t = _m.cols[3];
    t = vsetq_lane_f32(1.0f, t, 3);
    *_translation = t;
  }
  float32x4_t m_row0 = vdupq_n_f32(0.0f);
  float32x4_t m_row1 = vdupq_n_f32(0.0f);
  float32x4_t m_row2 = vdupq_n_f32(0.0f);
  m_row0 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[0], 0), m_row0, 0);
  m_row0 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[1], 0), m_row0, 1);
  m_row0 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[2], 0), m_row0, 2);
  m_row1 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[0], 1), m_row1, 0);
  m_row1 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[1], 1), m_row1, 1);
  m_row1 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[2], 1), m_row1, 2);
  m_row2 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[0], 2), m_row2, 0);
  m_row2 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[1], 2), m_row2, 1);
  m_row2 = vsetq_lane_f32(vgetq_lane_f32(_m.cols[2], 2), m_row2, 2);
  auto row_len2 = [](float32x4_t vec) -> float {
    const float x = vgetq_lane_f32(vec, 0);
    const float y = vgetq_lane_f32(vec, 1);
    const float z = vgetq_lane_f32(vec, 2);
    return x * x + y * y + z * z;
  };
  float len2[3] = {
      row_len2(m_row0),
      row_len2(m_row1),
      row_len2(m_row2),
  };
  float abs_scale[3] = {
      std::sqrt(len2[0]),
      std::sqrt(len2[1]),
      std::sqrt(len2[2]),
  };
  int mask = 0;
  if (len2[0] < ortho_tol) mask |= 1;
  if (len2[1] < ortho_tol) mask |= 2;
  if (len2[2] < ortho_tol) mask |= 4;
  Float4x4 orthonormal;
  if (mask & 1) {
    if (mask & 6) {
      return false;
    }
    float inv_s1 = 1.0f / abs_scale[1];
    orthonormal.cols[1] = vmulq_n_f32(_m.cols[1], inv_s1);
    orthonormal.cols[0] = Normalize3(Cross3(orthonormal.cols[1], _m.cols[2]));
    orthonormal.cols[2] =
        Normalize3(Cross3(orthonormal.cols[0], orthonormal.cols[1]));
  } else if (mask & 4) {
    if (mask & 3) {
      return false;
    }
    float inv_s0 = 1.0f / abs_scale[0];
    orthonormal.cols[0] = vmulq_n_f32(_m.cols[0], inv_s0);
    orthonormal.cols[2] = Normalize3(Cross3(orthonormal.cols[0], _m.cols[1]));
    orthonormal.cols[1] =
        Normalize3(Cross3(orthonormal.cols[2], orthonormal.cols[0]));
  } else {
    if (mask & 5) {
      return false;
    }
    float inv_s2 = 1.0f / abs_scale[2];
    orthonormal.cols[2] = vmulq_n_f32(_m.cols[2], inv_s2);
    orthonormal.cols[1] = Normalize3(Cross3(orthonormal.cols[2], _m.cols[0]));
    orthonormal.cols[0] =
        Normalize3(Cross3(orthonormal.cols[1], orthonormal.cols[2]));
  }
  orthonormal.cols[3] = simd_float4::w_axis();
  float32x4_t o_row0 = vdupq_n_f32(0.0f);
  float32x4_t o_row1 = vdupq_n_f32(0.0f);
  float32x4_t o_row2 = vdupq_n_f32(0.0f);
  o_row0 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[0], 0), o_row0, 0);
  o_row0 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[1], 0), o_row0, 1);
  o_row0 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[2], 0), o_row0, 2);
  o_row1 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[0], 1), o_row1, 0);
  o_row1 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[1], 1), o_row1, 1);
  o_row1 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[2], 1), o_row1, 2);
  o_row2 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[0], 2), o_row2, 0);
  o_row2 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[1], 2), o_row2, 1);
  o_row2 = vsetq_lane_f32(vgetq_lane_f32(orthonormal.cols[2], 2), o_row2, 2);
  auto row_dot = [](float32x4_t a, float32x4_t b) -> float {
    const float ax = vgetq_lane_f32(a, 0);
    const float ay = vgetq_lane_f32(a, 1);
    const float az = vgetq_lane_f32(a, 2);
    const float bx = vgetq_lane_f32(b, 0);
    const float by = vgetq_lane_f32(b, 1);
    const float bz = vgetq_lane_f32(b, 2);
    return ax * bx + ay * by + az * bz;
  };
  float scale_dot[3] = {
      row_dot(o_row0, m_row0),
      row_dot(o_row1, m_row1),
      row_dot(o_row2, m_row2),
  };
  float scale_s[3] = {
      (scale_dot[0] > 0.0f ? abs_scale[0] : -abs_scale[0]),
      (scale_dot[1] > 0.0f ? abs_scale[1] : -abs_scale[1]),
      (scale_dot[2] > 0.0f ? abs_scale[2] : -abs_scale[2]),
  };
  float32x4_t scale_vec = vdupq_n_f32(0.0f);
  scale_vec = vsetq_lane_f32(scale_s[0], scale_vec, 0);
  scale_vec = vsetq_lane_f32(scale_s[1], scale_vec, 1);
  scale_vec = vsetq_lane_f32(scale_s[2], scale_vec, 2);
  scale_vec = vsetq_lane_f32(1.0f, scale_vec, 3);
  *_scale = scale_vec;
  *_quaternion = ToQuaternion(orthonormal);
  return true;
}

OZZ_INLINE Float4x4 Float4x4::FromEuler(_SimdFloat4 _v) {
  return Float4x4::FromAxisAngle(simd_float4::y_axis(), SplatX(_v)) *
         Float4x4::FromAxisAngle(simd_float4::x_axis(), SplatY(_v)) *
         Float4x4::FromAxisAngle(simd_float4::z_axis(), SplatZ(_v));
}

OZZ_INLINE Float4x4 Float4x4::FromAxisAngle(_SimdFloat4 _axis,
                                            _SimdFloat4 _angle) {
  assert(AreAllTrue1(IsNormalizedEst3(_axis)));
  const float x = GetX(_axis);
  const float y = GetY(_axis);
  const float z = GetZ(_axis);
  const float angle = GetX(_angle);
  const float s = std::sin(angle);
  const float c = std::cos(angle);
  const float one_minus_c = 1.0f - c;
  float m00 = c + x * x * one_minus_c;
  float m01 = x * y * one_minus_c - z * s;
  float m02 = x * z * one_minus_c + y * s;
  float m10 = y * x * one_minus_c + z * s;
  float m11 = c + y * y * one_minus_c;
  float m12 = y * z * one_minus_c - x * s;
  float m20 = z * x * one_minus_c - y * s;
  float m21 = z * y * one_minus_c + x * s;
  float m22 = c + z * z * one_minus_c;
  Float4x4 out;
  out.cols[0] = vdupq_n_f32(0.0f);
  out.cols[0] = vsetq_lane_f32(m00, out.cols[0], 0);
  out.cols[0] = vsetq_lane_f32(m10, out.cols[0], 1);
  out.cols[0] = vsetq_lane_f32(m20, out.cols[0], 2);
  out.cols[0] = vsetq_lane_f32(0.0f, out.cols[0], 3);
  out.cols[1] = vdupq_n_f32(0.0f);
  out.cols[1] = vsetq_lane_f32(m01, out.cols[1], 0);
  out.cols[1] = vsetq_lane_f32(m11, out.cols[1], 1);
  out.cols[1] = vsetq_lane_f32(m21, out.cols[1], 2);
  out.cols[1] = vsetq_lane_f32(0.0f, out.cols[1], 3);
  out.cols[2] = vdupq_n_f32(0.0f);
  out.cols[2] = vsetq_lane_f32(m02, out.cols[2], 0);
  out.cols[2] = vsetq_lane_f32(m12, out.cols[2], 1);
  out.cols[2] = vsetq_lane_f32(m22, out.cols[2], 2);
  out.cols[2] = vsetq_lane_f32(0.0f, out.cols[2], 3);
  out.cols[3] = simd_float4::w_axis();
  return out;
}

inline Float4x4 Float4x4::FromQuaternion(_SimdFloat4 _q) {
  assert(AreAllTrue1(IsNormalizedEst4(_q)));
  const float x = GetX(_q);
  const float y = GetY(_q);
  const float z = GetZ(_q);
  const float w = GetW(_q);
  const float xx = x * x;
  const float yy = y * y;
  const float zz = z * z;
  const float xy = x * y;
  const float xz = x * z;
  const float yz = y * z;
  const float wx = w * x;
  const float wy = w * y;
  const float wz = w * z;
  float m00 = 1.0f - 2.0f * (yy + zz);
  float m01 = 2.0f * (xy - wz);
  float m02 = 2.0f * (xz + wy);
  float m10 = 2.0f * (xy + wz);
  float m11 = 1.0f - 2.0f * (xx + zz);
  float m12 = 2.0f * (yz - wx);
  float m20 = 2.0f * (xz - wy);
  float m21 = 2.0f * (yz + wx);
  float m22 = 1.0f - 2.0f * (xx + yy);
  Float4x4 out;
  out.cols[0] = vdupq_n_f32(0.0f);
  out.cols[0] = vsetq_lane_f32(m00, out.cols[0], 0);
  out.cols[0] = vsetq_lane_f32(m10, out.cols[0], 1);
  out.cols[0] = vsetq_lane_f32(m20, out.cols[0], 2);
  out.cols[0] = vsetq_lane_f32(0.0f, out.cols[0], 3);
  out.cols[1] = vdupq_n_f32(0.0f);
  out.cols[1] = vsetq_lane_f32(m01, out.cols[1], 0);
  out.cols[1] = vsetq_lane_f32(m11, out.cols[1], 1);
  out.cols[1] = vsetq_lane_f32(m21, out.cols[1], 2);
  out.cols[1] = vsetq_lane_f32(0.0f, out.cols[1], 3);
  out.cols[2] = vdupq_n_f32(0.0f);
  out.cols[2] = vsetq_lane_f32(m02, out.cols[2], 0);
  out.cols[2] = vsetq_lane_f32(m12, out.cols[2], 1);
  out.cols[2] = vsetq_lane_f32(m22, out.cols[2], 2);
  out.cols[2] = vsetq_lane_f32(0.0f, out.cols[2], 3);
  out.cols[3] = simd_float4::w_axis();
  return out;
}

inline Float4x4 Float4x4::FromAffine(_SimdFloat4 _translation,
                                     _SimdFloat4 _quaternion,
                                     _SimdFloat4 _scale) {
  assert(AreAllTrue1(IsNormalizedEst4(_quaternion)));
  Float4x4 rot = Float4x4::FromQuaternion(_quaternion);
  Float4x4 out;
  out.cols[0] = vmulq_f32(rot.cols[0], SplatX(_scale));
  out.cols[1] = vmulq_f32(rot.cols[1], SplatY(_scale));
  out.cols[2] = vmulq_f32(rot.cols[2], SplatZ(_scale));
  SimdFloat4 t = _translation;
  t = SetW(t, simd_float4::one());
  out.cols[3] = t;
  return out;
}

OZZ_INLINE ozz::math::SimdFloat4 TransformPoint(const ozz::math::Float4x4& _m,
                                                ozz::math::_SimdFloat4 _v) {
  const SimdFloat4 xxxx = vmulq_f32(SplatX(_v), _m.cols[0]);
  const SimdFloat4 a23 = MAdd(SplatZ(_v), _m.cols[2], _m.cols[3]);
  const SimdFloat4 a01 = MAdd(SplatY(_v), _m.cols[1], xxxx);
  return vaddq_f32(a01, a23);
}

OZZ_INLINE ozz::math::SimdFloat4 TransformVector(const ozz::math::Float4x4& _m,
                                                 ozz::math::_SimdFloat4 _v) {
  const SimdFloat4 xxxx = vmulq_f32(_m.cols[0], SplatX(_v));
  const SimdFloat4 zzzz = vmulq_f32(_m.cols[1], SplatY(_v));
  const SimdFloat4 a21 = MAdd(_m.cols[2], SplatZ(_v), xxxx);
  return vaddq_f32(zzzz, a21);
}

OZZ_INLINE ozz::math::SimdFloat4 operator*(const ozz::math::Float4x4& _m,
                                           ozz::math::_SimdFloat4 _v) {
  const SimdFloat4 xxxx = vmulq_f32(SplatX(_v), _m.cols[0]);
  const SimdFloat4 zzzz = vmulq_f32(SplatZ(_v), _m.cols[2]);
  const SimdFloat4 a01 = MAdd(SplatY(_v), _m.cols[1], xxxx);
  const SimdFloat4 a23 = MAdd(SplatW(_v), _m.cols[3], zzzz);
  return vaddq_f32(a01, a23);
}

OZZ_INLINE ozz::math::Float4x4 operator*(const ozz::math::Float4x4& _a,
                                         const ozz::math::Float4x4& _b) {
  Float4x4 vec;
  for (int i = 0; i < 4; ++i) {
    const SimdFloat4 xxxx = vmulq_f32(SplatX(_b.cols[i]), _a.cols[0]);
    const SimdFloat4 zzzz = vmulq_f32(SplatZ(_b.cols[i]), _a.cols[2]);
    const SimdFloat4 a01 = MAdd(SplatY(_b.cols[i]), _a.cols[1], xxxx);
    const SimdFloat4 a23 = MAdd(SplatW(_b.cols[i]), _a.cols[3], zzzz);
    vec.cols[i] = vaddq_f32(a01, a23);
  }
  return vec;
}

OZZ_INLINE ozz::math::Float4x4 operator+(const ozz::math::Float4x4& _a,
                                         const ozz::math::Float4x4& _b) {
  ozz::math::Float4x4 vec;
  vec.cols[0] = vaddq_f32(_a.cols[0], _b.cols[0]);
  vec.cols[1] = vaddq_f32(_a.cols[1], _b.cols[1]);
  vec.cols[2] = vaddq_f32(_a.cols[2], _b.cols[2]);
  vec.cols[3] = vaddq_f32(_a.cols[3], _b.cols[3]);
  return vec;
}

OZZ_INLINE ozz::math::Float4x4 operator-(const ozz::math::Float4x4& _a,
                                         const ozz::math::Float4x4& _b) {
  ozz::math::Float4x4 vec;
  vec.cols[0] = vsubq_f32(_a.cols[0], _b.cols[0]);
  vec.cols[1] = vsubq_f32(_a.cols[1], _b.cols[1]);
  vec.cols[2] = vsubq_f32(_a.cols[2], _b.cols[2]);
  vec.cols[3] = vsubq_f32(_a.cols[3], _b.cols[3]);
  return vec;
}

}  // namespace math
}  // namespace ozz

// On NEON primitives are not considered C++ types so these dont compile
#define OZZ_DISABLE_NEON_NATIVE_OPERATORS

// #if !defined(OZZ_DISABLE_NEON_NATIVE_OPERATORS)
// OZZ_INLINE ozz::math::SimdFloat4 operator+(ozz::math::_SimdFloat4 _a,
//                                            ozz::math::_SimdFloat4 _b) {
//   return vaddq_f32(_a, _b);
// }

// OZZ_INLINE ozz::math::SimdFloat4 operator-(ozz::math::_SimdFloat4 _a,
//                                            ozz::math::_SimdFloat4 _b) {
//   return vsubq_f32(_a, _b);
// }

// OZZ_INLINE ozz::math::SimdFloat4 operator-(ozz::math::_SimdFloat4 _v) {
//   return vnegq_f32(_v);
// }

// OZZ_INLINE ozz::math::SimdFloat4 operator*(ozz::math::_SimdFloat4 _a,
//                                            ozz::math::_SimdFloat4 _b) {
//   return vmulq_f32(_a, _b);
// }

// OZZ_INLINE ozz::math::SimdFloat4 operator/(ozz::math::_SimdFloat4 _a,
//                                            ozz::math::_SimdFloat4 _b) {
// #if defined(__aarch64__)
//   return vdivq_f32(_a, _b);
// #else
//   return vmulq_f32(_a, ozz::math::simd_float4::RcpEstNR(_b));
// #endif
// }
// #endif  // !defined(OZZ_DISABLE_SSE_NATIVE_OPERATORS)

namespace ozz {
namespace math {

OZZ_INLINE uint16_t FloatToHalf(float f) {
  union {
    float f;
    uint32_t u;
  } vec = {f};
  uint32_t x = vec.u;
  uint32_t sign = (x >> 16) & 0x8000u;                 // sign in bit 15
  uint32_t mant = x & 0x007FFFFFu;                     // mantissa
  int32_t exp = int32_t((x >> 23) & 0xFF) - 127 + 15;  // exponent bias adjust
  if (exp <= 0) {
    if (exp < -10) {
      return (uint16_t)sign;
    }
    mant |= 0x00800000u;
    uint32_t t = mant >> (1 - exp + 13);
    if (mant & (1u << (1 - exp + 12))) {
      t += 1;
    }
    return (uint16_t)(sign | t);
  } else if (exp >= 31) {
    if (mant == 0) {
      return (uint16_t)(sign | 0x7C00u);
    } else {
      return (uint16_t)(sign | 0x7C00u | (mant ? 0x0200u : 0));
    }
  } else {
    uint32_t h = (uint32_t(exp) << 10) | (mant >> 13);
    if (mant & 0x00001000u) {
      h += 1;
    }
    return (uint16_t)(sign | h);
  }
}

OZZ_INLINE float HalfToFloat(uint16_t h) {
  uint32_t sign = (h & 0x8000u) << 16;
  uint32_t exp = (h >> 10) & 0x1Fu;
  uint32_t mant = h & 0x03FFu;
  uint32_t out;
  if (exp == 0) {
    if (mant == 0) {
      out = sign;
    } else {
      int e = -1;
      do {
        e++;
        mant <<= 1;
      } while ((mant & 0x0400u) == 0);
      mant &= 0x03FFu;
      exp = uint32_t(127 - 15 - e);
      out = sign | (exp << 23) | (mant << 13);
    }
  } else if (exp == 0x1F) {
    out = sign | 0x7F800000u | (mant ? (mant << 13) : 0);
  } else {
    exp = exp + (127 - 15);
    out = sign | (exp << 23) | (mant << 13);
  }
  union {
    uint32_t u;
    float f;
  } vec = {out};
  return vec.f;
}

OZZ_INLINE SimdInt4 FloatToHalf(_SimdFloat4 _f) {
  uint16_t h[4];
  float tmp[4];
  vst1q_f32(tmp, _f);
  h[0] = FloatToHalf(tmp[0]);
  h[1] = FloatToHalf(tmp[1]);
  h[2] = FloatToHalf(tmp[2]);
  h[3] = FloatToHalf(tmp[3]);
  int32x4_t out = vdupq_n_s32(0);
  out = vsetq_lane_s32((int32_t)h[0], out, 0);
  out = vsetq_lane_s32((int32_t)h[1], out, 1);
  out = vsetq_lane_s32((int32_t)h[2], out, 2);
  out = vsetq_lane_s32((int32_t)h[3], out, 3);
  return out;
}

OZZ_INLINE SimdFloat4 HalfToFloat(_SimdInt4 _h) {
  uint16_t h[4];
  int32_t tmp_i[4];
  vst1q_s32(tmp_i, _h);
  h[0] = (uint16_t)(tmp_i[0] & 0xFFFF);
  h[1] = (uint16_t)(tmp_i[1] & 0xFFFF);
  h[2] = (uint16_t)(tmp_i[2] & 0xFFFF);
  h[3] = (uint16_t)(tmp_i[3] & 0xFFFF);
  float out_f[4];
  out_f[0] = HalfToFloat(h[0]);
  out_f[1] = HalfToFloat(h[1]);
  out_f[2] = HalfToFloat(h[2]);
  out_f[3] = HalfToFloat(h[3]);
  return vld1q_f32(out_f);
}

}  // namespace math
}  // namespace ozz

#undef OZZ_NEON_SPLAT_F
#undef OZZ_NEON_SPLAT_I
#if defined(__aarch64__)
#undef OZZ_NEON_UNPACKLO_PS
#undef OZZ_NEON_UNPACKHI_PS
#endif
#undef OZZ_NEON_MOVELH_PS
#undef OZZ_NEON_MOVEHL_PS
#undef OZZ_NEON_MOVE_SS
#undef OZZ_NEON_SELECT_F
#endif
