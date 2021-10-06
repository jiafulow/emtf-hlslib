#ifndef __EMTF_HLSLIB_TRKBUILDING_H__
#define __EMTF_HLSLIB_TRKBUILDING_H__

// Function hierarchy
//
// trkbuilding_layer
// +-- trkbuilding_op (INLINE)
//     |-- trkbuilding_find_ph_median_op
//     |-- trkbuilding_match_ph_op
//     |   +-- trkbuilding_match_ph_site_op (INLINE)
//     |       |-- trkbuilding_match_ph_compute_op (INLINE)
//     |       |-- trkbuilding_match_ph_pack_op (INLINE)
//     |       +-- trkbuilding_match_ph_argmin_op
//     |-- trkbuilding_find_th_median_op
//     |   +-- trkbuilding_find_th_median_of_nine_op (INLINE)
//     |-- trkbuilding_match_th_op
//     |   +-- trkbuilding_match_th_select_op (INLINE)
//     +-- trkbuilding_extract_features_op (INLINE)

// EMTF HLS
#include "layer_helpers.h"
#include "copy_kernels.h"
#include "sort_kernels.h"

namespace emtf_hlslib {

namespace phase2 {

namespace detail {

template <typename Site, typename T_IN, typename T_OUT>
void find_pattern_windows_op(const T_IN& in0, T_OUT& par0, T_OUT& par1, T_OUT& par2, T_OUT& par3) {
  static_assert(is_ap_int_type<T_IN>::value, "T_IN type check failed");
  static_assert(is_ap_int_type<T_OUT>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int M_TABLE = (1u << trk_zone_t::width);
  const unsigned int N_TABLE = (1u << trk_patt_t::width);
  static_assert(T_IN::width == (trk_zone_t::width + trk_patt_t::width), "T_IN_type check failed");

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static int pattern_col_start_table[M_TABLE * N_TABLE];
  static int pattern_col_mid_table[M_TABLE * N_TABLE];
  static int pattern_col_stop_table[M_TABLE * N_TABLE];
  static int pattern_col_pad_table[M_TABLE * N_TABLE];
#else
  bool initialized = false;
  int pattern_col_start_table[M_TABLE * N_TABLE];
  int pattern_col_mid_table[M_TABLE * N_TABLE];
  int pattern_col_stop_table[M_TABLE * N_TABLE];
  int pattern_col_pad_table[M_TABLE * N_TABLE];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    detail::init_2d_table_op<M_TABLE, N_TABLE>(pattern_col_start_table, detail::get_site_pattern_col_start_op<Site>());
    detail::init_2d_table_op<M_TABLE, N_TABLE>(pattern_col_mid_table, detail::get_site_pattern_col_mid_op<Site>());
    detail::init_2d_table_op<M_TABLE, N_TABLE>(pattern_col_stop_table, detail::get_site_pattern_col_stop_op<Site>());
    detail::init_2d_table_op<M_TABLE, N_TABLE>(pattern_col_pad_table, detail::get_site_pattern_col_pad_op<Site>());
  }

  // Intermediate arrays
  typedef struct {
    dio_patt_param_t par0;
    dio_patt_param_t par1;
    dio_patt_param_t par2;
    dio_patt_param_t par3;
  } quad_param_t;

  quad_param_t quad_param_table[M_TABLE * N_TABLE];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = quad_param_table complete dim = 0
#pragma HLS DATA_PACK variable = quad_param_table
  // hls-pragmas end

LOOP_PARAM:
  for (unsigned i = 0; i < (M_TABLE * N_TABLE); i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const quad_param_t x_i = {static_cast<dio_patt_param_t>(pattern_col_start_table[i]),
                              static_cast<dio_patt_param_t>(pattern_col_mid_table[i]),
                              static_cast<dio_patt_param_t>(pattern_col_stop_table[i]),
                              static_cast<dio_patt_param_t>(pattern_col_pad_table[i])};
    quad_param_table[i] = x_i;
  }

  // Lookup
  emtf_assert(in0 < (M_TABLE * N_TABLE));
  const quad_param_t& x_i = quad_param_table[in0];
  par0 = x_i.par0;
  par1 = x_i.par1;
  par2 = x_i.par2;
  par3 = x_i.par3;
}

// Helper function to calculate abs difference
template <typename T>
T calc_abs_diff(const T& lhs, const T& rhs) {
  static_assert(!ap_int_props<T>::is_signed_v, "T must be unsigned");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  return (lhs >= rhs) ? static_cast<T>(lhs - rhs) : static_cast<T>(rhs - lhs);
}

// Helper function to calculate signed difference
template <typename T, typename U = typename make_signed<typename make_wider<T>::type>::type>
U calc_signed_diff(const T& lhs, const T& rhs) {
  static_assert(!ap_int_props<T>::is_signed_v, "T must be unsigned");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  return static_cast<U>(static_cast<U>(lhs) - static_cast<U>(rhs));
}

// Helper function to calculate rectified difference i.e. max(0, lhs - rhs)
template <typename T>
T calc_rectified_diff(const T& lhs, const T& rhs) {
  static_assert(!ap_int_props<T>::is_signed_v, "T must be unsigned");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  return (lhs >= rhs) ? static_cast<T>(lhs - rhs) : static_cast<T>(0);
}

// Helper function to suppress value if condition is not met
template <typename B, typename T>
T take_value_if(B cond, const T& a) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  return cond ? a : static_cast<T>(0);
}

}  // namespace detail

// _____________________________________________________________________________
struct trkbuilding_internal_config {
  static const unsigned int num_site_segments = detail::num_chambers_max_allowed * num_emtf_segments;
  static const unsigned int num_gate_segments = num_site_segments / ((num_emtf_img_gates + 1) / 2);
  static const unsigned int num_th_median_entries = 9;

  // Typedefs
  typedef struct {
    emtf_phi_t emtf_phi;
    emtf_bend_t emtf_bend;
    emtf_theta1_t emtf_theta1;
    emtf_theta2_t emtf_theta2;
    emtf_qual1_t emtf_qual1;
    seg_valid_t seg_valid;
    trk_seg_t trk_seg;
    dio_ph_diff_t ph_diff;
  } stage_0_out_t;
};

// _____________________________________________________________________________
template <typename T_IN, typename T_OUT>
void trkbuilding_find_ph_median_op(const T_IN& curr_trk_col, T_OUT& ph_median, T_OUT& ph_sector) {
  static_assert(is_same<T_IN, trk_col_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, emtf_phi_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  constexpr int bits_to_shift = emtf_img_col_factor_log2;

  // Find curr_trk_col_corr
  const trk_col_t col_start_img = detail::chamber_img_joined_col_start;
  const trk_col_t curr_trk_col_corr = curr_trk_col + col_start_img;  // add offset

  // Find ph_median, ph_sector
  ph_median = ((static_cast<emtf_phi_t>(curr_trk_col_corr) << bits_to_shift) + (1u << (bits_to_shift - 1)));
  const trk_col_t col_sector = static_cast<trk_col_t>(num_emtf_img_cols / 2) + col_start_img;  // add offset
  ph_sector = ((static_cast<emtf_phi_t>(col_sector) << bits_to_shift) + (1u << (bits_to_shift - 1)));
}

// _____________________________________________________________________________
template <typename T = void>
void trkbuilding_match_ph_compute_op(const emtf_phi_t emtf_phi_mhph[trkbuilding_internal_config::num_site_segments],
                                     const seg_zones_t seg_zones_mhph[trkbuilding_internal_config::num_site_segments],
                                     const seg_tzones_t seg_tzones_mhph[trkbuilding_internal_config::num_site_segments],
                                     const seg_valid_t seg_valid_mhph[trkbuilding_internal_config::num_site_segments],
                                     const trk_col_t& curr_trk_col_corr, const bool_t& curr_trk_qual_gt_0,
                                     const trk_zone_t& curr_trk_zone, const trk_tzone_t& curr_trk_tzone,
                                     const dio_patt_param_t& col_start_param, const dio_patt_param_t& col_mid_param,
                                     const dio_patt_param_t& col_stop_param, const dio_patt_param_t& col_pad_param,
                                     dio_ph_diff_t ph_diff[trkbuilding_internal_config::num_site_segments],
                                     bool_t ph_diff_v[trkbuilding_internal_config::num_site_segments]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int num_site_segments = trkbuilding_internal_config::num_site_segments;
  typedef dio_ph_diff_t diff_t;
  const diff_t invalid_marker_ph_diff = ap_int_limits<diff_t>::max_value;

  constexpr int bits_to_shift = emtf_img_col_factor_log2;

  // Translate col_start, col_stop
  const trk_col_t col_start = curr_trk_col_corr + col_start_param;
  const trk_col_t col_stop = curr_trk_col_corr + col_stop_param;

  // Translate col_mid to col_patt: subtract padding
  // Translate col_patt to ph_patt: append 4 bits, then add bias
  const trk_col_t col_patt = curr_trk_col_corr + col_mid_param - col_pad_param;
  const emtf_phi_t ph_patt = ((static_cast<emtf_phi_t>(col_patt) << bits_to_shift) + (1u << (bits_to_shift - 1)));

  // Translate trk_zone, trk_tzone into bit selection
  constexpr int bit_sel_zone_hi = num_emtf_zones - 1;
  constexpr int bit_sel_tzone_hi = num_emtf_timezones - 1;
  const trk_zone_t bit_sel_zone = bit_sel_zone_hi - curr_trk_zone;
  const trk_tzone_t bit_sel_tzone = bit_sel_tzone_hi - curr_trk_tzone;

  // Loop over segments
LOOP_PHI_2:
  for (unsigned i = 0; i < num_site_segments; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Translate emtf_phi to col: truncate the last 4 bits, then add padding
    const emtf_phi_t ph0 = emtf_phi_mhph[i];
    const trk_col_t col = static_cast<trk_col_t>(ph0 >> bits_to_shift) + col_pad_param;

    // Condition: is_valid_seg && is_valid_trk && is_same_zone && is_same_timezone && is_in_window
    const bool valid =
        ((seg_valid_mhph[i] == 1) and (curr_trk_qual_gt_0 == 1) and (seg_zones_mhph[i][bit_sel_zone] == 1) and
         (seg_tzones_mhph[i][bit_sel_tzone] == 1) and (col_start <= col) and (col <= col_stop));

    // Calculate abs(delta-phi)
    const diff_t ph_diff_tmp = detail::calc_abs_diff(ph0, ph_patt);
    emtf_assert((ph_patt >> bits_to_shift) == col_patt);
    if (valid) {
      emtf_assert((curr_trk_col_corr + col_mid_param - col_pad_param) >= 0);
      emtf_assert(((ph_patt + ph_diff_tmp) == ph0) or ((ph_patt - ph_diff_tmp) == ph0));
    }

    // Output
    ph_diff[i] = valid ? ph_diff_tmp : invalid_marker_ph_diff;
    ph_diff_v[i] = valid;
  }  // end loop over segments
}

template <typename T>
void trkbuilding_match_ph_pack_op(const emtf_phi_t emtf_phi_mhph[trkbuilding_internal_config::num_site_segments],
                                  const emtf_bend_t emtf_bend_mhph[trkbuilding_internal_config::num_site_segments],
                                  const emtf_theta1_t emtf_theta1_mhph[trkbuilding_internal_config::num_site_segments],
                                  const emtf_theta2_t emtf_theta2_mhph[trkbuilding_internal_config::num_site_segments],
                                  const emtf_qual1_t emtf_qual1_mhph[trkbuilding_internal_config::num_site_segments],
                                  const trk_seg_t trk_seg_mhph[trkbuilding_internal_config::num_site_segments],
                                  const dio_ph_diff_t ph_diff[trkbuilding_internal_config::num_site_segments],
                                  const bool_t ph_diff_v[trkbuilding_internal_config::num_site_segments],
                                  const trk_col_t& curr_trk_col_corr,
                                  T stage_0_out[trkbuilding_internal_config::num_gate_segments]) {
  static_assert(is_same<T, trkbuilding_internal_config::stage_0_out_t>::value, "T type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int num_site_segments = trkbuilding_internal_config::num_site_segments;
  const unsigned int num_gate_segments = trkbuilding_internal_config::num_gate_segments;
  const unsigned int half_num_gate_segments = num_gate_segments / 2;
  const emtf_theta_t invalid_marker_th = detail::th_invalid;

  typedef trkbuilding_internal_config::stage_0_out_t stage_0_out_t;

  // Intermediate arrays
  stage_0_out_t stage_0_out_tmp[num_site_segments];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = stage_0_out_tmp complete dim = 0
#pragma HLS DATA_PACK variable = stage_0_out_tmp
  // hls-pragmas end

  // Loop over segments
LOOP_PHI_3:
  for (unsigned i = 0; i < num_site_segments; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const emtf_theta_t x_emtf_theta1 = ph_diff_v[i] ? emtf_theta1_mhph[i] : invalid_marker_th;
    const emtf_theta_t x_emtf_theta2 = ph_diff_v[i] ? emtf_theta2_mhph[i] : invalid_marker_th;
    const stage_0_out_t x_i = {emtf_phi_mhph[i],   emtf_bend_mhph[i], x_emtf_theta1,   x_emtf_theta2,
                               emtf_qual1_mhph[i], ph_diff_v[i],      trk_seg_mhph[i], ph_diff[i]};
    stage_0_out_tmp[i] = x_i;
  }  // end loop over segments

  // Select an 40-deg gate (overlapping window)
  // gate 0: 0 - 165
  // gate 1: 75 - 240
  // gate 2: 150 - 315
  // use edges (0, 120, 195, 315)
  const int half_chamber_img_bw = (detail::chamber_img_bw / 2);                                  // 45
  const trk_col_t col_stop_gate_0 = detail::chamber_ph_init_20deg_ext[0] + half_chamber_img_bw;  // 120
  const trk_col_t col_stop_gate_1 = detail::chamber_ph_init_20deg_ext[1] + half_chamber_img_bw;  // 195

  // Output
  trk_gate_t curr_trk_gate = 0;
  unsigned gate_begin_index = 0;

  if (curr_trk_col_corr < col_stop_gate_0) {
    curr_trk_gate = 0;
    gate_begin_index = static_cast<unsigned>(curr_trk_gate) * half_num_gate_segments;
    detail::copy_n_values<num_gate_segments>(&(stage_0_out_tmp[gate_begin_index]), stage_0_out);
  } else if (curr_trk_col_corr < col_stop_gate_1) {
    curr_trk_gate = 1;
    gate_begin_index = static_cast<unsigned>(curr_trk_gate) * half_num_gate_segments;
    detail::copy_n_values<num_gate_segments>(&(stage_0_out_tmp[gate_begin_index]), stage_0_out);
  } else {
    curr_trk_gate = 2;
    gate_begin_index = static_cast<unsigned>(curr_trk_gate) * half_num_gate_segments;
    detail::copy_n_values<num_gate_segments>(&(stage_0_out_tmp[gate_begin_index]), stage_0_out);
  }
}

template <typename T>
void trkbuilding_match_ph_argmin_op(const T in0[trkbuilding_internal_config::num_gate_segments],
                                    emtf_phi_t& feat_emtf_phi_site_k, emtf_bend_t& feat_emtf_bend_site_k,
                                    emtf_theta1_t& feat_emtf_theta1_site_k, emtf_theta2_t& feat_emtf_theta2_site_k,
                                    emtf_qual1_t& feat_emtf_qual1_site_k, trk_seg_t& ph_seg_site_k,
                                    bool_t& ph_seg_site_k_v) {
  static_assert(is_same<T, trkbuilding_internal_config::stage_0_out_t>::value, "T type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int N = trkbuilding_internal_config::num_gate_segments;
  typedef trkbuilding_internal_config::stage_0_out_t stage_0_out_t;
  typedef stage_0_out_t pair_t;

  // Binary tree structure (N must be an even number)
  const unsigned int num_nodes = (N * 2) - 1;

  pair_t binary_tree[num_nodes];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = binary_tree complete dim = 0
#pragma HLS DATA_PACK variable = binary_tree
  // hls-pragmas end

  // For N = 12, the binary tree is not balanced (as N is not a power of 2), we need to alter
  // the ordering of the nodes. By default:
  //     0 -> 1 2 -> 3 4 5 6 -> 7 8 9 10 11 12 13 14 -> 15 16 17 18 19 20 21 22 X X X X X X X X
  // In order to preserve order:
  //     0 -> 1 2 -> 3 4 5 6 -> 7 8 9 10 19 20 21 22 -> 11 12 13 14 15 16 17 18 X X X X X X X X

  // Fetch input
LOOP_ARGMIN_1:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned int node_index = (N - 1) + ((i + 4) % N);  // N-1 .. (N*2)-2 with rotation
    emtf_assert(node_index < num_nodes);

    binary_tree[node_index] = in0[i];
  }  // end fetch input loop

  // Tree reduce
LOOP_ARGMIN_2:
  for (int i = (N - 1) - 1; i >= 0; i--) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned int node_index = i;  // 0 .. N-2 in reverse order
    const unsigned int child_index = (2 * node_index) + 1;
    emtf_assert(node_index < num_nodes);
    emtf_assert(((child_index + 0) < num_nodes) and ((child_index + 1) < num_nodes));

    // Compare-swap
    const pair_t& lhs = binary_tree[child_index + 0];
    const pair_t& rhs = binary_tree[child_index + 1];
    const bool cmp = (lhs.ph_diff <= rhs.ph_diff);
    binary_tree[node_index] = cmp ? lhs : rhs;
  }  // end tree reduce loop

  // Output
  const pair_t& x_i = binary_tree[0];
  feat_emtf_phi_site_k = x_i.emtf_phi;
  feat_emtf_bend_site_k = x_i.emtf_bend;
  feat_emtf_theta1_site_k = x_i.emtf_theta1;
  feat_emtf_theta2_site_k = x_i.emtf_theta2;
  feat_emtf_qual1_site_k = x_i.emtf_qual1;
  ph_seg_site_k = x_i.trk_seg;
  ph_seg_site_k_v = x_i.seg_valid;
}

// _____________________________________________________________________________
template <typename Site>
void trkbuilding_match_ph_site_op(
    const emtf_phi_t emtf_phi[model_config::n_in], const emtf_bend_t emtf_bend[model_config::n_in],
    const emtf_theta1_t emtf_theta1[model_config::n_in], const emtf_theta2_t emtf_theta2[model_config::n_in],
    const emtf_qual1_t emtf_qual1[model_config::n_in], const seg_zones_t seg_zones[model_config::n_in],
    const seg_tzones_t seg_tzones[model_config::n_in], const seg_valid_t seg_valid[model_config::n_in],
    const trk_qual_t& curr_trk_qual, const trk_patt_t& curr_trk_patt, const trk_col_t& curr_trk_col,
    const trk_zone_t& curr_trk_zone, const trk_tzone_t& curr_trk_tzone, emtf_phi_t& feat_emtf_phi_site_k,
    emtf_bend_t& feat_emtf_bend_site_k, emtf_theta1_t& feat_emtf_theta1_site_k, emtf_theta2_t& feat_emtf_theta2_site_k,
    emtf_qual1_t& feat_emtf_qual1_site_k, trk_seg_t& ph_seg_site_k, bool_t& ph_seg_site_k_v) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int num_site_segments = trkbuilding_internal_config::num_site_segments;
  const unsigned int num_gate_segments = trkbuilding_internal_config::num_gate_segments;

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static int segment_id_table[num_site_segments];
#else
  bool initialized = false;
  int segment_id_table[num_site_segments];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    detail::init_table_op<num_site_segments>(segment_id_table, detail::get_segment_id_op<Site>());
  }

  // Intermediate arrays
  emtf_phi_t emtf_phi_mhph[num_site_segments];
  emtf_bend_t emtf_bend_mhph[num_site_segments];
  emtf_theta1_t emtf_theta1_mhph[num_site_segments];
  emtf_theta2_t emtf_theta2_mhph[num_site_segments];
  emtf_qual1_t emtf_qual1_mhph[num_site_segments];
  seg_zones_t seg_zones_mhph[num_site_segments];
  seg_tzones_t seg_tzones_mhph[num_site_segments];
  seg_valid_t seg_valid_mhph[num_site_segments];
  trk_seg_t trk_seg_mhph[num_site_segments];
  dio_ph_diff_t ph_diff[num_site_segments];
  bool_t ph_diff_v[num_site_segments];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = emtf_phi_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_bend_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_theta1_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_theta2_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_qual1_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_zones_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_tzones_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_valid_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_mhph complete dim = 0
#pragma HLS ARRAY_PARTITION variable = ph_diff complete dim = 0
#pragma HLS ARRAY_PARTITION variable = ph_diff_v complete dim = 0
  // hls-pragmas end

  typedef trkbuilding_internal_config::stage_0_out_t stage_0_out_t;

  // Intermediate arrays
  stage_0_out_t stage_0_out[num_gate_segments];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = stage_0_out complete dim = 0
#pragma HLS DATA_PACK variable = stage_0_out
  // hls-pragmas end

  // Loop over segments (incl those in fake chambers)
LOOP_PHI_1:
  for (unsigned i = 0; i < num_site_segments; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned iseg = static_cast<unsigned>(segment_id_table[i]);
    const bool valid = (iseg != model_config::n_in);

    if (valid) {
      // Valid chambers
      emtf_phi_mhph[i] = emtf_phi[iseg];
      emtf_bend_mhph[i] = emtf_bend[iseg];
      emtf_theta1_mhph[i] = emtf_theta1[iseg];
      emtf_theta2_mhph[i] = emtf_theta2[iseg];
      emtf_qual1_mhph[i] = emtf_qual1[iseg];
      seg_zones_mhph[i] = seg_zones[iseg];
      seg_tzones_mhph[i] = seg_tzones[iseg];
      seg_valid_mhph[i] = seg_valid[iseg];
    } else {
      // Fake chambers
      emtf_phi_mhph[i] = 0;
      emtf_bend_mhph[i] = 0;
      emtf_theta1_mhph[i] = 0;
      emtf_theta2_mhph[i] = 0;
      emtf_qual1_mhph[i] = 0;
      seg_zones_mhph[i] = 0;
      seg_tzones_mhph[i] = 0;
      seg_valid_mhph[i] = 0;
    }
    trk_seg_mhph[i] = static_cast<trk_seg_t>(iseg);
  }  // end loop over segments

  // Find curr_trk_col_corr
  const trk_col_t col_start_img = detail::chamber_img_joined_col_start;
  const trk_col_t col_stop_img = detail::chamber_img_joined_col_stop;
  const trk_col_t curr_trk_col_corr = curr_trk_col + col_start_img;  // add offset
  emtf_assert((col_start_img <= curr_trk_col_corr) and (curr_trk_col_corr <= col_stop_img));

  // Find curr_trk_qual_gt_0
  const bool_t curr_trk_qual_gt_0 = (curr_trk_qual > 0);  // require quality > 0

  // Retrieve pattern window params
  typedef make_concat<trk_zone_t, trk_patt_t>::type table_index_t;

  table_index_t table_index = (curr_trk_zone, curr_trk_patt);
  dio_patt_param_t col_start_param = 0;
  dio_patt_param_t col_mid_param = 0;
  dio_patt_param_t col_stop_param = 0;
  dio_patt_param_t col_pad_param = 0;
  detail::find_pattern_windows_op<Site>(table_index, col_start_param, col_mid_param, col_stop_param, col_pad_param);

  // Find ph_diff, ph_diff_v
  trkbuilding_match_ph_compute_op(emtf_phi_mhph, seg_zones_mhph, seg_tzones_mhph, seg_valid_mhph, curr_trk_col_corr,
                                  curr_trk_qual_gt_0, curr_trk_zone, curr_trk_tzone, col_start_param, col_mid_param,
                                  col_stop_param, col_pad_param, ph_diff, ph_diff_v);

  // Pack stage_0_out
  trkbuilding_match_ph_pack_op(emtf_phi_mhph, emtf_bend_mhph, emtf_theta1_mhph, emtf_theta2_mhph, emtf_qual1_mhph,
                               trk_seg_mhph, ph_diff, ph_diff_v, curr_trk_col_corr, stage_0_out);

  // Find ph_seg_site_k, ph_seg_site_k_v
  trkbuilding_match_ph_argmin_op(stage_0_out, feat_emtf_phi_site_k, feat_emtf_bend_site_k, feat_emtf_theta1_site_k,
                                 feat_emtf_theta2_site_k, feat_emtf_qual1_site_k, ph_seg_site_k, ph_seg_site_k_v);
}

template <typename T = void>
void trkbuilding_match_ph_op(
    const emtf_phi_t emtf_phi[model_config::n_in], const emtf_bend_t emtf_bend[model_config::n_in],
    const emtf_theta1_t emtf_theta1[model_config::n_in], const emtf_theta2_t emtf_theta2[model_config::n_in],
    const emtf_qual1_t emtf_qual1[model_config::n_in], const seg_zones_t seg_zones[model_config::n_in],
    const seg_tzones_t seg_tzones[model_config::n_in], const seg_valid_t seg_valid[model_config::n_in],
    const trk_qual_t& curr_trk_qual, const trk_patt_t& curr_trk_patt, const trk_col_t& curr_trk_col,
    const trk_zone_t& curr_trk_zone, const trk_tzone_t& curr_trk_tzone, emtf_phi_t feat_emtf_phi[num_emtf_sites],
    emtf_bend_t feat_emtf_bend[num_emtf_sites], emtf_theta_t feat_emtf_theta_ambi[num_emtf_sites * 2],
    emtf_qual_t feat_emtf_qual[num_emtf_sites], trk_seg_t ph_seg[num_emtf_sites], bool_t ph_seg_v[num_emtf_sites]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  auto feat_emtf_theta1 = &(feat_emtf_theta_ambi[0]);
  auto feat_emtf_theta2 = &(feat_emtf_theta_ambi[num_emtf_sites + 0]);

  // Loop over sites manually
  trkbuilding_match_ph_site_op<m_site_0_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[0], feat_emtf_bend[0],
      feat_emtf_theta1[0], feat_emtf_theta2[0], feat_emtf_qual[0], ph_seg[0], ph_seg_v[0]);
  trkbuilding_match_ph_site_op<m_site_1_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[1], feat_emtf_bend[1],
      feat_emtf_theta1[1], feat_emtf_theta2[1], feat_emtf_qual[1], ph_seg[1], ph_seg_v[1]);
  trkbuilding_match_ph_site_op<m_site_2_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[2], feat_emtf_bend[2],
      feat_emtf_theta1[2], feat_emtf_theta2[2], feat_emtf_qual[2], ph_seg[2], ph_seg_v[2]);
  trkbuilding_match_ph_site_op<m_site_3_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[3], feat_emtf_bend[3],
      feat_emtf_theta1[3], feat_emtf_theta2[3], feat_emtf_qual[3], ph_seg[3], ph_seg_v[3]);
  trkbuilding_match_ph_site_op<m_site_4_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[4], feat_emtf_bend[4],
      feat_emtf_theta1[4], feat_emtf_theta2[4], feat_emtf_qual[4], ph_seg[4], ph_seg_v[4]);
  trkbuilding_match_ph_site_op<m_site_5_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[5], feat_emtf_bend[5],
      feat_emtf_theta1[5], feat_emtf_theta2[5], feat_emtf_qual[5], ph_seg[5], ph_seg_v[5]);
  trkbuilding_match_ph_site_op<m_site_6_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[6], feat_emtf_bend[6],
      feat_emtf_theta1[6], feat_emtf_theta2[6], feat_emtf_qual[6], ph_seg[6], ph_seg_v[6]);
  trkbuilding_match_ph_site_op<m_site_7_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[7], feat_emtf_bend[7],
      feat_emtf_theta1[7], feat_emtf_theta2[7], feat_emtf_qual[7], ph_seg[7], ph_seg_v[7]);
  trkbuilding_match_ph_site_op<m_site_8_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[8], feat_emtf_bend[8],
      feat_emtf_theta1[8], feat_emtf_theta2[8], feat_emtf_qual[8], ph_seg[8], ph_seg_v[8]);
  trkbuilding_match_ph_site_op<m_site_9_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[9], feat_emtf_bend[9],
      feat_emtf_theta1[9], feat_emtf_theta2[9], feat_emtf_qual[9], ph_seg[9], ph_seg_v[9]);
  trkbuilding_match_ph_site_op<m_site_10_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[10], feat_emtf_bend[10],
      feat_emtf_theta1[10], feat_emtf_theta2[10], feat_emtf_qual[10], ph_seg[10], ph_seg_v[10]);
  trkbuilding_match_ph_site_op<m_site_11_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid, curr_trk_qual,
      curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi[11], feat_emtf_bend[11],
      feat_emtf_theta1[11], feat_emtf_theta2[11], feat_emtf_qual[11], ph_seg[11], ph_seg_v[11]);
}

// _____________________________________________________________________________
template <typename T_IN, typename T_OUT>
void trkbuilding_find_th_median_of_nine_op(const T_IN in0[trkbuilding_internal_config::num_th_median_entries],
                                           const bool_t in1[trkbuilding_internal_config::num_th_median_entries],
                                           T_OUT& out, bool_t& vld) {
  static_assert(is_same<T_IN, emtf_theta_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, emtf_theta_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int N = trkbuilding_internal_config::num_th_median_entries;
  typedef T_IN data_t;
  typedef bool_t arg_t;
  typedef detail::argsort_pair<arg_t, data_t> pair_t;

  // Use a different invalid_marker_th_1, because the usual invalid_marker_th, which is 0,
  // doesn't work in the following median sort.
  const data_t invalid_marker_th = detail::th_invalid;
  const data_t invalid_marker_th_1 = ap_int_limits<data_t>::max_value;

  // Ternary tree structure (N must be power of 3)
  const unsigned int num_nodes = ((N * 3) - 1) / 2;  // = 13

  pair_t ternary_tree[num_nodes];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = ternary_tree complete dim = 0
#pragma HLS DATA_PACK variable = ternary_tree
  // hls-pragmas end

  // Fetch input
LOOP_MEDIAN_1:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned int node_index = ((N - 1) / 2) + i;  // 4 .. 12
    emtf_assert(node_index < num_nodes);

    // Make pairs
    // If invalid, insert invalid_marker_th_1
    const data_t data = in1[i] ? in0[i] : invalid_marker_th_1;
    const arg_t arg = in1[i];
    ternary_tree[node_index] = pair_t(arg, data);
  }  // end fetch input loop

  // Tree reduce
LOOP_MEDIAN_2:
  for (int i = ((N - 1) / 2) - 1; i >= 0; i--) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned int node_index = i;  // 0 .. 3 in reverse order
    const unsigned int child_index = (3 * node_index) + 1;
    emtf_assert(node_index < num_nodes);
    emtf_assert(((child_index + 0) < num_nodes) and ((child_index + 2) < num_nodes));

    detail::median_of_three_op(ternary_tree[child_index + 0], ternary_tree[child_index + 1],
                               ternary_tree[child_index + 2], ternary_tree[node_index]);
  }  // end tree reduce loop

  // Output
  // If invalid, insert invalid_marker_th
  out = ternary_tree[0].first ? ternary_tree[0].second : invalid_marker_th;
  vld = ternary_tree[0].first;
}

template <typename T_IN, typename T_OUT>
void trkbuilding_find_th_median_op(const T_IN feat_emtf_theta_ambi[num_emtf_sites * 2], T_OUT& th_median) {
  static_assert(is_same<T_IN, emtf_theta_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, emtf_theta_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  const unsigned int num_theta_values = trkbuilding_internal_config::num_th_median_entries;
  const emtf_theta_t invalid_marker_th = detail::th_invalid;

#ifndef __SYNTHESIS__
  static bool initialized = false;
  static int theta_indices_table[num_theta_values];
  static int theta_indices_alt_table[num_theta_values];
  static int theta_indices_me1_table[num_theta_values];
#else
  bool initialized = false;
  int theta_indices_table[num_theta_values];
  int theta_indices_alt_table[num_theta_values];
  int theta_indices_me1_table[num_theta_values];
#endif  // __SYNTHESIS__ not defined

  if (!initialized) {
    initialized = true;
    detail::init_table_op<num_theta_values>(theta_indices_table, detail::get_trk_theta_indices_op());
    detail::init_table_op<num_theta_values>(theta_indices_alt_table, detail::get_trk_theta_indices_alt_op());
    detail::init_table_op<num_theta_values>(theta_indices_me1_table, detail::get_trk_theta_indices_me1_op());
  }

  // Intermediate arrays
  emtf_theta_t theta_values[num_theta_values];
  bool_t theta_values_v[num_theta_values];
  emtf_theta_t theta_values_me1[num_theta_values];
  bool_t theta_values_me1_v[num_theta_values];

  // hls-pragmas begin
#pragma HLS ARRAY_RESHAPE variable = theta_values complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = theta_values_v complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = theta_values_me1 complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = theta_values_me1_v complete dim = 0
  // hls-pragmas end

  // Loop over theta values
LOOP_THETA_1:
  for (unsigned i = 0; i < num_theta_values; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const emtf_theta_t& th0 = feat_emtf_theta_ambi[theta_indices_table[i]];
    const emtf_theta_t& th0_alt = feat_emtf_theta_ambi[theta_indices_alt_table[i]];
    const emtf_theta_t& th0_me1 = feat_emtf_theta_ambi[theta_indices_me1_table[i]];
    emtf_assert((theta_indices_table[i] < (num_emtf_sites * 2)) and
                (theta_indices_alt_table[i] < (num_emtf_sites * 2)) and
                (theta_indices_me1_table[i] < (num_emtf_sites * 2)));

    // ME2,3,4
    if (theta_indices_table[i] != theta_indices_alt_table[i]) {
      // Mux of 2
      theta_values[i] = (th0 != invalid_marker_th) ? th0 : th0_alt;
      theta_values_v[i] = (th0 != invalid_marker_th) or (th0_alt != invalid_marker_th);
    } else {
      theta_values[i] = th0;
      theta_values_v[i] = (th0 != invalid_marker_th);
    }
    // ME1
    theta_values_me1[i] = th0_me1;
    theta_values_me1_v[i] = (th0_me1 != invalid_marker_th);
  }  // end loop over theta values

  // Find th_median, considering two different scenarios:
  // - In stations 2,3,4
  // - In station 1 only
  emtf_theta_t th_median_tmp_0 = 0;
  emtf_theta_t th_median_tmp_1 = 0;
  bool_t th_median_vld_0 = 0;
  bool_t th_median_vld_1 = 0;

  // If valid, use th_median from stations 2,3,4; else, use th_median from station 1.
  trkbuilding_find_th_median_of_nine_op(theta_values, theta_values_v, th_median_tmp_0, th_median_vld_0);
  trkbuilding_find_th_median_of_nine_op(theta_values_me1, theta_values_me1_v, th_median_tmp_1, th_median_vld_1);
  th_median = th_median_vld_0 ? th_median_tmp_0 : th_median_tmp_1;
}

// _____________________________________________________________________________
template <typename T_IN, typename T_OUT>
void trkbuilding_match_th_select_op(const T_IN& th0, const T_IN& th1, const T_IN& th_median, T_OUT& out, bool_t& vld) {
  static_assert(is_same<T_IN, emtf_theta_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, emtf_theta_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  typedef T_IN data_t;
  typedef dio_th_diff_t diff_t;
  const data_t invalid_marker_th = detail::th_invalid;
  const diff_t invalid_marker_th_diff = ap_int_limits<diff_t>::max_value;
  const diff_t th_window = detail::th_window;

  // Calculate abs(delta-theta)
  const data_t th_diff_tmp_0 = detail::calc_abs_diff(th0, th_median);
  const data_t th_diff_tmp_1 = detail::calc_abs_diff(th1, th_median);
  const diff_t th_diff_0 =
      (((th0 != invalid_marker_th) and (th_diff_tmp_0 <= invalid_marker_th_diff)) ? static_cast<diff_t>(th_diff_tmp_0)
                                                                                  : invalid_marker_th_diff);
  const diff_t th_diff_1 =
      (((th1 != invalid_marker_th) and (th_diff_tmp_1 <= invalid_marker_th_diff)) ? static_cast<diff_t>(th_diff_tmp_1)
                                                                                  : invalid_marker_th_diff);
  const bool_t th_diff_0_v = (th_diff_0 < th_window);
  const bool_t th_diff_1_v = (th_diff_1 < th_window);
  const data_t th_sel = (th_diff_0 <= th_diff_1) ? th0 : th1;

  // Output
  out = (th_diff_0_v | th_diff_1_v) ? th_sel : invalid_marker_th;
  vld = (th_diff_0_v | th_diff_1_v);
}

template <typename T_IN, typename T_OUT>
void trkbuilding_match_th_select_op(const T_IN& th0, const T_IN& th_median, T_OUT& out, bool_t& vld) {
  static_assert(is_same<T_IN, emtf_theta_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, emtf_theta_t>::value, "T_OUT type check failed");

  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  typedef T_IN data_t;
  typedef dio_th_diff_t diff_t;
  const data_t invalid_marker_th = detail::th_invalid;
  const diff_t invalid_marker_th_diff = ap_int_limits<diff_t>::max_value;
  const diff_t th_window = detail::th_window;

  // Calculate abs(delta-theta)
  const data_t th_diff_tmp_0 = detail::calc_abs_diff(th0, th_median);
  const diff_t th_diff_0 =
      (((th0 != invalid_marker_th) and (th_diff_tmp_0 <= invalid_marker_th_diff)) ? static_cast<diff_t>(th_diff_tmp_0)
                                                                                  : invalid_marker_th_diff);
  const bool_t th_diff_0_v = (th_diff_0 < th_window);

  // Output
  out = th_diff_0_v ? th0 : invalid_marker_th;
  vld = th_diff_0_v;
}

template <typename T = void>
void trkbuilding_match_th_op(const emtf_theta_t feat_emtf_theta_ambi[num_emtf_sites * 2], const emtf_theta_t& th_median,
                             emtf_theta_t feat_emtf_theta[num_emtf_sites], trk_seg_t th_seg[num_emtf_sites],
                             bool_t th_seg_v[num_emtf_sites]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  //#pragma HLS INLINE
  // hls-pragmas end

  // Find the most compatible theta values, also apply theta window cut
LOOP_THETA_2:
  for (unsigned i = 0; i < num_emtf_sites; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const emtf_theta_t& th0 = feat_emtf_theta_ambi[i];
    const emtf_theta_t& th1 = feat_emtf_theta_ambi[num_emtf_sites + i];

    // Note: th_seg is ignored.
    if (i < 5) {  // CSC
      trkbuilding_match_th_select_op(th0, th1, th_median, feat_emtf_theta[i], th_seg_v[i]);
    } else {  // non-CSC
      trkbuilding_match_th_select_op(th0, th_median, feat_emtf_theta[i], th_seg_v[i]);
    }
  }  // end loop over theta values

  // Sanity check
#ifndef __SYNTHESIS__
  const emtf_theta_t invalid_marker_th = detail::th_invalid;
  bool is_any_valid = false;
  for (unsigned i = 0; i < num_emtf_sites; i++) {
    const emtf_theta_t& th0 = feat_emtf_theta_ambi[i];
    const emtf_theta_t& th1 = feat_emtf_theta_ambi[num_emtf_sites + i];
    if ((th0 != invalid_marker_th) or (th1 != invalid_marker_th)) {
      is_any_valid = true;
      break;
    }
  }
  // th_median must be valid if at least one theta value is valid
  emtf_assert((is_any_valid and (th_median != invalid_marker_th)) or not is_any_valid);
#endif  // __SYNTHESIS__ not defined
}

// _____________________________________________________________________________
template <typename T = void>
void trkbuilding_extract_features_op(const emtf_phi_t feat_emtf_phi[num_emtf_sites],
                                     const emtf_bend_t feat_emtf_bend[num_emtf_sites],
                                     const emtf_theta_t feat_emtf_theta[num_emtf_sites],
                                     const emtf_qual_t feat_emtf_qual[num_emtf_sites],
                                     const trk_seg_t ph_seg[num_emtf_sites], const bool_t th_seg_v[num_emtf_sites],
                                     const emtf_phi_t& ph_median, const emtf_phi_t& ph_sector,
                                     const emtf_theta_t& th_median, const trk_qual_t& curr_trk_qual,
                                     trk_seg_t curr_trk_seg[num_emtf_sites], trk_seg_v_t& curr_trk_seg_v,
                                     trk_feat_t curr_trk_feat[num_emtf_features], trk_valid_t& curr_trk_valid) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // clang-format off
  // Table showing the first 36 features sent to NN
  //
  // feat       | ME1/1 | ME1/2 |  ME2  |  ME3  |  ME4  |  RE1  |  RE2  |  RE3  |  RE4  | GE1/1 | GE2/1 |  ME0
  // -----------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------
  // emtf_phi   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *
  // emtf_theta |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *   |   *
  // emtf_bend  |   *   |   *   |   *   |   *   |   *   |       |       |       |       |       |       |   *
  // emtf_qual  |   *   |   *   |   *   |   *   |   *   |       |       |       |       |       |       |   *
  // emtf_time  |       |       |       |       |       |       |       |       |       |       |       |
  //
  // 4 additional features are: ph_median, th_median, trk_qual, trk_bx
  // clang-format on

  typedef make_signed<make_wider<emtf_phi_t>::type>::type emtf_phi_signed_t;
  typedef make_signed<make_wider<emtf_theta_t>::type>::type emtf_theta_signed_t;

  // Intermediate arrays
  emtf_phi_signed_t feat_emtf_phi_signed[num_emtf_sites];
  emtf_theta_signed_t feat_emtf_theta_signed[num_emtf_sites];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = feat_emtf_phi_signed complete dim = 0
#pragma HLS ARRAY_PARTITION variable = feat_emtf_theta_signed complete dim = 0
  // hls-pragmas end

  // Subtract ph_median and th_median from phi and theta values
LOOP_EXTRACT_1:
  for (unsigned i = 0; i < num_emtf_sites; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    feat_emtf_phi_signed[i] = detail::calc_signed_diff(feat_emtf_phi[i], ph_median);
    feat_emtf_theta_signed[i] = detail::calc_signed_diff(feat_emtf_theta[i], th_median);
  }  // end loop over phi and theta values

  const emtf_phi_signed_t ph_median_signed = detail::calc_signed_diff(ph_median, ph_sector);

  // Find curr_trk_qual_gt_0
  const bool_t curr_trk_qual_gt_0 = (curr_trk_qual > 0);  // require quality > 0

  // Set curr_trk_seg, curr_trk_seg_v
LOOP_EXTRACT_2:
  for (unsigned i = 0; i < num_emtf_sites; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    curr_trk_seg[i] = ph_seg[i];
    curr_trk_seg_v[i] = th_seg_v[i];
  }  // end loop over seg indices

  // Set curr_trk_feat
  {
    unsigned i = 0;

    // emtf_phi
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[0], feat_emtf_phi_signed[0]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[1], feat_emtf_phi_signed[1]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[2], feat_emtf_phi_signed[2]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[3], feat_emtf_phi_signed[3]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[4], feat_emtf_phi_signed[4]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[5], feat_emtf_phi_signed[5]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[6], feat_emtf_phi_signed[6]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[7], feat_emtf_phi_signed[7]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[8], feat_emtf_phi_signed[8]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[9], feat_emtf_phi_signed[9]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[10], feat_emtf_phi_signed[10]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[11], feat_emtf_phi_signed[11]);
    // emtf_theta
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[0], feat_emtf_theta_signed[0]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[1], feat_emtf_theta_signed[1]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[2], feat_emtf_theta_signed[2]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[3], feat_emtf_theta_signed[3]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[4], feat_emtf_theta_signed[4]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[5], feat_emtf_theta_signed[5]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[6], feat_emtf_theta_signed[6]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[7], feat_emtf_theta_signed[7]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[8], feat_emtf_theta_signed[8]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[9], feat_emtf_theta_signed[9]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[10], feat_emtf_theta_signed[10]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[11], feat_emtf_theta_signed[11]);
    // emtf_bend
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[0], feat_emtf_bend[0]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[1], feat_emtf_bend[1]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[2], feat_emtf_bend[2]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[3], feat_emtf_bend[3]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[4], feat_emtf_bend[4]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[11], feat_emtf_bend[11]);
    // emtf_qual
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[0], feat_emtf_qual[0]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[1], feat_emtf_qual[1]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[2], feat_emtf_qual[2]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[3], feat_emtf_qual[3]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[4], feat_emtf_qual[4]);
    curr_trk_feat[i++] = detail::take_value_if(th_seg_v[11], feat_emtf_qual[11]);
    // additional features
    curr_trk_feat[i++] = detail::take_value_if(curr_trk_qual_gt_0, ph_median_signed);
    curr_trk_feat[i++] = detail::take_value_if(curr_trk_qual_gt_0, th_median);
    curr_trk_feat[i++] = curr_trk_qual;
    curr_trk_feat[i++] = 0;  // unused
    emtf_assert(i == num_emtf_features);
  }

  // Set curr_trk_valid
  {
    curr_trk_valid = static_cast<bool>(curr_trk_seg_v);  // bitwise OR reduced
  }
}

// _____________________________________________________________________________
// Track building op

template <typename Zone>
void trkbuilding_op(const emtf_phi_t emtf_phi[model_config::n_in], const emtf_bend_t emtf_bend[model_config::n_in],
                    const emtf_theta1_t emtf_theta1[model_config::n_in],
                    const emtf_theta2_t emtf_theta2[model_config::n_in],
                    const emtf_qual1_t emtf_qual1[model_config::n_in],
                    const emtf_qual2_t emtf_qual2[model_config::n_in], const emtf_time_t emtf_time[model_config::n_in],
                    const seg_zones_t seg_zones[model_config::n_in], const seg_tzones_t seg_tzones[model_config::n_in],
                    const seg_cscfr_t seg_cscfr[model_config::n_in], const seg_gemdl_t seg_gemdl[model_config::n_in],
                    const seg_bx_t seg_bx[model_config::n_in], const seg_valid_t seg_valid[model_config::n_in],
                    const trk_qual_t& curr_trk_qual, const trk_patt_t& curr_trk_patt, const trk_col_t& curr_trk_col,
                    const trk_zone_t& curr_trk_zone, const trk_tzone_t& curr_trk_tzone,
                    trk_seg_t curr_trk_seg[num_emtf_sites], trk_seg_v_t& curr_trk_seg_v,
                    trk_feat_t curr_trk_feat[num_emtf_features], trk_valid_t& curr_trk_valid) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Intermediate arrays
  emtf_phi_t feat_emtf_phi[num_emtf_sites];
  emtf_bend_t feat_emtf_bend[num_emtf_sites];
  emtf_theta_t feat_emtf_theta_ambi[num_emtf_sites * 2];
  emtf_theta_t feat_emtf_theta[num_emtf_sites];
  emtf_qual_t feat_emtf_qual[num_emtf_sites];
  trk_seg_t ph_seg[num_emtf_sites];
  bool_t ph_seg_v[num_emtf_sites];
  trk_seg_t th_seg[num_emtf_sites];
  bool_t th_seg_v[num_emtf_sites];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = feat_emtf_phi complete dim = 0
#pragma HLS ARRAY_PARTITION variable = feat_emtf_bend complete dim = 0
#pragma HLS ARRAY_PARTITION variable = feat_emtf_theta_ambi complete dim = 0
#pragma HLS ARRAY_PARTITION variable = feat_emtf_theta complete dim = 0
#pragma HLS ARRAY_PARTITION variable = feat_emtf_qual complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = ph_seg complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = ph_seg_v complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = th_seg complete dim = 0
#pragma HLS ARRAY_RESHAPE variable = th_seg_v complete dim = 0
  // hls-pragmas end

  emtf_phi_t ph_median = 0;
  emtf_phi_t ph_sector = 0;
  emtf_theta_t th_median = 0;

  trkbuilding_find_ph_median_op(curr_trk_col, ph_median, ph_sector);

  trkbuilding_match_ph_op(emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, seg_zones, seg_tzones, seg_valid,
                          curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, feat_emtf_phi,
                          feat_emtf_bend, feat_emtf_theta_ambi, feat_emtf_qual, ph_seg, ph_seg_v);

  trkbuilding_find_th_median_op(feat_emtf_theta_ambi, th_median);

  trkbuilding_match_th_op(feat_emtf_theta_ambi, th_median, feat_emtf_theta, th_seg, th_seg_v);

  // Note: only ph_seg and th_seg_v are used. th_seg and ph_seg_v are ignored.
  trkbuilding_extract_features_op(feat_emtf_phi, feat_emtf_bend, feat_emtf_theta, feat_emtf_qual, ph_seg, th_seg_v,
                                  ph_median, ph_sector, th_median, curr_trk_qual, curr_trk_seg, curr_trk_seg_v,
                                  curr_trk_feat, curr_trk_valid);
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void trkbuilding_layer(const emtf_phi_t emtf_phi[model_config::n_in], const emtf_bend_t emtf_bend[model_config::n_in],
                       const emtf_theta1_t emtf_theta1[model_config::n_in],
                       const emtf_theta2_t emtf_theta2[model_config::n_in],
                       const emtf_qual1_t emtf_qual1[model_config::n_in],
                       const emtf_qual2_t emtf_qual2[model_config::n_in],
                       const emtf_time_t emtf_time[model_config::n_in], const seg_zones_t seg_zones[model_config::n_in],
                       const seg_tzones_t seg_tzones[model_config::n_in],
                       const seg_cscfr_t seg_cscfr[model_config::n_in], const seg_gemdl_t seg_gemdl[model_config::n_in],
                       const seg_bx_t seg_bx[model_config::n_in], const seg_valid_t seg_valid[model_config::n_in],
                       const trk_qual_t& curr_trk_qual, const trk_patt_t& curr_trk_patt, const trk_col_t& curr_trk_col,
                       const trk_zone_t& curr_trk_zone, const trk_tzone_t& curr_trk_tzone,
                       trk_seg_t curr_trk_seg[num_emtf_sites], trk_seg_v_t& curr_trk_seg_v,
                       trk_feat_t curr_trk_feat[num_emtf_features], trk_valid_t& curr_trk_valid) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::layer_target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  // hls-pragmas end

  // Check assumptions
  static_assert(trkbuilding_config::n_in == num_emtf_tracks, "trkbuilding_config::n_in check failed");
  static_assert(trkbuilding_config::n_out == num_emtf_tracks, "trkbuilding_config::n_out check failed");
  static_assert(num_emtf_sites == 12, "num_emtf_sites must be 12");
  static_assert(num_emtf_features == 40, "num_emtf_features must be 40");
  static_assert(num_emtf_img_gates == 3, "num_emtf_img_gates must be 3");
  static_assert(dio_ph_diff_idx_t::width == detail::ceil_log2<trkbuilding_internal_config::num_site_segments>::value,
                "dio_ph_diff_idx_t type check failed");

  trkbuilding_op<Zone>(emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, emtf_qual2, emtf_time, seg_zones,
                       seg_tzones, seg_cscfr, seg_gemdl, seg_bx, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col,
                       curr_trk_zone, curr_trk_tzone, curr_trk_seg, curr_trk_seg_v, curr_trk_feat, curr_trk_valid);
}

}  // namespace phase2

}  // namespace emtf_hlslib

#endif  // __EMTF_HLSLIB_TRKBUILDING_H__ not defined
