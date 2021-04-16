#ifndef __EMTF_HLSLIB_NNET_KERNELS_H__
#define __EMTF_HLSLIB_NNET_KERNELS_H__

#include <cmath>  // provides std::tanh, std::pow, std::round

namespace emtf {

namespace details {

template <unsigned int N, typename T_IN, typename T_OUT>
void init_nnet_weights_op(const T_IN w[N], T_OUT table[N]) {
  static_assert(is_same<T_IN, int>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");
  constexpr int W_OUT = T_OUT::width;

  for (unsigned i = 0; i < N; i++) {
    // Cast from integer to ap_int
    ap_int<W_OUT> w_i = w[i];
    // Reinterpret an integer w_i as fixed point T_OUT
    table[i].range() = w_i.range();
  }
}

template <unsigned int N, typename T_IN, typename T_OUT>
void init_tanh_table_op(T_OUT table[N]) {
  static_assert(is_ap_fixed_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");
  constexpr int F_OUT = find_ap_fixed_fwidth<T_OUT>::value;

  T_IN x = 0;

  for (unsigned i = 0; i < N; i++) {
    // Reinterpret an integer i as fixed point T_IN
    x.range() = i;
    // Cast from fixed point T_IN to float32, call tanh(), cast to fixed point T_OUT
    float x_f32 = static_cast<float>(x);
    float y_f32 = std::tanh(x_f32);
    float q_f32 = std::pow(2.f, -F_OUT);
    table[i] = static_cast<T_OUT>(std::round(y_f32 / q_f32) * q_f32);
  }
}

// Returns activation(X) using tanh
template <unsigned int N, typename T_IN, typename T_OUT>
void vector_activate(const T_IN x[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const unsigned int N_TABLE = (1u << T_IN::width);

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static T_OUT tanh_table[N_TABLE];
#else
  bool initialized = false;
  T_OUT tanh_table[N_TABLE];
#endif

  if (!initialized) {
    init_tanh_table_op<N_TABLE, T_IN>(tanh_table);
    initialized = true;
  }

  ap_uint<T_IN::width> index = 0;

  LOOP_ACT: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const T_IN x_i = x[i];
    const ap_uint<T_IN::width> index = x_i.range();
    out[i] = tanh_table[index];
  }
}

// Returns static_cast<T_OUT>(x)
template <unsigned int N, typename T_IN, typename T_OUT>
void vector_cast(const T_IN x[N], T_OUT out[N]) {
  static_assert(is_ap_int_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  LOOP_CAST: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const T_IN x_i = x[i];
    out[i] = static_cast<T_OUT>(x_i);
  }
}

// Returns normalization(X), which is the same as doing X * Y
template <unsigned int N, typename T_IN0, typename T_IN1, typename T_OUT>
void vector_normalize(const T_IN0 x[N], const T_IN1 y[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN0>::value, "T_IN0 type check failed");
  static_assert(is_ap_fixed_type<T_IN1>::value, "T_IN1 type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  LOOP_NORM: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const T_IN0 x_i = x[i];
    const T_IN1 y_i = y[i];
    out[i] = x_i * y_i;
  }
}

// Returns dot(X, Y) + Z
// X has rows = N, Y has rows = N, Z is a scalar
// It follows the convention as used in a NN, not the convention in basic linear algebra.
template <unsigned int N, typename T_IN0, typename T_IN1, typename T_IN2, typename T_OUT>
void vector_vector_mult_biasadd(const T_IN0 x[N], const T_IN1 y[N], const T_IN2& z, T_OUT& out) {
  static_assert(is_ap_fixed_type<T_IN0>::value, "T_IN0 type check failed");
  static_assert(is_ap_fixed_type<T_IN1>::value, "T_IN1 type check failed");
  static_assert(is_ap_fixed_type<T_IN2>::value, "T_IN2 type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

//#pragma HLS EXPRESSION_BALANCE off

  constexpr int W_MULT = (T_IN0::width + T_IN1::width);
  constexpr int I_MULT = (T_IN0::iwidth + T_IN1::iwidth);
  constexpr int W_ACCUM = W_MULT + 4;  // additional bits to prevent overflow (for 15<=x<31 accumulation terms)
  constexpr int I_ACCUM = I_MULT + 4;
  constexpr int W_OUT = T_OUT::width;
  constexpr int I_OUT = T_OUT::iwidth;

  ap_fixed<W_MULT,I_MULT> mult[N];

#pragma HLS ARRAY_PARTITION variable=mult complete

  // Multiply
  LOOP_MULT: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const T_IN0 x_i = x[i];
    const T_IN1 y_i = y[i];
    const auto m = x_i * y_i;

#pragma HLS RESOURCE variable=m core=DSP48

    mult[i] = m;
  }

  ap_fixed<W_ACCUM,I_ACCUM> accum = z;  // init with the bias term

  // Accumulate
  LOOP_ACCUM: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    accum += mult[i];
  }

  // Round and saturate
  out = ap_fixed<W_OUT,I_OUT,AP_RND,AP_SAT>(accum);

  // Sanity check
#ifndef __SYNTHESIS__
  {
    float f_accum = static_cast<float>(z);
    for (unsigned i = 0; i < N; i++) {
      f_accum += static_cast<float>(x[i]) * static_cast<float>(y[i]);
    }
    emtf_assert(std::abs(f_accum) < static_cast<float>(1 << (I_ACCUM-1)));  // make sure no overflow
  }
#endif

}

// Returns matmul(X, Y) + Z
// X has dim (rows, cols) = (M, N), Y has rows = M, Z has cols = N
// It follows the convention as used in a NN, not the convention in basic linear algebra.
template <unsigned int M, unsigned int N, typename T_IN0, typename T_IN1, typename T_IN2, typename T_OUT>
void matrix_vector_mult_biasadd(const T_IN0 x[M * N], const T_IN1 y[M], const T_IN2 z[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN0>::value, "T_IN0 type check failed");
  static_assert(is_ap_fixed_type<T_IN1>::value, "T_IN1 type check failed");
  static_assert(is_ap_fixed_type<T_IN2>::value, "T_IN2 type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

//#pragma HLS EXPRESSION_BALANCE off

  constexpr int W_MULT = (T_IN0::width + T_IN1::width);
  constexpr int I_MULT = (T_IN0::iwidth + T_IN1::iwidth);
  constexpr int W_ACCUM = W_MULT + 4;  // additional bits to prevent overflow (for 15<=x<31 accumulation terms)
  constexpr int I_ACCUM = I_MULT + 4;
  constexpr int W_OUT = T_OUT::width;
  constexpr int I_OUT = T_OUT::iwidth;

  ap_fixed<W_MULT,I_MULT> mult[M * N];

#pragma HLS ARRAY_PARTITION variable=mult complete

  // Multiply
  LOOP_MULT_I: for (unsigned i = 0; i < M; i++) {

#pragma HLS UNROLL

    const T_IN1 y_i = y[i];

    LOOP_MULT_J: for (unsigned j = 0; j < N; j++) {

#pragma HLS UNROLL

      const T_IN0 x_ij = x[(i * N) + j];
      const auto m = x_ij * y_i;

#pragma HLS RESOURCE variable=m core=DSP48

      mult[(i * N) + j] = m;
    }
  }

  // Accumulate
  LOOP_ACCUM_J: for (unsigned j = 0; j < N; j++) {

#pragma HLS UNROLL

    ap_fixed<W_ACCUM,I_ACCUM> accum = z[j];  // init with the bias term

    LOOP_ACCUM_I: for (unsigned i = 0; i < M; i++) {

#pragma HLS UNROLL

      accum += mult[(i * N) + j];
    }

    // Round and saturate
    out[j] = ap_fixed<W_OUT,I_OUT,AP_RND,AP_SAT>(accum);
  }

  // Sanity check
#ifndef __SYNTHESIS__
  for (unsigned j = 0; j < N; j++) {
    float f_accum = static_cast<float>(z[j]);
    for (unsigned i = 0; i < M; i++) {
      f_accum += static_cast<float>(x[(i * N) + j]) * static_cast<float>(y[i]);
    }
    emtf_assert(std::abs(f_accum) < static_cast<float>(1 << (I_ACCUM-1)));  // make sure no overflow
  }
#endif

}

}  // namespace details

}  // namespace emtf

#endif  // __EMTF_HLSLIB_NNET_KERNELS_H__ not defined
