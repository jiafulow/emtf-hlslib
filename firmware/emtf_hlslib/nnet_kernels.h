#ifndef __EMTF_HLSLIB_NNET_KERNELS_H__
#define __EMTF_HLSLIB_NNET_KERNELS_H__

#include <cmath>  // provides std::tanh, std::pow, std::round

// EMTF HLS
#include "layer_helpers.h"

namespace emtf {

namespace phase2 {

namespace detail {

template <unsigned int N, typename T, typename U>
void init_nnet_weights_op(T* arr, U op) {
  static_assert(is_ap_fixed_type<T>::value, "T type check failed");

  for (unsigned i = 0; i < N; i++) {
    // Cast from int to ap_int
    ap_int<T::width> w = op(i);
    // Reinterpret ap_int as ap_fixed
    arr[i].range() = w.range();
  }
}

template <unsigned int N, typename T_IN, typename T_OUT>
void init_tanh_table_op(T_OUT table[N]) {
  static_assert(is_ap_fixed_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");
  static_assert(N == (1u << T_IN::width), "N value check failed");

  constexpr int W_OUT = T_OUT::width;
  constexpr int I_OUT = T_OUT::iwidth;
  constexpr int F_OUT = W_OUT - I_OUT;

  T_IN x = 0;

  for (unsigned i = 0; i < N; i++) {
    // Cast from unsigned to ap_uint
    ap_uint<T_IN::width> w = i;
    // Reinterpret ap_uint as ap_fixed
    x.range() = w.range();
    // Cast from ap_fixed T_IN to float32, call tanh(), cast to ap_fixed T_OUT
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::tanh(x_f32);
    float q_f32 = std::pow(2.f, -F_OUT);
    table[i] = static_cast<T_OUT>(std::round(y_f32 / q_f32) * q_f32);
  }
}

// Returns activation(X) using tanh
template <unsigned int N, typename T_IN, typename T_OUT>
void vector_tanh_activate_op(const T_IN x[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = fullyconnect_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int N_TABLE = (1u << T_IN::width);

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static T_OUT tanh_table[N_TABLE];
#else
  bool initialized = false;
  T_OUT tanh_table[N_TABLE];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    init_tanh_table_op<N_TABLE, T_IN>(tanh_table);
  }

LOOP_ACT:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Reinterpret ap_fixed as ap_uint
    const T_IN x_i = x[i];
    const ap_uint<T_IN::width> index = x_i.range();
    emtf_assert(index < N_TABLE);
    out[i] = tanh_table[index];
  }
}

// Returns static_cast<T_OUT>(x)
template <unsigned int N, typename T_IN, typename T_OUT>
void vector_cast_op(const T_IN x[N], T_OUT out[N]) {
  static_assert(is_ap_int_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");
  static_assert(T_IN::width == T_OUT::width, "T_OUT type check faild");

  // hls-pragmas begin
#pragma HLS PIPELINE II = fullyconnect_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

LOOP_CAST:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Cast from ap_int to ap_fixed
    const T_IN x_i = x[i];
    out[i] = static_cast<T_OUT>(x_i);
  }
}

// Returns X * Y
template <unsigned int N, typename T_IN0, typename T_IN1, typename T_OUT>
void vec_vec_mult_op(const T_IN0 x[N], const T_IN1 y[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN0>::value, "T_IN0 type check failed");
  static_assert(is_ap_fixed_type<T_IN1>::value, "T_IN1 type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = fullyconnect_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  constexpr int W_MULT = T_OUT::width;
  constexpr int I_MULT = T_OUT::iwidth;

  ap_fixed<W_MULT, I_MULT> mult[N];

  // hls-pragmas begin
#pragma HLS RESOURCE variable = mult core = DSP48 latency = 1
  // hls-pragmas end

  // Multiply
LOOP_MULT_1:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const T_IN0 x_i = x[i];
    const T_IN1 y_i = y[i];
    mult[i] = x_i * y_i;  // using DSP48
    out[i] = mult[i];
  }
}

// Returns dot(X, Y) + Z
// X has rows = N, Y has rows = N, Z is a scalar
template <unsigned int N, typename T_IN0, typename T_IN1, typename T_IN2, typename T_OUT>
void vec_vec_mult_biasadd_op(const T_IN0 x[N], const T_IN1 y[N], const T_IN2& z, T_OUT& out) {
  static_assert(is_ap_fixed_type<T_IN0>::value, "T_IN0 type check failed");
  static_assert(is_ap_fixed_type<T_IN1>::value, "T_IN1 type check failed");
  static_assert(is_ap_fixed_type<T_IN2>::value, "T_IN2 type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = fullyconnect_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  constexpr int W_MULT = AP_MIN(T_IN0::width + T_IN1::width, 24);  // capped at 24 bits (found empirically)
  constexpr int I_MULT = (T_IN0::iwidth + T_IN1::iwidth);
  constexpr int W_ACCUM = W_MULT + 4;  // additional 4 bits to prevent overflow (for 15<=x<31 accum terms)
  constexpr int I_ACCUM = I_MULT + 4;
  constexpr int W_OUT = T_OUT::width;
  constexpr int I_OUT = T_OUT::iwidth;

  ap_fixed<W_MULT, I_MULT> mult[N];

  // hls-pragmas begin
#pragma HLS RESOURCE variable = mult core = DSP48 latency = 1
  // hls-pragmas end

  // Multiply
LOOP_MULT_2:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const T_IN0 x_i = x[i];
    const T_IN1 y_i = y[i];
    mult[i] = x_i * y_i;  // using DSP48
  }

  ap_fixed<W_ACCUM, I_ACCUM> accum = z;  // init with the bias term

  // hls-pragmas begin
  //#pragma HLS RESOURCE variable=accum core=AddSubnS latency=2
  // hls-pragmas end

  // Accumulate
LOOP_ACCUM_2:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    accum += mult[i];
  }

  // Round and saturate
  out = ap_fixed<W_OUT, I_OUT, AP_RND, AP_SAT>(accum);

  // Sanity check
#ifndef __SYNTHESIS__
  {
    float f_accum = static_cast<float>(z);
    for (unsigned i = 0; i < N; i++) {
      f_accum += static_cast<float>(x[i]) * static_cast<float>(y[i]);
    }
    emtf_assert(std::abs(f_accum) < static_cast<float>(1 << (I_ACCUM - 1)));  // make sure no overflow
  }
#endif  // __SYNTHESIS__ not defined
}

// Returns matmul(X, Y) + Z
// X has rows = M, Y has (rows, cols) = (M, N) Z has cols = N
template <unsigned int M, unsigned int N, typename T_IN0, typename T_IN1, typename T_IN2, typename T_OUT>
void mat_vec_mult_biasadd_op(const T_IN0 x[M], const T_IN1 y[M * N], const T_IN2 z[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN0>::value, "T_IN0 type check failed");
  static_assert(is_ap_fixed_type<T_IN1>::value, "T_IN1 type check failed");
  static_assert(is_ap_fixed_type<T_IN2>::value, "T_IN2 type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = fullyconnect_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  constexpr int W_MULT = AP_MIN(T_IN0::width + T_IN1::width, 24);  // capped at 24 bits (found empirically)
  constexpr int I_MULT = (T_IN0::iwidth + T_IN1::iwidth);
  constexpr int W_ACCUM = W_MULT + 4;  // additional 4 bits to prevent overflow (for 15<=x<31 accum terms)
  constexpr int I_ACCUM = I_MULT + 4;
  constexpr int W_OUT = T_OUT::width;
  constexpr int I_OUT = T_OUT::iwidth;

LOOP_MULT_3:
  for (unsigned j = 0; j < N; j++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    ap_fixed<W_MULT, I_MULT> mult[M];

    // hls-pragmas begin
#pragma HLS RESOURCE variable = mult core = DSP48 latency = 1
    // hls-pragmas end

    // Multiply
  LOOP_MULT_3_1:
    for (unsigned i = 0; i < M; i++) {
      // hls-pragmas begin
#pragma HLS UNROLL
      // hls-pragmas end

      const T_IN0 x_i = x[i];
      const T_IN1 y_i = y[(j * M) + i];
      mult[i] = x_i * y_i;  // using DSP48
    }

    ap_fixed<W_ACCUM, I_ACCUM> accum = z[j];  // init with the bias term

    // hls-pragmas begin
    //#pragma HLS RESOURCE variable=accum core=AddSubnS latency=2
    // hls-pragmas end

    // Accumulate
  LOOP_ACCUM_3_1:
    for (unsigned i = 0; i < M; i++) {
      // hls-pragmas begin
#pragma HLS UNROLL
      // hls-pragmas end

      accum += mult[i];
    }

    // Round and saturate
    out[j] = ap_fixed<W_OUT, I_OUT, AP_RND, AP_SAT>(accum);
  }

  // Sanity check
#ifndef __SYNTHESIS__
  for (unsigned j = 0; j < N; j++) {
    float f_accum = static_cast<float>(z[j]);
    for (unsigned i = 0; i < M; i++) {
      f_accum += static_cast<float>(x[i]) * static_cast<float>(y[(i * N) + j]);
    }
    emtf_assert(std::abs(f_accum) < static_cast<float>(1 << (I_ACCUM - 1)));  // make sure no overflow
  }
#endif  // __SYNTHESIS__ not defined
}

}  // namespace detail

}  // namespace phase2

}  // namespace emtf

#endif  // __EMTF_HLSLIB_NNET_KERNELS_H__ not defined
