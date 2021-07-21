#ifndef __EMTF_HLSLIB_ZONING_H__
#define __EMTF_HLSLIB_ZONING_H__

// Function hierarchy
//
// zoning_layer
// +-- zoning_op (INLINE)
//     +-- zoning_row_op (INLINE)
//         |-- zoning_row_gather_op (INLINE)
//         |   +-- zoning_row_fill_op
//         |-- zoning_row_gather_mux_op (INLINE)
//         +-- zoning_row_join_op (INLINE)

// EMTF HLS
#include "layer_helpers.h"

namespace emtf_hlslib {

namespace phase2 {

struct zoning_internal_config : zoning_config {
  // This enumerates the 8 ph_init + 8 ph_cover values (in ascending order) for the
  // 8 possible 10-deg chambers in a sector.
  constexpr static const int col_start_0 = detail::chamber_img_joined_col_start;
  constexpr static const int col_start_1 = detail::chamber_ph_init_10deg[6];
  constexpr static const int col_start_3 = detail::chamber_ph_init_10deg[0];
  constexpr static const int col_start_5 = detail::chamber_ph_init_10deg[1];
  constexpr static const int col_start_7 = detail::chamber_ph_init_10deg[2];
  constexpr static const int col_start_9 = detail::chamber_ph_init_10deg[3];
  constexpr static const int col_start_b = detail::chamber_ph_init_10deg[4];
  constexpr static const int col_start_d = detail::chamber_ph_init_10deg[5];

  constexpr static const int col_start_2 = detail::chamber_ph_cover_10deg[0] - detail::chamber_ph_init_10deg[0];
  constexpr static const int col_start_4 = detail::chamber_ph_cover_10deg[6];
  constexpr static const int col_start_6 = detail::chamber_ph_cover_10deg[0];
  constexpr static const int col_start_8 = detail::chamber_ph_cover_10deg[1];
  constexpr static const int col_start_a = detail::chamber_ph_cover_10deg[2];
  constexpr static const int col_start_c = detail::chamber_ph_cover_10deg[3];
  constexpr static const int col_start_e = detail::chamber_ph_cover_10deg[4];
  constexpr static const int col_start_f = detail::chamber_ph_cover_10deg[5];

  // This enumerates the bit widths of the 15 slices defined by the above 16 edges.
  // The total bit width must be equal to num_emtf_img_cols.
  constexpr static const int bw_slice_0 = col_start_1 - col_start_0;
  constexpr static const int bw_slice_1 = col_start_2 - col_start_1;
  constexpr static const int bw_slice_2 = col_start_3 - col_start_2;
  constexpr static const int bw_slice_3 = col_start_4 - col_start_3;
  constexpr static const int bw_slice_4 = col_start_5 - col_start_4;
  constexpr static const int bw_slice_5 = col_start_6 - col_start_5;
  constexpr static const int bw_slice_6 = col_start_7 - col_start_6;
  constexpr static const int bw_slice_7 = col_start_8 - col_start_7;
  constexpr static const int bw_slice_8 = col_start_9 - col_start_8;
  constexpr static const int bw_slice_9 = col_start_a - col_start_9;
  constexpr static const int bw_slice_a = col_start_b - col_start_a;
  constexpr static const int bw_slice_b = col_start_c - col_start_b;
  constexpr static const int bw_slice_c = col_start_d - col_start_c;
  constexpr static const int bw_slice_d = col_start_e - col_start_d;
  constexpr static const int bw_slice_e = col_start_f - col_start_e;

  // Typedefs
  typedef ap_uint<detail::chamber_img_bw> chamber_img_t;
  typedef ap_uint<detail::chamber_img_joined_bw> chamber_img_joined_t;
};

// _____________________________________________________________________________
template <unsigned int N, typename T_IN, typename T_OUT>
void zoning_row_join_op(const T_IN in0[N], T_OUT& out, m_10deg_chamber_tag) {
  static_assert(is_same<T_IN, zoning_internal_config::chamber_img_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zoning_out_t>::value, "T_OUT type check failed");
  static_assert(N == detail::num_chambers_traits<m_10deg_chamber_tag>::value, "N value check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Slices
  using cfg = zoning_internal_config;
  ap_uint<cfg::bw_slice_0> tmp_0_0 = 0;
  ap_uint<cfg::bw_slice_1> tmp_1_0 = 0;
  ap_uint<cfg::bw_slice_1> tmp_1_1 =
      in0[6].range(cfg::col_start_2 - cfg::col_start_1 - 1, cfg::col_start_1 - cfg::col_start_1);
  ap_uint<cfg::bw_slice_2> tmp_2_0 =
      in0[6].range(cfg::col_start_3 - cfg::col_start_1 - 1, cfg::col_start_2 - cfg::col_start_1);
  ap_uint<cfg::bw_slice_3> tmp_3_0 =
      in0[6].range(cfg::col_start_4 - cfg::col_start_1 - 1, cfg::col_start_3 - cfg::col_start_1);
  ap_uint<cfg::bw_slice_3> tmp_3_1 =
      in0[0].range(cfg::col_start_4 - cfg::col_start_3 - 1, cfg::col_start_3 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_4> tmp_4_0 =
      in0[0].range(cfg::col_start_5 - cfg::col_start_3 - 1, cfg::col_start_4 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_5> tmp_5_0 =
      in0[0].range(cfg::col_start_6 - cfg::col_start_3 - 1, cfg::col_start_5 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_5> tmp_5_1 =
      in0[1].range(cfg::col_start_6 - cfg::col_start_5 - 1, cfg::col_start_5 - cfg::col_start_5);
  ap_uint<cfg::bw_slice_6> tmp_6_0 =
      in0[1].range(cfg::col_start_7 - cfg::col_start_5 - 1, cfg::col_start_6 - cfg::col_start_5);
  ap_uint<cfg::bw_slice_7> tmp_7_0 =
      in0[1].range(cfg::col_start_8 - cfg::col_start_5 - 1, cfg::col_start_7 - cfg::col_start_5);
  ap_uint<cfg::bw_slice_7> tmp_7_1 =
      in0[2].range(cfg::col_start_8 - cfg::col_start_7 - 1, cfg::col_start_7 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_8> tmp_8_0 =
      in0[2].range(cfg::col_start_9 - cfg::col_start_7 - 1, cfg::col_start_8 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_9> tmp_9_0 =
      in0[2].range(cfg::col_start_a - cfg::col_start_7 - 1, cfg::col_start_9 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_9> tmp_9_1 =
      in0[3].range(cfg::col_start_a - cfg::col_start_9 - 1, cfg::col_start_9 - cfg::col_start_9);
  ap_uint<cfg::bw_slice_a> tmp_a_0 =
      in0[3].range(cfg::col_start_b - cfg::col_start_9 - 1, cfg::col_start_a - cfg::col_start_9);
  ap_uint<cfg::bw_slice_b> tmp_b_0 =
      in0[3].range(cfg::col_start_c - cfg::col_start_9 - 1, cfg::col_start_b - cfg::col_start_9);
  ap_uint<cfg::bw_slice_b> tmp_b_1 =
      in0[4].range(cfg::col_start_c - cfg::col_start_b - 1, cfg::col_start_b - cfg::col_start_b);
  ap_uint<cfg::bw_slice_c> tmp_c_0 =
      in0[4].range(cfg::col_start_d - cfg::col_start_b - 1, cfg::col_start_c - cfg::col_start_b);
  ap_uint<cfg::bw_slice_d> tmp_d_0 =
      in0[4].range(cfg::col_start_e - cfg::col_start_b - 1, cfg::col_start_d - cfg::col_start_b);
  ap_uint<cfg::bw_slice_d> tmp_d_1 =
      in0[5].range(cfg::col_start_e - cfg::col_start_d - 1, cfg::col_start_d - cfg::col_start_d);
  ap_uint<cfg::bw_slice_e> tmp_e_0 =
      in0[5].range(cfg::col_start_f - cfg::col_start_d - 1, cfg::col_start_e - cfg::col_start_d);

  // Logical OR
  ap_uint<cfg::bw_slice_0> tmp_1_0_0 = tmp_0_0;
  ap_uint<cfg::bw_slice_1> tmp_1_1_0 = (tmp_1_0 | tmp_1_1);
  ap_uint<cfg::bw_slice_2> tmp_1_2_0 = tmp_2_0;
  ap_uint<cfg::bw_slice_3> tmp_1_3_0 = (tmp_3_0 | tmp_3_1);
  ap_uint<cfg::bw_slice_4> tmp_1_4_0 = tmp_4_0;
  ap_uint<cfg::bw_slice_5> tmp_1_5_0 = (tmp_5_0 | tmp_5_1);
  ap_uint<cfg::bw_slice_6> tmp_1_6_0 = tmp_6_0;
  ap_uint<cfg::bw_slice_7> tmp_1_7_0 = (tmp_7_0 | tmp_7_1);
  ap_uint<cfg::bw_slice_8> tmp_1_8_0 = tmp_8_0;
  ap_uint<cfg::bw_slice_9> tmp_1_9_0 = (tmp_9_0 | tmp_9_1);
  ap_uint<cfg::bw_slice_a> tmp_1_a_0 = tmp_a_0;
  ap_uint<cfg::bw_slice_b> tmp_1_b_0 = (tmp_b_0 | tmp_b_1);
  ap_uint<cfg::bw_slice_c> tmp_1_c_0 = tmp_c_0;
  ap_uint<cfg::bw_slice_d> tmp_1_d_0 = (tmp_d_0 | tmp_d_1);
  ap_uint<cfg::bw_slice_e> tmp_1_e_0 = tmp_e_0;

  // Bit concatenation
  const auto tmp_2_0_0 = (tmp_1_e_0, tmp_1_d_0, tmp_1_c_0, tmp_1_b_0, tmp_1_a_0, tmp_1_9_0, tmp_1_8_0, tmp_1_7_0,
                          tmp_1_6_0, tmp_1_5_0, tmp_1_4_0, tmp_1_3_0, tmp_1_2_0, tmp_1_1_0, tmp_1_0_0);
  emtf_assert(tmp_2_0_0.length() == zoning_out_t::width);

  // Output
  out = tmp_2_0_0;
}

template <unsigned int N, typename T_IN, typename T_OUT>
void zoning_row_join_op(const T_IN in0[N], T_OUT& out, m_20deg_chamber_tag) {
  static_assert(is_same<T_IN, zoning_internal_config::chamber_img_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zoning_out_t>::value, "T_OUT type check failed");
  static_assert(N == detail::num_chambers_traits<m_20deg_chamber_tag>::value, "N value check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Slices
  using cfg = zoning_internal_config;
  ap_uint<cfg::bw_slice_0> tmp_0_0 = in0[3].range(cfg::col_start_1 - 1, cfg::col_start_0);
  ap_uint<cfg::bw_slice_1> tmp_1_0 = in0[3].range(cfg::col_start_2 - 1, cfg::col_start_1);
  ap_uint<cfg::bw_slice_2> tmp_2_0 = in0[3].range(cfg::col_start_3 - 1, cfg::col_start_2);
  ap_uint<cfg::bw_slice_3> tmp_3_0 = in0[3].range(cfg::col_start_4 - 1, cfg::col_start_3);
  ap_uint<cfg::bw_slice_3> tmp_3_1 =
      in0[0].range(cfg::col_start_4 - cfg::col_start_3 - 1, cfg::col_start_3 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_4> tmp_4_0 =
      in0[0].range(cfg::col_start_5 - cfg::col_start_3 - 1, cfg::col_start_4 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_5> tmp_5_0 =
      in0[0].range(cfg::col_start_6 - cfg::col_start_3 - 1, cfg::col_start_5 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_6> tmp_6_0 =
      in0[0].range(cfg::col_start_7 - cfg::col_start_3 - 1, cfg::col_start_6 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_7> tmp_7_0 =
      in0[0].range(cfg::col_start_8 - cfg::col_start_3 - 1, cfg::col_start_7 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_7> tmp_7_1 =
      in0[1].range(cfg::col_start_8 - cfg::col_start_7 - 1, cfg::col_start_7 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_8> tmp_8_0 =
      in0[1].range(cfg::col_start_9 - cfg::col_start_7 - 1, cfg::col_start_8 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_9> tmp_9_0 =
      in0[1].range(cfg::col_start_a - cfg::col_start_7 - 1, cfg::col_start_9 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_a> tmp_a_0 =
      in0[1].range(cfg::col_start_b - cfg::col_start_7 - 1, cfg::col_start_a - cfg::col_start_7);
  ap_uint<cfg::bw_slice_b> tmp_b_0 =
      in0[1].range(cfg::col_start_c - cfg::col_start_7 - 1, cfg::col_start_b - cfg::col_start_7);
  ap_uint<cfg::bw_slice_b> tmp_b_1 =
      in0[2].range(cfg::col_start_c - cfg::col_start_b - 1, cfg::col_start_b - cfg::col_start_b);
  ap_uint<cfg::bw_slice_c> tmp_c_0 =
      in0[2].range(cfg::col_start_d - cfg::col_start_b - 1, cfg::col_start_c - cfg::col_start_b);
  ap_uint<cfg::bw_slice_d> tmp_d_0 =
      in0[2].range(cfg::col_start_e - cfg::col_start_b - 1, cfg::col_start_d - cfg::col_start_b);
  ap_uint<cfg::bw_slice_e> tmp_e_0 =
      in0[2].range(cfg::col_start_f - cfg::col_start_b - 1, cfg::col_start_e - cfg::col_start_b);

  // Logical OR
  ap_uint<cfg::bw_slice_0> tmp_1_0_0 = tmp_0_0;
  ap_uint<cfg::bw_slice_1> tmp_1_1_0 = tmp_1_0;
  ap_uint<cfg::bw_slice_2> tmp_1_2_0 = tmp_2_0;
  ap_uint<cfg::bw_slice_3> tmp_1_3_0 = (tmp_3_0 | tmp_3_1);
  ap_uint<cfg::bw_slice_4> tmp_1_4_0 = tmp_4_0;
  ap_uint<cfg::bw_slice_5> tmp_1_5_0 = tmp_5_0;
  ap_uint<cfg::bw_slice_6> tmp_1_6_0 = tmp_6_0;
  ap_uint<cfg::bw_slice_7> tmp_1_7_0 = (tmp_7_0 | tmp_7_1);
  ap_uint<cfg::bw_slice_8> tmp_1_8_0 = tmp_8_0;
  ap_uint<cfg::bw_slice_9> tmp_1_9_0 = tmp_9_0;
  ap_uint<cfg::bw_slice_a> tmp_1_a_0 = tmp_a_0;
  ap_uint<cfg::bw_slice_b> tmp_1_b_0 = (tmp_b_0 | tmp_b_1);
  ap_uint<cfg::bw_slice_c> tmp_1_c_0 = tmp_c_0;
  ap_uint<cfg::bw_slice_d> tmp_1_d_0 = tmp_d_0;
  ap_uint<cfg::bw_slice_e> tmp_1_e_0 = tmp_e_0;

  // Bit concatenation
  const auto tmp_2_0_0 = (tmp_1_e_0, tmp_1_d_0, tmp_1_c_0, tmp_1_b_0, tmp_1_a_0, tmp_1_9_0, tmp_1_8_0, tmp_1_7_0,
                          tmp_1_6_0, tmp_1_5_0, tmp_1_4_0, tmp_1_3_0, tmp_1_2_0, tmp_1_1_0, tmp_1_0_0);
  emtf_assert(tmp_2_0_0.length() == zoning_out_t::width);

  // Output
  out = tmp_2_0_0;
}

template <unsigned int N, typename T_IN, typename T_OUT>
void zoning_row_join_op(const T_IN in0[N], T_OUT& out, m_20deg_ext_chamber_tag) {
  static_assert(is_same<T_IN, zoning_internal_config::chamber_img_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zoning_out_t>::value, "T_OUT type check failed");
  static_assert(N == detail::num_chambers_traits<m_20deg_ext_chamber_tag>::value, "N value check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Slices
  using cfg = zoning_internal_config;
  ap_uint<cfg::bw_slice_0> tmp_0_0 = in0[3].range(cfg::col_start_1 - 1, cfg::col_start_0);
  ap_uint<cfg::bw_slice_1> tmp_1_0 = in0[3].range(cfg::col_start_2 - 1, cfg::col_start_1);
  ap_uint<cfg::bw_slice_2> tmp_2_0 = in0[3].range(cfg::col_start_3 - 1, cfg::col_start_2);
  ap_uint<cfg::bw_slice_3> tmp_3_0 = in0[3].range(cfg::col_start_4 - 1, cfg::col_start_3);
  ap_uint<cfg::bw_slice_3> tmp_3_1 =
      in0[0].range(cfg::col_start_4 - cfg::col_start_3 - 1, cfg::col_start_3 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_4> tmp_4_0 =
      in0[0].range(cfg::col_start_5 - cfg::col_start_3 - 1, cfg::col_start_4 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_5> tmp_5_0 =
      in0[0].range(cfg::col_start_6 - cfg::col_start_3 - 1, cfg::col_start_5 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_6> tmp_6_0 =
      in0[0].range(cfg::col_start_7 - cfg::col_start_3 - 1, cfg::col_start_6 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_7> tmp_7_0 =
      in0[0].range(cfg::col_start_8 - cfg::col_start_3 - 1, cfg::col_start_7 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_7> tmp_7_1 =
      in0[1].range(cfg::col_start_8 - cfg::col_start_7 - 1, cfg::col_start_7 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_8> tmp_8_0 =
      in0[1].range(cfg::col_start_9 - cfg::col_start_7 - 1, cfg::col_start_8 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_9> tmp_9_0 =
      in0[1].range(cfg::col_start_a - cfg::col_start_7 - 1, cfg::col_start_9 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_a> tmp_a_0 =
      in0[1].range(cfg::col_start_b - cfg::col_start_7 - 1, cfg::col_start_a - cfg::col_start_7);
  ap_uint<cfg::bw_slice_b> tmp_b_0 =
      in0[1].range(cfg::col_start_c - cfg::col_start_7 - 1, cfg::col_start_b - cfg::col_start_7);
  ap_uint<cfg::bw_slice_b> tmp_b_1 =
      in0[2].range(cfg::col_start_c - cfg::col_start_b - 1, cfg::col_start_b - cfg::col_start_b);
  ap_uint<cfg::bw_slice_c> tmp_c_0 =
      in0[2].range(cfg::col_start_d - cfg::col_start_b - 1, cfg::col_start_c - cfg::col_start_b);
  ap_uint<cfg::bw_slice_d> tmp_d_0 =
      in0[2].range(cfg::col_start_e - cfg::col_start_b - 1, cfg::col_start_d - cfg::col_start_b);
  ap_uint<cfg::bw_slice_e> tmp_e_0 =
      in0[2].range(cfg::col_start_f - cfg::col_start_b - 1, cfg::col_start_e - cfg::col_start_b);

  ap_uint<cfg::bw_slice_1> tmp_1_1 =
      in0[4 + 6].range(cfg::col_start_2 - cfg::col_start_1 - 1, cfg::col_start_1 - cfg::col_start_1);
  ap_uint<cfg::bw_slice_2> tmp_2_1 =
      in0[4 + 6].range(cfg::col_start_3 - cfg::col_start_1 - 1, cfg::col_start_2 - cfg::col_start_1);
  ap_uint<cfg::bw_slice_3> tmp_3_2 =
      in0[4 + 6].range(cfg::col_start_4 - cfg::col_start_1 - 1, cfg::col_start_3 - cfg::col_start_1);
  ap_uint<cfg::bw_slice_3> tmp_3_3 =
      in0[4 + 0].range(cfg::col_start_4 - cfg::col_start_3 - 1, cfg::col_start_3 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_4> tmp_4_1 =
      in0[4 + 0].range(cfg::col_start_5 - cfg::col_start_3 - 1, cfg::col_start_4 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_5> tmp_5_1 =
      in0[4 + 0].range(cfg::col_start_6 - cfg::col_start_3 - 1, cfg::col_start_5 - cfg::col_start_3);
  ap_uint<cfg::bw_slice_5> tmp_5_2 =
      in0[4 + 1].range(cfg::col_start_6 - cfg::col_start_5 - 1, cfg::col_start_5 - cfg::col_start_5);
  ap_uint<cfg::bw_slice_6> tmp_6_1 =
      in0[4 + 1].range(cfg::col_start_7 - cfg::col_start_5 - 1, cfg::col_start_6 - cfg::col_start_5);
  ap_uint<cfg::bw_slice_7> tmp_7_2 =
      in0[4 + 1].range(cfg::col_start_8 - cfg::col_start_5 - 1, cfg::col_start_7 - cfg::col_start_5);
  ap_uint<cfg::bw_slice_7> tmp_7_3 =
      in0[4 + 2].range(cfg::col_start_8 - cfg::col_start_7 - 1, cfg::col_start_7 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_8> tmp_8_1 =
      in0[4 + 2].range(cfg::col_start_9 - cfg::col_start_7 - 1, cfg::col_start_8 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_9> tmp_9_1 =
      in0[4 + 2].range(cfg::col_start_a - cfg::col_start_7 - 1, cfg::col_start_9 - cfg::col_start_7);
  ap_uint<cfg::bw_slice_9> tmp_9_2 =
      in0[4 + 3].range(cfg::col_start_a - cfg::col_start_9 - 1, cfg::col_start_9 - cfg::col_start_9);
  ap_uint<cfg::bw_slice_a> tmp_a_1 =
      in0[4 + 3].range(cfg::col_start_b - cfg::col_start_9 - 1, cfg::col_start_a - cfg::col_start_9);
  ap_uint<cfg::bw_slice_b> tmp_b_2 =
      in0[4 + 3].range(cfg::col_start_c - cfg::col_start_9 - 1, cfg::col_start_b - cfg::col_start_9);
  ap_uint<cfg::bw_slice_b> tmp_b_3 =
      in0[4 + 4].range(cfg::col_start_c - cfg::col_start_b - 1, cfg::col_start_b - cfg::col_start_b);
  ap_uint<cfg::bw_slice_c> tmp_c_1 =
      in0[4 + 4].range(cfg::col_start_d - cfg::col_start_b - 1, cfg::col_start_c - cfg::col_start_b);
  ap_uint<cfg::bw_slice_d> tmp_d_1 =
      in0[4 + 4].range(cfg::col_start_e - cfg::col_start_b - 1, cfg::col_start_d - cfg::col_start_b);
  ap_uint<cfg::bw_slice_d> tmp_d_2 =
      in0[4 + 5].range(cfg::col_start_e - cfg::col_start_d - 1, cfg::col_start_d - cfg::col_start_d);
  ap_uint<cfg::bw_slice_e> tmp_e_1 =
      in0[4 + 5].range(cfg::col_start_f - cfg::col_start_d - 1, cfg::col_start_e - cfg::col_start_d);

  // Logical OR
  ap_uint<cfg::bw_slice_0> tmp_1_0_0 = tmp_0_0;
  ap_uint<cfg::bw_slice_1> tmp_1_1_0 = (tmp_1_0 | tmp_1_1);
  ap_uint<cfg::bw_slice_2> tmp_1_2_0 = (tmp_2_0 | tmp_2_1);
  ap_uint<cfg::bw_slice_3> tmp_1_3_0 = ((tmp_3_0 | tmp_3_1) | (tmp_3_2 | tmp_3_3));
  ap_uint<cfg::bw_slice_4> tmp_1_4_0 = (tmp_4_0 | tmp_4_1);
  ap_uint<cfg::bw_slice_5> tmp_1_5_0 = (tmp_5_0 | (tmp_5_1 | tmp_5_2));
  ap_uint<cfg::bw_slice_6> tmp_1_6_0 = (tmp_6_0 | tmp_6_1);
  ap_uint<cfg::bw_slice_7> tmp_1_7_0 = ((tmp_7_0 | tmp_7_1) | (tmp_7_2 | tmp_7_3));
  ap_uint<cfg::bw_slice_8> tmp_1_8_0 = (tmp_8_0 | tmp_8_1);
  ap_uint<cfg::bw_slice_9> tmp_1_9_0 = (tmp_9_0 | (tmp_9_1 | tmp_9_2));
  ap_uint<cfg::bw_slice_a> tmp_1_a_0 = (tmp_a_0 | tmp_a_1);
  ap_uint<cfg::bw_slice_b> tmp_1_b_0 = ((tmp_b_0 | tmp_b_1) | (tmp_b_2 | tmp_b_3));
  ap_uint<cfg::bw_slice_c> tmp_1_c_0 = (tmp_c_0 | tmp_c_1);
  ap_uint<cfg::bw_slice_d> tmp_1_d_0 = (tmp_d_0 | (tmp_d_1 | tmp_d_2));
  ap_uint<cfg::bw_slice_e> tmp_1_e_0 = (tmp_e_0 | tmp_e_1);

  // Bit concatenation
  const auto tmp_2_0_0 = (tmp_1_e_0, tmp_1_d_0, tmp_1_c_0, tmp_1_b_0, tmp_1_a_0, tmp_1_9_0, tmp_1_8_0, tmp_1_7_0,
                          tmp_1_6_0, tmp_1_5_0, tmp_1_4_0, tmp_1_3_0, tmp_1_2_0, tmp_1_1_0, tmp_1_0_0);
  emtf_assert(tmp_2_0_0.length() == zoning_out_t::width);

  // Output
  out = tmp_2_0_0;
}

// _____________________________________________________________________________
template <typename T_IN, typename T_OUT>
void zoning_row_fill_op(const T_IN in0[num_emtf_segments], const bool_t in1[num_emtf_segments], T_OUT& out) {
  static_assert(is_same<T_IN, trk_col_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zoning_internal_config::chamber_img_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  T_OUT chamber_img = 0;  // init as zero

  // Loop over segments
LOOP_SEG_2:
  for (unsigned j = 0; j < num_emtf_segments; j++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const T_IN col = in0[j];
    const bool valid = in1[j];

    // Fill the chamber image: set bit to 1 at a particular col if segment is found.
    // Note: it might need expression balancing if num_emtf_segments > 2.
    if (valid) {
      chamber_img[col] = 1;  // set bit to 1
    }
  }  // end loop over segments

  // Output
  out = chamber_img;
}

// _____________________________________________________________________________
template <typename Zone, typename Timezone, typename Row, unsigned int N, typename T_OUT>
void zoning_row_gather_op(const emtf_phi_t emtf_phi[model_config::n_in],
                          const seg_zones_t seg_zones[model_config::n_in],
                          const seg_tzones_t seg_tzones[model_config::n_in],
                          const seg_valid_t seg_valid[model_config::n_in], T_OUT chamber_images[N]) {
  static_assert(is_same<T_OUT, zoning_internal_config::chamber_img_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  typedef typename detail::chamber_category_traits<Row>::chamber_category chamber_category;
  static_assert(N == detail::num_chambers_traits<chamber_category>::value, "N value check failed");

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static int chamber_id_table[N];
  static int chamber_ph_init_table[N];
  static int chamber_ph_cover_table[N];
#else
  bool initialized = false;
  int chamber_id_table[N];
  int chamber_ph_init_table[N];
  int chamber_ph_cover_table[N];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    detail::init_table_op<N>(chamber_id_table, detail::get_chamber_id_op<Row>{});
    detail::init_table_op<N>(chamber_ph_init_table, detail::get_chamber_ph_init_op<chamber_category>{});
    detail::init_table_op<N>(chamber_ph_cover_table, detail::get_chamber_ph_cover_op<chamber_category>{});
  }

  // Translate zone, timezone into bit selection
  const trk_zone_t the_zone = detail::zone_traits<Zone>::value;
  const trk_tzone_t the_tzone = detail::timezone_traits<Timezone>::value;
  const trk_zone_t bit_sel_zone = static_cast<trk_zone_t>(num_emtf_zones - 1) - the_zone;
  const trk_tzone_t bit_sel_tzone = static_cast<trk_tzone_t>(num_emtf_timezones - 1) - the_tzone;

  // Loop over chambers
LOOP_CHM_1:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Intermediate arrays
    trk_col_t columns[num_emtf_segments];
    bool_t columns_v[num_emtf_segments];

    // hls-pragmas begin
#pragma HLS ARRAY_RESHAPE variable = columns complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = columns_v complete dim = 0
    // hls-pragmas end

    // std::cout << "[DEBUG] zone " << detail::zone_traits<Zone>::value
    //           << " chamber: " << chamber_id_table[i]
    //           << " ph_init: " << chamber_ph_init_table[i]
    //           << " ph_cover: " << chamber_ph_cover_table[i] << std::endl;

    // Loop over segments
  LOOP_SEG_1:
    for (unsigned j = 0; j < num_emtf_segments; j++) {
      // hls-pragmas begin
#pragma HLS UNROLL
      // hls-pragmas end

      const unsigned iseg = static_cast<unsigned>(chamber_id_table[i] * num_emtf_segments) + j;
      const trk_col_t ph_init = chamber_ph_init_table[i];
      emtf_assert(chamber_id_table[i] < num_emtf_chambers);
      emtf_assert(iseg < (num_emtf_chambers * num_emtf_segments));

      // Translate emtf_phi to col: truncate the last 4 bits, then subtract ph_init
      constexpr int bits_to_shift = emtf_img_col_factor_log2;
      const emtf_phi_t ph0 = emtf_phi[iseg];
      const trk_col_t col = static_cast<trk_col_t>(ph0 >> bits_to_shift) - ph_init;  // unsafe math

      // Condition: is_valid_seg && is_same_zone && is_same_timezone
      const bool valid =
          ((seg_valid[iseg] == 1) and (seg_zones[iseg][bit_sel_zone] == 1) and (seg_tzones[iseg][bit_sel_tzone] == 1));

      if (valid) {
        emtf_assert((ph0 >> bits_to_shift) >= ph_init);
        emtf_assert(col < detail::chamber_img_bw);
        // std::cout << "[DEBUG] .. segment: " << j << " emtf_phi: " << emtf_phi[iseg]
        //           << " col: " << col << std::endl;
      }

      columns[j] = col;
      columns_v[j] = valid;
    }  // end loop over segments

    // Fill the chamber image
    zoning_row_fill_op(columns, columns_v, chamber_images[i]);
  }  // end loop over chambers
}

template <unsigned int N, typename T_IN, typename T_OUT>
void zoning_row_gather_mux_op(const T_IN in0[N], const T_IN in1[N], T_OUT out[N]) {
  static_assert(is_same<T_IN, zoning_internal_config::chamber_img_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zoning_internal_config::chamber_img_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Loop over chambers
LOOP_CHM_2:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Logical OR
    out[i] = (in0[i] | in1[i]);
  }  // end loop over chambers
}

// _____________________________________________________________________________
// Perform loop over chambers and all the segments in the chambers.
// Fill the chamber image for each chamber, then join all the chamber images.

template <typename Zone, typename Timezone, typename Row, typename SecondRow = Row>
void zoning_row_op(const emtf_phi_t emtf_phi[model_config::n_in], const seg_zones_t seg_zones[model_config::n_in],
                   const seg_tzones_t seg_tzones[model_config::n_in], const seg_valid_t seg_valid[model_config::n_in],
                   zoning_out_t& zoning_out_row_k) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  typedef typename detail::chamber_category_traits<Row>::chamber_category chamber_category;
  const unsigned int N = detail::num_chambers_traits<chamber_category>::value;

  typedef zoning_internal_config::chamber_img_t chamber_img_t;

  // Intermediate arrays
  chamber_img_t chamber_images_tmp_0[N];
  chamber_img_t chamber_images_tmp_1[N];
  chamber_img_t chamber_images[N];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = chamber_images_tmp_0 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = chamber_images_tmp_1 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = chamber_images complete dim = 0
  // hls-pragmas end

  // Fill the chamber images
  if (!is_same<Row, SecondRow>::value) {  // enable if Row and SecondRow are different
    zoning_row_gather_op<Zone, Timezone, Row, N>(emtf_phi, seg_zones, seg_tzones, seg_valid, chamber_images_tmp_0);
    zoning_row_gather_op<Zone, Timezone, SecondRow, N>(emtf_phi, seg_zones, seg_tzones, seg_valid,
                                                       chamber_images_tmp_1);
    zoning_row_gather_mux_op<N>(chamber_images_tmp_0, chamber_images_tmp_1, chamber_images);

  } else {  // enable if Row and Second are identical
    zoning_row_gather_op<Zone, Timezone, Row, N>(emtf_phi, seg_zones, seg_tzones, seg_valid, chamber_images);
  }

  // Join the chamber images
  zoning_row_join_op<N>(chamber_images, zoning_out_row_k, chamber_category{});
}

// _____________________________________________________________________________
// Zoning op

template <typename Zone, typename Timezone>
void zoning_op(const emtf_phi_t emtf_phi[model_config::n_in], const seg_zones_t seg_zones[model_config::n_in],
               const seg_tzones_t seg_tzones[model_config::n_in], const seg_valid_t seg_valid[model_config::n_in],
               zoning_out_t zoning_out[zoning_config::n_out]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Loop over the rows manually
  if (is_same<Zone, m_zone_0_tag>::value) {  // enable if Zone 0
    zoning_row_op<Zone, Timezone, m_zone_0_row_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[0]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[1]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_2_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[2]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_3_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[3]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_4_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[4]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_5_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[5]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_6_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[6]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_7_0_tag, m_zone_0_row_7_1_tag>(emtf_phi, seg_zones, seg_tzones,
                                                                              seg_valid, zoning_out[7]);

  } else if (is_same<Zone, m_zone_1_tag>::value) {  // enable if Zone 1
    zoning_row_op<Zone, Timezone, m_zone_1_row_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[0]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[1]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_2_0_tag, m_zone_1_row_2_1_tag>(emtf_phi, seg_zones, seg_tzones,
                                                                              seg_valid, zoning_out[2]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_3_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[3]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_4_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[4]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_5_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[5]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_6_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[6]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_7_0_tag, m_zone_1_row_7_1_tag>(emtf_phi, seg_zones, seg_tzones,
                                                                              seg_valid, zoning_out[7]);

  } else if (is_same<Zone, m_zone_2_tag>::value) {  // enable if Zone 2
    zoning_row_op<Zone, Timezone, m_zone_2_row_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[0]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[1]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_2_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[2]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_3_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[3]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_4_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[4]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_5_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[5]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_6_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[6]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_7_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[7]);
  }
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void zoning_layer(const emtf_phi_t emtf_phi[model_config::n_in], const seg_zones_t seg_zones[model_config::n_in],
                  const seg_tzones_t seg_tzones[model_config::n_in], const seg_valid_t seg_valid[model_config::n_in],
                  zoning_out_t zoning_0_out[zoning_config::n_out], zoning_out_t zoning_1_out[zoning_config::n_out],
                  zoning_out_t zoning_2_out[zoning_config::n_out]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = zoning_config::layer_target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  // hls-pragmas end

  // Check assumptions
  static_assert(zoning_config::n_out == num_emtf_img_rows, "zoning_config::n_out check failed");
  static_assert(num_emtf_img_rows == 8, "num_emtf_img_rows must be 8");
  static_assert(num_emtf_img_cols == 288, "num_emtf_img_cols must be 288");

  typedef m_timezone_0_tag Timezone;  // default timezone

  // Loop over the zones manually
  zoning_op<m_zone_0_tag, Timezone>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_0_out);
  zoning_op<m_zone_1_tag, Timezone>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_1_out);
  zoning_op<m_zone_2_tag, Timezone>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_2_out);
}

}  // namespace phase2

}  // namespace emtf_hlslib

#endif  // __EMTF_HLSLIB_ZONING_H__ not defined
