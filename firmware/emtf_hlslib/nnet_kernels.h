#ifndef __EMTF_HLSLIB_NNET_KERNELS_H__
#define __EMTF_HLSLIB_NNET_KERNELS_H__

namespace emtf {

namespace details {

// Piecewise-linear tanh-like activation function
// This version is designed for ap_fixed<14,1> output
template <int W, int I>
dio_hard_tanh_t hard_tanh(const ap_fixed<W,I>& in0) {

#pragma HLS PIPELINE II=nnet_config::target_ii

//#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  constexpr int W_OUT = dio_hard_tanh_t::width;
  constexpr int I_OUT = dio_hard_tanh_t::iwidth;
  constexpr int F = W - I;
  constexpr int F_OUT = W_OUT - I_OUT;

  // Use unsigned value
  const ap_uint<1> sign0 = in0[W-1];
  const ap_ufixed<W,I> inabs = (sign0 == 0) ? ap_ufixed<W,I>(in0) : ap_ufixed<W,I>(-in0);

  // Threshold terms
  const ap_ufixed<1,0> b0 = 0.5;
  const ap_ufixed<1,1> b1 = 1.0;
  const ap_ufixed<2,1> b2 = 1.5;
  const ap_ufixed<2,2> b3 = 2.0;
  const ap_ufixed<3,2> b4 = 2.5;

  // Subtract-and-divide terms
  // The number of bits needs to be extended before right shift
  const ap_ufixed<W,I-1> c0 = (inabs >= b0) ? ap_ufixed<W,I-1>(ap_ufixed<W+1,I>(inabs - b0) >> 1) : ap_ufixed<W,I-1>(0);
  const ap_ufixed<W,I-2> c1 = (inabs >= b1) ? ap_ufixed<W,I-2>(ap_ufixed<W+2,I>(inabs - b1) >> 2) : ap_ufixed<W,I-2>(0);
  const ap_ufixed<W,I-3> c2 = (inabs >= b2) ? ap_ufixed<W,I-3>(ap_ufixed<W+3,I>(inabs - b2) >> 3) : ap_ufixed<W,I-3>(0);
  const ap_ufixed<W,I-4> c3 = (inabs >= b3) ? ap_ufixed<W,I-4>(ap_ufixed<W+4,I>(inabs - b3) >> 4) : ap_ufixed<W,I-4>(0);
  const ap_ufixed<W,I-5> c4 = (inabs >= b4) ? ap_ufixed<W,I-5>(ap_ufixed<W+5,I>(inabs - b4) >> 5) : ap_ufixed<W,I-5>(0);

  // Compute
  ap_fixed<W+6,I+1> out_tmp = inabs;  // signed
  out_tmp -= c0;
  out_tmp -= c1;
  out_tmp -= c2;
  out_tmp -= c3;
  out_tmp -= c4;
  emtf_assert(out_tmp >= 0);
  emtf_assert(out_tmp < (1<<(I-1)));

  // Round
  emtf_assert((F+5) >= (F_OUT+1));
  ap_ufixed<1,-F_OUT> half_val = 0;
  half_val[0] = 1;  // 0.5 * LSB
  out_tmp += half_val;

  // Saturate
  ap_fixed<W_OUT,I_OUT,AP_TRN,AP_SAT> out_tmp_sat = out_tmp;

  // Remember the sign
  ap_fixed<W_OUT,I_OUT> out = (sign0 == 0) ? ap_fixed<W_OUT,I_OUT>(out_tmp_sat) : ap_fixed<W_OUT,I_OUT>(-out_tmp_sat);
  emtf_assert(in0 == 0 or (in0 != 0 && out != 0));
  return out;
}

// Returns activation(X)
template <unsigned int N, typename T_IN, typename T_OUT>
void vector_activate(const T_IN x[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  emtf_assert((is_same<T_OUT, dio_hard_tanh_t>::value));

  LOOP_ACT: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const T_IN x_i = x[i];
    out[i] = hard_tanh(x_i);
  }
}

// Returns normalization(X), which is the same as doing X * Y
template <unsigned int N, typename T_IN0, typename T_IN1, typename T_OUT>
void vector_normalize(const T_IN0 x[N], const T_IN1 y[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN0>::value, "T_IN0 type check failed");
  static_assert(is_ap_int_type<T_IN1>::value, "T_IN1 type check failed");  // ap_int instead of ap_fixed
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  LOOP_MULT: for (unsigned i = 0; i < N; i++) {

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
  constexpr int W_ACCUM = W_MULT + 4;  // additional bits to prevent overflow (with 15<=x<31 accumulation terms)
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

  ap_fixed<W_ACCUM,I_ACCUM> accum = z;  // init with bias

  // Accumulate
  LOOP_ACC: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    accum += mult[i];
  }

  // Round and saturate
  out = ap_fixed<W_OUT,I_OUT,AP_RND,AP_SAT>(accum);

  // Sanity check
#ifndef __SYNTHESIS__
  {
    double f_accum = static_cast<double>(z);
    for (unsigned i = 0; i < N; i++) {
      f_accum += static_cast<double>(x[i]) * static_cast<double>(y[i]);
    }
    emtf_assert(std::abs(f_accum) < static_cast<double>(1<<(I_ACCUM-1)));  // make sure no overflow
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
  constexpr int W_ACCUM = W_MULT + 4;  // additional bits to prevent overflow (with 15<=x<31 accumulation terms)
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
  LOOP_ACC_J: for (unsigned j = 0; j < N; j++) {

#pragma HLS UNROLL

    ap_fixed<W_ACCUM,I_ACCUM> accum = z[j];  // init with bias

    LOOP_ACC_I: for (unsigned i = 0; i < M; i++) {

#pragma HLS UNROLL

      accum += mult[(i * N) + j];
    }

    // Round and saturate
    out[j] = ap_fixed<W_OUT,I_OUT,AP_RND,AP_SAT>(accum);
  }

  // Sanity check
#ifndef __SYNTHESIS__
  for (unsigned j = 0; j < N; j++) {
    double f_accum = static_cast<double>(z[j]);
    for (unsigned i = 0; i < M; i++) {
      f_accum += static_cast<double>(x[(i * N) + j]) * static_cast<double>(y[i]);
    }
    emtf_assert(std::abs(f_accum) < static_cast<double>(1<<(I_ACCUM-1)));  // make sure no overflow
  }
#endif

}

}  // namespace details

}  // namespace emtf

#endif  // __EMTF_HLSLIB_NNET_KERNELS_H__ not defined
