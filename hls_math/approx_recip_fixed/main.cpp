#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>  // provides std::floor, std::ldexp, std::abs

// EMTF HLS
#include "common.h"
#include "traits.h"

namespace emtf_hlslib {

namespace phase2 {

// Adapted from:
// https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/kernels/cwise_ops.h
template <typename T = void>
float round_half_to_even(float x) {
  const float round_val = std::floor(x + 0.5f);
  const float fraction = round_val - x;
  if (emtf_unlikely(fraction == 0.5f)) {
    return 2.0f * std::floor(0.5f * x + 0.5f);
  } else {
    return round_val;
  }
}

template <unsigned int N, typename T_IN, typename T_OUT>
void init_recip_table_op(T_OUT table[N]) {
  static_assert(is_ap_fixed_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");
  static_assert(N == (1u << (T_IN::width - 2)), "N value check failed");

  // Use only (1u << (T_IN::width - 2)) addresses, as there are 2 implicit bits:
  // - sign bit is always 0 after taking the absolute value
  // - msb bit is always 1 after taking the normalized floating-point value

  ap_fixed<T_IN::width, 1> x = 0;  // use only 1 integer bit

  for (unsigned i = 0; i < N; i++) {
    // Cast from unsigned to ap_uint
    ap_uint<T_IN::width> w = i;
    // Reinterpret ap_uint as ap_fixed
    x.range() = w.range();
    // Add sign bit and msb bit
    x[T_IN::width - 1] = 0;  // sign bit is always 0
    x[T_IN::width - 2] = 1;  // msb bit is always 1
    // Cast from ap_fixed T_IN to float32, compute 1/x, cast to ap_fixed T_OUT
    float x_f32 = static_cast<float>(x);
    emtf_assert(x_f32 != 0.0f);  // avoid division by zero
    float y_f32 = 1.0f / x_f32;
    float q_f32 = std::ldexp(1.0f, -1 * ap_fixed_widths<T_OUT>::fwidth);
    table[i] = static_cast<T_OUT>(round_half_to_even(y_f32 / q_f32) * q_f32);
  }
}

// Returns 1/X
template <unsigned int N, typename T_IN, typename T_OUT>
void vector_recip_op(const T_IN x[N], T_OUT out[N]) {
  static_assert(is_ap_fixed_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_fixed_type<T_OUT>::value, "T_OUT type check failed");
  static_assert(T_IN::width >= 2, "T_IN type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = fullyconnect_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  constexpr int W_IN = T_IN::width;

  typedef ap_uint<W_IN> index_t;            // address before removing the 2 implicit bits
  typedef ap_uint<W_IN - 2> index_trunc_t;  // address after removing the 2 implicit bits

  typedef ap_ufixed<12, 1> recip_table_t;  // result is stored in 12 bits, unsigned (hardcoded)
  typedef ap_fixed<16, 3> recip_t;         // result is then expanded to 16 bits, signed (hardcoded)

  typedef ap_fixed<recip_t::width + (W_IN - 2), recip_t::iwidth + (W_IN - 2)> shifted_recip_t;  // after left shift

  const unsigned int N_TABLE = (1u << index_trunc_t::width);

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static recip_table_t recip_table[N_TABLE];
#else
  bool initialized = false;
  recip_table_t recip_table[N_TABLE];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    init_recip_table_op<N_TABLE, T_IN>(recip_table);
  }

LOOP_RECIP:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Reinterpret ap_fixed as ap_uint, taking absolute value
    const T_IN x_i = x[i];
    const bool is_zero = x_i.is_zero();
    const bool is_neg = x_i.is_neg();
    const index_t index_tmp_0_0 = x_i.range();
    const index_t index_tmp_0_1 = (-x_i).range();
    const index_t index_tmp_0 = is_neg ? index_tmp_0_1 : index_tmp_0_0;

    // Take normalized floating-point value
    // To be reinterpreted as ap_fixed<W_IN, 1>
    const int lead_zeros = const_cast<index_t&>(index_tmp_0).countLeadingZeros();
    int lead_zeros_sub_1 = 0;
    index_t index_tmp_1 = 0;

    if (lead_zeros == 0) {  // lowest (negative) value
      lead_zeros_sub_1 = 0;
      index_tmp_1 = (static_cast<index_t>(-1) >> 1);
    } else if (lead_zeros == W_IN) {  // zero
      lead_zeros_sub_1 = W_IN - 2;
      index_tmp_1 = (static_cast<index_t>(1) << lead_zeros_sub_1);
    } else {
      lead_zeros_sub_1 = lead_zeros - 1;
      index_tmp_1 = (index_tmp_0 << lead_zeros_sub_1);
    }

    // Reinterpret ap_uint as ap_fixed
    ap_fixed<W_IN, 1> index_tmp_1_cast = 0;
    index_tmp_1_cast.range() = index_tmp_1.range();

    // Sanity check
#ifndef __SYNTHESIS__
    {
      emtf_assert((0 <= lead_zeros) and (lead_zeros <= W_IN));
      emtf_assert((0 <= lead_zeros_sub_1) and (lead_zeros_sub_1 <= (W_IN - 2)));
      emtf_assert(index_tmp_1[W_IN - 1] == 0);           // sign bit is always 0
      emtf_assert(index_tmp_1[W_IN - 2] == 1);           // msb bit is always 1
      emtf_assert(0.5f <= index_tmp_1_cast.to_float());  // normalized range is [0.5, 1)
      emtf_assert(index_tmp_1_cast.to_float() < 1.0f);
    }
#endif  // __SYNTHESIS__ not defined

    // Lookup
    const index_trunc_t index = index_tmp_1;  // drop sign bit and msb bit
    emtf_assert(index < N_TABLE);
    const recip_t recip_tmp_0_0 = static_cast<recip_t>(recip_table[index]);
    const recip_t recip_tmp_0_1 = static_cast<recip_t>(2.0f);  // special case when index == 0 and recip = 1/0.5 = 2
    const recip_t recip_tmp_0 = (index == 0) ? recip_tmp_0_1 : recip_tmp_0_0;

    // Newton-Raphson division
    // See: https://en.wikipedia.org/wiki/Division_algorithm#Newton.E2.80.93Raphson_division
    // Using the formula: x_{i+1} = x_{i} + x_{i} * (1 - D * x_{i})
    // Each iteration requires two fused multiply-adds
    const bool perform_nr_step = false;  // didn't work
    const recip_t recip_tmp_1_0 = recip_tmp_0;
    const recip_t recip_tmp_1_1 = static_cast<recip_t>(1.0f) - (index_tmp_1_cast * recip_tmp_1_0);
    const recip_t recip_tmp_1_2 = recip_tmp_1_0 + (recip_tmp_1_0 * recip_tmp_1_1);
    const recip_t recip_tmp_1 = perform_nr_step ? recip_tmp_1_2 : recip_tmp_1_0;

    // Left-shift and saturate
    const shifted_recip_t recip_tmp_2 = (static_cast<shifted_recip_t>(recip_tmp_1) << lead_zeros_sub_1);
    emtf_assert(not recip_tmp_2.is_neg());

    constexpr int deleted_bits_lo = shifted_recip_t::width - shifted_recip_t::iwidth + T_OUT::iwidth - 1;
    constexpr int deleted_bits_hi = shifted_recip_t::width - 1;
    const bool overflow = (recip_tmp_2.range(deleted_bits_hi, deleted_bits_lo) != 0);
    const T_OUT max_allowed = ap_fixed_limits<T_OUT>::max_value;  // based on output range
    const T_OUT recip_tmp_3_0 = overflow ? max_allowed : static_cast<T_OUT>(recip_tmp_2);
    const T_OUT recip_tmp_3_1 = overflow ? -max_allowed : -recip_tmp_3_0;
    const T_OUT recip_tmp_3 = is_neg ? recip_tmp_3_1 : recip_tmp_3_0;  // recover sign

    // Output
    out[i] = recip_tmp_3;

    // Sanity check
#ifndef __SYNTHESIS__
    {
      emtf_assert((0 <= T_OUT::iwidth) and (T_OUT::iwidth < shifted_recip_t::iwidth));
      emtf_assert((0 <= deleted_bits_lo) and (deleted_bits_lo < deleted_bits_hi));
      const recip_t recip_tmp_1_chk =
          ((static_cast<shifted_recip_t>(recip_tmp_1) << lead_zeros_sub_1) >> lead_zeros_sub_1);
      emtf_assert(recip_tmp_1 == recip_tmp_1_chk);
      const bool overflow_chk = (recip_tmp_2 > max_allowed);
      emtf_assert(overflow == overflow_chk);
      const float max_allowed_based_on_input = (1.0f / ap_fixed_limits<T_IN>::epsilon_value);  // based on input epsilon
      emtf_assert(0.0f < recip_tmp_2.to_float());  // result range is (0, 1/eps]
      emtf_assert(recip_tmp_2.to_float() <= max_allowed_based_on_input);

      if (not is_zero) {
        float x_f32 = std::abs(static_cast<float>(x_i));
        float y_f32 = 1.0f / x_f32;
        float rel_error = std::abs(recip_tmp_2.to_float() - y_f32) / y_f32;
        float rel_error_tol = (1.5f * std::ldexp(1.0f, -12));  // 3.6621e-4
        emtf_assert(rel_error < rel_error_tol);
      }

      if ((not is_zero) and (not overflow)) {
        float x_f32 = static_cast<float>(x_i);
        float y_f32 = 1.0f / x_f32;
        float abs_error = std::abs(recip_tmp_3.to_float() - y_f32);
        float abs_error_tol = ap_fixed_limits<T_OUT>::epsilon_value * 2;  // not sure why need a factor of 2
        emtf_assert(abs_error < abs_error_tol);
      }
    }
#endif  // __SYNTHESIS__ not defined

  }  // end loop
}

}  // namespace phase2

}  // namespace emtf_hlslib

using namespace emtf_hlslib::phase2;

int main() {
  typedef ap_fixed<14, 1> T_IN;    // range: [-1, 0.999878] step_size: 1.220703e-04
  typedef ap_fixed<15, 11> T_OUT;  // range: [-1024, 1023.9375] step_size: 0.0625

  constexpr int N = (1 << T_IN::width);

  T_IN in0[N];
  T_OUT out[N];

  for (unsigned i = 0; i < N; ++i) {
    in0[i].range() = i;
  }

  vector_recip_op<N>(in0, out);

  for (unsigned i = 0; i < N; ++i) {
    float y_f32 = (in0[i] == 0 ? 0.0f : (1.0f / in0[i].to_float()));
    std::cout << in0[i] << " " << out[i] << " " << y_f32 << std::endl;
  }

  // Done
  return 0;
}
