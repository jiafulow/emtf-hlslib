#ifndef __EMTF_HLSLIB_POOLING_H__
#define __EMTF_HLSLIB_POOLING_H__

// Function hierarchy
//
// pooling_layer
// +-- pooling_op (INLINE)
//     +-- pooling_fused_col_op
//         +-- pooling_col_op (INLINE)
//             |-- pooling_col_pool_op (INLINE)
//             +-- pooling_col_argmax_op

// EMTF HLS
#include "layer_helpers.h"

namespace emtf {

namespace phase2 {

namespace detail {

template <typename Zone, typename T_IN, typename T_OUT>
void apply_pattern_activation_op(const T_IN& in0, T_OUT& out) {
  static_assert(is_ap_int_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_int_type<T_OUT>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = pooling_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int N_TABLE = (1u << T_IN::width);

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static int lookup_table[N_TABLE];
#else
  bool initialized = false;
  int lookup_table[N_TABLE];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    detail::init_table_op<N_TABLE>(lookup_table, detail::get_pattern_activation_op<Zone>{});
  }

  // Lookup
  emtf_assert(in0 < N_TABLE);
  out = static_cast<T_OUT>(lookup_table[in0]);
}

}  // namespace detail

// _____________________________________________________________________________
template <typename Zone>
void pooling_col_pool_op(const typename detail::select_pattern_col_patch_type<Zone, 0>::type& patch_row_0,
                         const typename detail::select_pattern_col_patch_type<Zone, 1>::type& patch_row_1,
                         const typename detail::select_pattern_col_patch_type<Zone, 2>::type& patch_row_2,
                         const typename detail::select_pattern_col_patch_type<Zone, 3>::type& patch_row_3,
                         const typename detail::select_pattern_col_patch_type<Zone, 4>::type& patch_row_4,
                         const typename detail::select_pattern_col_patch_type<Zone, 5>::type& patch_row_5,
                         const typename detail::select_pattern_col_patch_type<Zone, 6>::type& patch_row_6,
                         const typename detail::select_pattern_col_patch_type<Zone, 7>::type& patch_row_7,
                         trk_qual_t activations[num_emtf_patterns]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = pooling_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static int pattern_col_start_table[num_emtf_patterns * num_emtf_img_rows];
  static int pattern_col_stop_table[num_emtf_patterns * num_emtf_img_rows];
#else
  bool initialized = false;
  int pattern_col_start_table[num_emtf_patterns * num_emtf_img_rows];
  int pattern_col_stop_table[num_emtf_patterns * num_emtf_img_rows];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    detail::init_2d_table_op<num_emtf_patterns, num_emtf_img_rows>(pattern_col_start_table,
                                                                   detail::get_pattern_col_start_op<Zone>{});
    detail::init_2d_table_op<num_emtf_patterns, num_emtf_img_rows>(pattern_col_stop_table,
                                                                   detail::get_pattern_col_stop_op<Zone>{});
  }

  // Loop over patterns
LOOP_POOL:
  for (unsigned i = 0; i < num_emtf_patterns; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned int table_index = (i * num_emtf_img_rows);

    const unsigned int col_start_0 = pattern_col_start_table[table_index + 0];
    const unsigned int col_start_1 = pattern_col_start_table[table_index + 1];
    const unsigned int col_start_2 = pattern_col_start_table[table_index + 2];
    const unsigned int col_start_3 = pattern_col_start_table[table_index + 3];
    const unsigned int col_start_4 = pattern_col_start_table[table_index + 4];
    const unsigned int col_start_5 = pattern_col_start_table[table_index + 5];
    const unsigned int col_start_6 = pattern_col_start_table[table_index + 6];
    const unsigned int col_start_7 = pattern_col_start_table[table_index + 7];

    const unsigned int col_stop_0 = pattern_col_stop_table[table_index + 0];
    const unsigned int col_stop_1 = pattern_col_stop_table[table_index + 1];
    const unsigned int col_stop_2 = pattern_col_stop_table[table_index + 2];
    const unsigned int col_stop_3 = pattern_col_stop_table[table_index + 3];
    const unsigned int col_stop_4 = pattern_col_stop_table[table_index + 4];
    const unsigned int col_stop_5 = pattern_col_stop_table[table_index + 5];
    const unsigned int col_stop_6 = pattern_col_stop_table[table_index + 6];
    const unsigned int col_stop_7 = pattern_col_stop_table[table_index + 7];

    // Pooling, done by logical OR reduction
    // Note: static_cast<bool> is different from static_cast<bool_t>. static_cast<bool>
    // performs logical OR reduction, whereas static_cast<bool_t> performs bit selection.
    const bool_t b0 = static_cast<bool>(patch_row_0.range(col_stop_0, col_start_0));
    const bool_t b1 = static_cast<bool>(patch_row_1.range(col_stop_1, col_start_1));
    const bool_t b2 = static_cast<bool>(patch_row_2.range(col_stop_2, col_start_2));
    const bool_t b3 = static_cast<bool>(patch_row_3.range(col_stop_3, col_start_3));
    const bool_t b4 = static_cast<bool>(patch_row_4.range(col_stop_4, col_start_4));
    const bool_t b5 = static_cast<bool>(patch_row_5.range(col_stop_5, col_start_5));
    const bool_t b6 = static_cast<bool>(patch_row_6.range(col_stop_6, col_start_6));
    const bool_t b7 = static_cast<bool>(patch_row_7.range(col_stop_7, col_start_7));

    // Preactivation, done by bit concatenation
    const dio_patt_preact_t preactivation = (b7, b6, b5, b4, b3, b2, b1, b0);

    // Activation (a.k.a. trk_qual), done by a LUT
    detail::apply_pattern_activation_op<Zone>(preactivation, activations[i]);
  }  // end loop over patterns
}

template <typename T_IN, typename T_OUT>
void pooling_col_argmax_op(const T_IN in0[num_emtf_patterns], T_OUT& out) {
  static_assert(is_same<T_IN, trk_qual_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, pooling_out_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = pooling_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  typedef T_IN data_t;
  typedef trk_patt_t arg_t;
  typedef typename make_concat<arg_t, data_t>::type pair_t;

  constexpr int bits_lo = 0;
  constexpr int bits_hi = (data_t::width - 1);

  const pair_t tmp_0_0 = (static_cast<arg_t>(0), in0[0]);
  // const pair_t tmp_0_1 = (static_cast<arg_t>(0), in0[0]);  // unused
  const pair_t tmp_0_2 = (static_cast<arg_t>(1), in0[1]);
  const pair_t tmp_0_3 = (static_cast<arg_t>(2), in0[2]);
  const pair_t tmp_0_4 = (static_cast<arg_t>(3), in0[3]);
  const pair_t tmp_0_5 = (static_cast<arg_t>(4), in0[4]);
  const pair_t tmp_0_6 = (static_cast<arg_t>(5), in0[5]);
  const pair_t tmp_0_7 = (static_cast<arg_t>(6), in0[6]);

  // See max_of_seven_op() to understand how this works.
  const pair_t tmp_1_2 = (tmp_0_2.range(bits_hi, bits_lo) < tmp_0_3.range(bits_hi, bits_lo)) ? tmp_0_3 : tmp_0_2;
  const pair_t tmp_1_4 = (tmp_0_4.range(bits_hi, bits_lo) < tmp_0_5.range(bits_hi, bits_lo)) ? tmp_0_5 : tmp_0_4;
  const pair_t tmp_1_6 = (tmp_0_6.range(bits_hi, bits_lo) < tmp_0_7.range(bits_hi, bits_lo)) ? tmp_0_7 : tmp_0_6;
  const pair_t tmp_2_0 = (tmp_0_0.range(bits_hi, bits_lo) < tmp_1_2.range(bits_hi, bits_lo)) ? tmp_1_2 : tmp_0_0;
  const pair_t tmp_2_4 = (tmp_1_4.range(bits_hi, bits_lo) < tmp_1_6.range(bits_hi, bits_lo)) ? tmp_1_6 : tmp_1_4;
  const pair_t tmp_3_0 = (tmp_2_0.range(bits_hi, bits_lo) < tmp_2_4.range(bits_hi, bits_lo)) ? tmp_2_4 : tmp_2_0;

  // Output
  out = tmp_3_0;
}

// _____________________________________________________________________________
// Perform pooling with a set of patterns, apply pattern activation, and output
// the pattern with max activation.

template <typename Zone>
void pooling_col_op(const typename detail::select_pattern_col_patch_type<Zone, 0>::type& patch_row_0,
                    const typename detail::select_pattern_col_patch_type<Zone, 1>::type& patch_row_1,
                    const typename detail::select_pattern_col_patch_type<Zone, 2>::type& patch_row_2,
                    const typename detail::select_pattern_col_patch_type<Zone, 3>::type& patch_row_3,
                    const typename detail::select_pattern_col_patch_type<Zone, 4>::type& patch_row_4,
                    const typename detail::select_pattern_col_patch_type<Zone, 5>::type& patch_row_5,
                    const typename detail::select_pattern_col_patch_type<Zone, 6>::type& patch_row_6,
                    const typename detail::select_pattern_col_patch_type<Zone, 7>::type& patch_row_7,
                    pooling_out_t& pooling_out_col_k) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = pooling_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Intermediate arrays
  trk_qual_t activations[num_emtf_patterns];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = activations complete dim = 0
  // hls-pragmas end

  pooling_col_pool_op<Zone>(patch_row_0, patch_row_1, patch_row_2, patch_row_3, patch_row_4, patch_row_5, patch_row_6,
                            patch_row_7, activations);

  pooling_col_argmax_op(activations, pooling_out_col_k);
}

template <typename Zone>
void pooling_fused_col_op(const typename detail::select_pattern_fused_col_patch_type<Zone, 0>::type& fused_patch_row_0,
                          const typename detail::select_pattern_fused_col_patch_type<Zone, 1>::type& fused_patch_row_1,
                          const typename detail::select_pattern_fused_col_patch_type<Zone, 2>::type& fused_patch_row_2,
                          const typename detail::select_pattern_fused_col_patch_type<Zone, 3>::type& fused_patch_row_3,
                          const typename detail::select_pattern_fused_col_patch_type<Zone, 4>::type& fused_patch_row_4,
                          const typename detail::select_pattern_fused_col_patch_type<Zone, 5>::type& fused_patch_row_5,
                          const typename detail::select_pattern_fused_col_patch_type<Zone, 6>::type& fused_patch_row_6,
                          const typename detail::select_pattern_fused_col_patch_type<Zone, 7>::type& fused_patch_row_7,
                          pooling_out_t pooling_out_reg[pooling_config::fusion_factor]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = pooling_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  typedef typename detail::select_pattern_col_padding_type<Zone, 0>::type padding_row_0_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 1>::type padding_row_1_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 2>::type padding_row_2_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 3>::type padding_row_3_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 4>::type padding_row_4_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 5>::type padding_row_5_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 6>::type padding_row_6_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 7>::type padding_row_7_t;

  const unsigned int fusion_factor = pooling_config::fusion_factor;

  // Loop over columns that are fused
LOOP_COL_3:
  for (unsigned j = 0; j < fusion_factor; j++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Apply pooling to each column
    pooling_col_op<Zone>(fused_patch_row_0.range(j + (padding_row_0_t::width * 2), j),
                         fused_patch_row_1.range(j + (padding_row_1_t::width * 2), j),
                         fused_patch_row_2.range(j + (padding_row_2_t::width * 2), j),
                         fused_patch_row_3.range(j + (padding_row_3_t::width * 2), j),
                         fused_patch_row_4.range(j + (padding_row_4_t::width * 2), j),
                         fused_patch_row_5.range(j + (padding_row_5_t::width * 2), j),
                         fused_patch_row_6.range(j + (padding_row_6_t::width * 2), j),
                         fused_patch_row_7.range(j + (padding_row_7_t::width * 2), j), pooling_out_reg[j]);
  }  // end loop over columns that are fused
}

// _____________________________________________________________________________
// Pooling op

template <typename Zone>
void pooling_op(const pooling_in_t pooling_in[pooling_config::n_in], pooling_out_t pooling_out[pooling_config::n_out]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = pooling_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  typedef typename detail::select_pattern_col_padding_type<Zone, 0>::type padding_row_0_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 1>::type padding_row_1_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 2>::type padding_row_2_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 3>::type padding_row_3_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 4>::type padding_row_4_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 5>::type padding_row_5_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 6>::type padding_row_6_t;
  typedef typename detail::select_pattern_col_padding_type<Zone, 7>::type padding_row_7_t;

  typedef typename detail::select_pattern_col_padded_type<Zone, 0>::type padded_row_0_t;
  typedef typename detail::select_pattern_col_padded_type<Zone, 1>::type padded_row_1_t;
  typedef typename detail::select_pattern_col_padded_type<Zone, 2>::type padded_row_2_t;
  typedef typename detail::select_pattern_col_padded_type<Zone, 3>::type padded_row_3_t;
  typedef typename detail::select_pattern_col_padded_type<Zone, 4>::type padded_row_4_t;
  typedef typename detail::select_pattern_col_padded_type<Zone, 5>::type padded_row_5_t;
  typedef typename detail::select_pattern_col_padded_type<Zone, 6>::type padded_row_6_t;
  typedef typename detail::select_pattern_col_padded_type<Zone, 7>::type padded_row_7_t;

  // Add padding (two-sided)
  const padded_row_0_t padded_row_0 = (padding_row_0_t(0), pooling_in[0], padding_row_0_t(0));
  const padded_row_1_t padded_row_1 = (padding_row_1_t(0), pooling_in[1], padding_row_1_t(0));
  const padded_row_2_t padded_row_2 = (padding_row_2_t(0), pooling_in[2], padding_row_2_t(0));
  const padded_row_3_t padded_row_3 = (padding_row_3_t(0), pooling_in[3], padding_row_3_t(0));
  const padded_row_4_t padded_row_4 = (padding_row_4_t(0), pooling_in[4], padding_row_4_t(0));
  const padded_row_5_t padded_row_5 = (padding_row_5_t(0), pooling_in[5], padding_row_5_t(0));
  const padded_row_6_t padded_row_6 = (padding_row_6_t(0), pooling_in[6], padding_row_6_t(0));
  const padded_row_7_t padded_row_7 = (padding_row_7_t(0), pooling_in[7], padding_row_7_t(0));

  const unsigned int fusion_factor = pooling_config::fusion_factor;

  unsigned col = 0;

  // Loop over columns with step size = fusion_factor
LOOP_COL_1:
  for (unsigned i = 0; i < num_emtf_img_cols; i += fusion_factor) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Intermediate arrays
    pooling_out_t pooling_out_reg[fusion_factor];

    // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = pooling_out_reg complete dim = 0
    // hls-pragmas end

    // Apply pooling to several columns
    pooling_fused_col_op<Zone>(padded_row_0.range(i + (padding_row_0_t::width * 2) + (fusion_factor - 1), i),
                               padded_row_1.range(i + (padding_row_1_t::width * 2) + (fusion_factor - 1), i),
                               padded_row_2.range(i + (padding_row_2_t::width * 2) + (fusion_factor - 1), i),
                               padded_row_3.range(i + (padding_row_3_t::width * 2) + (fusion_factor - 1), i),
                               padded_row_4.range(i + (padding_row_4_t::width * 2) + (fusion_factor - 1), i),
                               padded_row_5.range(i + (padding_row_5_t::width * 2) + (fusion_factor - 1), i),
                               padded_row_6.range(i + (padding_row_6_t::width * 2) + (fusion_factor - 1), i),
                               padded_row_7.range(i + (padding_row_7_t::width * 2) + (fusion_factor - 1), i),
                               pooling_out_reg);

    // Loop over columns that are fused
  LOOP_COL_2:
    for (unsigned j = 0; j < fusion_factor; j++) {
      // hls-pragmas begin
#pragma HLS UNROLL
      // hls-pragmas end

      pooling_out[col] = pooling_out_reg[j];
      col++;
    }  // end loop over columns that are fused
  }    // end loop over columns
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void pooling_layer(const pooling_in_t pooling_in[pooling_config::n_in],
                   pooling_out_t pooling_out[pooling_config::n_out]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = pooling_config::layer_target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  // hls-pragmas end

  // Check assumptions
  static_assert(pooling_config::n_in == num_emtf_img_rows, "pooling_config::n_in check failed");
  static_assert(pooling_config::n_out == num_emtf_img_cols, "pooling_config::n_out check failed");
  static_assert(pooling_config::fusion_factor >= 1, "pooling_config::fusion_factor check failed");
  static_assert(num_emtf_img_rows == 8, "num_emtf_img_rows must be 8");
  static_assert(num_emtf_img_cols == 288, "num_emtf_img_cols must be 288");
  static_assert(num_emtf_patterns == 7, "num_emtf_patterns must be 7");
  static_assert(dio_patt_preact_t::width == pooling_config::n_in, "dio_patt_preact_t type check failed");

  pooling_op<Zone>(pooling_in, pooling_out);
}

}  // namespace phase2

}  // namespace emtf

#endif  // __EMTF_HLSLIB_POOLING_H__ not defined
