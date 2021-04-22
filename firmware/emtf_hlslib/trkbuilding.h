#ifndef __EMTF_HLSLIB_TRKBUILDING_H__
#define __EMTF_HLSLIB_TRKBUILDING_H__

namespace emtf {

template <typename Category, typename T_SEL, typename T_IN, typename T_OUT>
void trkbuilding_reduce_argmin_ph_diff_op(
    const T_SEL& area,
    const T_IN in0[details::site_num_segments_traits<Category>::value],
    T_OUT& out,
    bool_t& vld
) {
  static_assert(is_same<T_SEL, dio_ph_area_t>::value, "T_IN type check failed");
  static_assert(is_same<T_IN, dio_ph_diff_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, dio_ph_idx_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const unsigned int N = details::area_num_segments_traits<Category>::value;
  emtf_assert(N == 4 or N == 8 or N == 12);

  const T_OUT area_step_size = N / 2;  // used to address in0, and used for output
  const T_OUT area_start = area * area_step_size;

  typedef ap_uint<details::ceil_log2<N-1>::value> idx_t;  // encodes 0..N-1
  typedef T_IN data_t;
  typedef details::argsort_pair<idx_t, data_t> pair_t;

  // Binary tree structure (N must be an even number)
  const unsigned int num_nodes_in = N;
  const unsigned int num_nodes_io = N - 1;
  const unsigned int num_nodes = num_nodes_in + num_nodes_io;

  pair_t nodes[num_nodes];

#pragma HLS DATA_PACK variable=nodes

#pragma HLS ARRAY_PARTITION variable=nodes complete dim=0

  LOOP_REGION: {

#pragma HLS LOOP_MERGE

    // Fetch input
    LOOP_NOD_1: for (unsigned i = 0; i < num_nodes_in; i++) {

#pragma HLS UNROLL

      const unsigned int node_index = (num_nodes - 1) - i;
      const unsigned int node_index_in = (num_nodes_in - 1) - i;
      nodes[node_index] = pair_t(idx_t(node_index_in), in0[area_start + node_index_in]);
    }

    // Binary-tree reduce
    LOOP_NOD_2: for (unsigned i = 0; i < num_nodes_io; i++) {

#pragma HLS UNROLL

      const unsigned int node_index = (num_nodes_io - 1) - i;
      const unsigned int child_l_index = (2 * node_index) + 1;
      const unsigned int child_r_index = (2 * node_index) + 2;
      emtf_assert((child_l_index < num_nodes) and (child_r_index < num_nodes));
      nodes[node_index] = (nodes[child_l_index] <= nodes[child_r_index]) ? nodes[child_l_index] : nodes[child_r_index];
    }
  }  // end loop region

  // Output
  const idx_t idx = nodes[0].first;
  const data_t invalid_marker = find_ap_int_max_allowed<data_t>::value;
  out = (area_start + idx);
  vld = (nodes[0].second != invalid_marker);
}

template <typename T_IN, typename T_OUT>
void trkbuilding_select_phi_median_op(const T_IN& in0, T_OUT& out) {
  static_assert(is_same<T_IN, trk_col_t>::value, "T_IN0 type check failed");
  static_assert(is_same<T_OUT, emtf_phi_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  constexpr int bits_to_shift = emtf_img_col_factor_log2;
  const trk_col_t col = in0 + static_cast<trk_col_t>(details::chamber_img_joined_col_start);
  out = (static_cast<emtf_phi_t>(col) << bits_to_shift) + (1u << (bits_to_shift - 1));
}

template <typename T_IN, typename T_OUT>
void trkbuilding_reduce_median_of_three_op(const T_IN& in0, const T_IN& in1, const T_IN& in2, T_OUT& out) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef T_IN data_t;
  const data_t invalid_marker_th = details::th_invalid;

  const bool vld0 = (in0 != invalid_marker_th);
  const bool vld1 = (in1 != invalid_marker_th);
  const bool vld2 = (in2 != invalid_marker_th);

  // Choose b if (a <= b <= c) or (a > b > c) or when a & c are invalid
  const bool sel0 = vld0 and (
      ((not vld2 or (in2 <= in0)) and (vld1 and (in0 <= in1))) or \
      ((vld2 and not (in2 <= in0)) and (not vld1 or not (in0 <= in1))) or \
      (not vld2 and not vld1)
  );
  const bool sel1 = vld1 and (
      ((not vld0 or (in0 <= in1)) and (vld2 and (in1 <= in2))) or \
      ((vld0 and not (in0 <= in1)) and (not vld2 or not (in1 <= in2))) or \
      (not vld0 and not vld2)
  );
  const bool sel2 = vld2 and (
      ((not vld1 or (in1 <= in2)) and (vld0 and (in2 <= in0))) or \
      ((vld1 and not (in1 <= in2)) and (not vld0 or not (in2 <= in0))) or \
      (not vld1 and not vld0)
  );

  // Output
  if (sel1) {
    out = in1;
  } else if (sel0) {
    out = in0;
  } else if (sel2) {
    out = in2;
  } else {
    out = invalid_marker_th;
  }
}

template <typename T_IN, typename T_OUT>
void trkbuilding_reduce_median_theta_op(const T_IN in0[9], T_OUT& out) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef T_IN data_t;

  data_t stage_0_out[3];

#pragma HLS ARRAY_PARTITION variable=stage_0_out complete dim=0

  // Split 9 entries into 3 groups of 3, find median in each group, then find median of medians
  trkbuilding_reduce_median_of_three_op(in0[0], in0[1], in0[2], stage_0_out[0]);
  trkbuilding_reduce_median_of_three_op(in0[3], in0[4], in0[5], stage_0_out[1]);
  trkbuilding_reduce_median_of_three_op(in0[6], in0[7], in0[8], stage_0_out[2]);

  // Output
  trkbuilding_reduce_median_of_three_op(stage_0_out[0], stage_0_out[1], stage_0_out[2], out);
}

template <typename T_IN, typename T_OUT>
void trkbuilding_select_theta_op(const T_IN& in0, const T_IN& in1, const T_IN& th_median, T_OUT& out, bool_t& vld) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef T_IN data_t;
  typedef dio_th_diff_t difference_t;
  const data_t invalid_marker_th = details::th_invalid;
  const difference_t invalid_marker_th_diff = find_ap_int_max_allowed<difference_t>::value;
  const difference_t th_window = details::th_window;

  const data_t th_diff_tmp_0 = details::calc_abs_diff(in0, th_median);
  const data_t th_diff_tmp_1 = details::calc_abs_diff(in1, th_median);
  const difference_t th_diff_0 = (th_diff_tmp_0 < invalid_marker_th_diff) ? static_cast<difference_t>(th_diff_tmp_0) : invalid_marker_th_diff;
  const difference_t th_diff_1 = (th_diff_tmp_1 < invalid_marker_th_diff) ? static_cast<difference_t>(th_diff_tmp_1) : invalid_marker_th_diff;
  const bool vld0 = (in0 != invalid_marker_th) and (th_diff_0 < th_window);
  const bool vld1 = (in1 != invalid_marker_th) and (th_diff_1 < th_window);

  // Output
  if (vld0 and vld1) {
    out = (th_diff_0 < th_diff_1) ? in0 : in1;
    vld = true;
  } else if (vld0) {
    out = in0;
    vld = true;
  } else if (vld1) {
    out = in1;
    vld = true;
  } else {
    out = invalid_marker_th;
    vld = false;
  }
}

template <typename T_IN, typename T_OUT>
void trkbuilding_select_theta_op(const T_IN& in0, const T_IN& th_median, T_OUT& out, bool_t& vld) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef T_IN data_t;
  typedef dio_th_diff_t difference_t;
  const data_t invalid_marker_th = details::th_invalid;
  const difference_t invalid_marker_th_diff = find_ap_int_max_allowed<difference_t>::value;
  const difference_t th_window = details::th_window;

  const data_t th_diff_tmp_0 = details::calc_abs_diff(in0, th_median);
  const difference_t th_diff_0 = (th_diff_tmp_0 < invalid_marker_th_diff) ? static_cast<difference_t>(th_diff_tmp_0) : invalid_marker_th_diff;
  const bool vld0 = (in0 != invalid_marker_th) and (th_diff_0 < th_window);

  // Output
  if (vld0) {
    out = in0;
    vld = true;
  } else {
    out = invalid_marker_th;
    vld = false;
  }
}

// _____________________________________________________________________________
// Perform loop over chambers and all the segments in the chambers.
// Find the best matches in phi.

template <typename Site>
void trkbuilding_match_ph_site_op(
    const emtf_phi_t emtf_phi[model_config::n_in],
    const seg_zones_t seg_zones[model_config::n_in],
    const seg_tzones_t seg_tzones[model_config::n_in],
    const seg_valid_t seg_valid[model_config::n_in],
    const trk_qual_t& curr_trk_qual,
    const trk_patt_t& curr_trk_patt,
    const trk_col_t& curr_trk_col,
    const trk_zone_t& curr_trk_zone,
    const trk_tzone_t& curr_trk_tzone,
    trk_seg_t& curr_trk_seg_site_i,
    bool_t& curr_trk_seg_v_site_i
) {

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef typename details::chamber_category_traits<Site>::chamber_category chamber_category;
  const unsigned int N = details::site_num_segments_traits<chamber_category>::value;
  emtf_assert(N == 8 or N == 16 or N == 24);

  int segment_id_table[N];
  details::init_table_op<N>(segment_id_table, details::get_site_segment_id_op<Site>());

  int pattern_col_start_table[num_emtf_zones * num_emtf_patterns];
  details::init_2d_table_op<num_emtf_zones, num_emtf_patterns>(pattern_col_start_table, details::get_site_pattern_col_start_op<Site>());

  int pattern_col_mid_table[num_emtf_zones * num_emtf_patterns];
  details::init_2d_table_op<num_emtf_zones, num_emtf_patterns>(pattern_col_mid_table, details::get_site_pattern_col_mid_op<Site>());

  int pattern_col_stop_table[num_emtf_zones * num_emtf_patterns];
  details::init_2d_table_op<num_emtf_zones, num_emtf_patterns>(pattern_col_stop_table, details::get_site_pattern_col_stop_op<Site>());

  int pattern_col_pad_table[num_emtf_zones];
  details::init_table_op<num_emtf_zones>(pattern_col_pad_table, details::get_site_pattern_col_pad_op<Site>());

  // Find segments matched to the pattern
  dio_ph_diff_t site_ph_diff[N];

#pragma HLS ARRAY_PARTITION variable=site_ph_diff complete dim=0

  const trk_seg_t invalid_marker_ph_seg = model_config::n_in;
  const dio_ph_diff_t invalid_marker_ph_diff = find_ap_int_max_allowed<dio_ph_diff_t>::value;
  const ap_uint<details::ceil_log2<num_emtf_patterns>::value> npatts = num_emtf_patterns;

  const trk_col_t col_patt = curr_trk_col + static_cast<trk_col_t>(details::chamber_img_joined_col_start);
  const trk_col_t col_start = col_patt + pattern_col_start_table[(curr_trk_zone * npatts) + curr_trk_patt];
  const trk_col_t col_mid = col_patt + pattern_col_mid_table[(curr_trk_zone * npatts) + curr_trk_patt];
  const trk_col_t col_stop = col_patt + pattern_col_stop_table[(curr_trk_zone * npatts) + curr_trk_patt];
  const trk_col_t col_pad = pattern_col_pad_table[curr_trk_zone];

  // Loop over segments (incl those in fake chambers)
  LOOP_SEG: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const trk_seg_t iseg = segment_id_table[i];
    const bool is_valid_seg = (iseg != invalid_marker_ph_seg);

    if (is_valid_seg) {
      constexpr int bits_to_shift = emtf_img_col_factor_log2;
      const trk_col_t col = (emtf_phi[iseg] >> bits_to_shift);
      const trk_col_t padded_col = col + col_pad;
      const bool is_valid_col = (
          (seg_valid[iseg] == 1) and \
          (seg_zones[iseg][(num_emtf_zones - 1) - curr_trk_zone] == 1) and \
          (seg_tzones[iseg][(num_emtf_zones - 1) - curr_trk_tzone] == 1) and \
          (curr_trk_qual > 0) and \
          (col_start <= padded_col) and (padded_col <= col_stop)
      );

      if (is_valid_col) {
        // Calculate abs phi difference
        const emtf_phi_t ph_patt_tmp = details::calc_rectified_diff(col_mid, col_pad);
        const emtf_phi_t ph_patt = (ph_patt_tmp << bits_to_shift) + (1u << (bits_to_shift - 1));
        emtf_assert(((ph_patt >> bits_to_shift) + col_pad) == col_mid);
        const emtf_phi_t ph_diff_tmp = details::calc_abs_diff(emtf_phi[iseg], ph_patt);
        site_ph_diff[i] = (ph_diff_tmp < invalid_marker_ph_diff) ? static_cast<dio_ph_diff_t>(ph_diff_tmp) : invalid_marker_ph_diff;
      } else {
        // Invalid segment
        site_ph_diff[i] = invalid_marker_ph_diff;
      }
    } else {
      // Invalid segment id
      site_ph_diff[i] = invalid_marker_ph_diff;
    }
  }  // end loop over segments

  // Select an 40-deg area (overlapping)
  // area 0: 0 - 165
  // area 1: 75 - 240
  // area 2: 150 - 315
  // use edges: 0, 120, 195, 315
  const int area_margin = (details::chamber_img_bw / 2);  // 45
  const trk_col_t col_start_area_0 = details::chamber_ph_init_20deg[0] + area_margin;  // 120
  const trk_col_t col_start_area_1 = details::chamber_ph_init_20deg[1] + area_margin;  // 195
  const trk_col_t col_start_area_2 = details::chamber_ph_init_20deg[2] + area_margin + area_margin;  // 315

  const bool vld_area_0 = (col_patt < col_start_area_0);
  const bool vld_area_1 = (col_patt < col_start_area_1);
  const bool vld_area_2 = (col_patt < col_start_area_2);
  const dio_ph_area_t area = vld_area_0 ? 0u : (vld_area_1 ? 1u : (vld_area_2 ? 2u : 3u));

  dio_ph_idx_t site_ph_idx;  // placeholder for result
  bool_t is_valid_seg;       // placeholder for result

  // Select min phi difference
  trkbuilding_reduce_argmin_ph_diff_op<chamber_category>(
      area,
      site_ph_diff,
      site_ph_idx,
      is_valid_seg
  );

  curr_trk_seg_site_i = static_cast<trk_seg_t>(segment_id_table[site_ph_idx]);
  curr_trk_seg_v_site_i = is_valid_seg;

  //// Debug
  //std::cout << "[DEBUG] site " << details::site_traits<Site>::value << " seg: [";
  //for (unsigned i = 0; i < N; i++) {
  //  std::cout << segment_id_table[i] << ", ";
  //}
  //std::cout << "] diff: [";
  //for (unsigned i = 0; i < N; i++) {
  //  std::cout << site_ph_diff[i] << ", ";
  //}
  //std::cout << "] out: ("<< curr_trk_seg_site_i << ", " << std::boolalpha << curr_trk_seg_v_site_i << ")" << std::endl;
}

template <typename T=void>
void trkbuilding_match_ph_op(
    const emtf_phi_t emtf_phi[model_config::n_in],
    const seg_zones_t seg_zones[model_config::n_in],
    const seg_tzones_t seg_tzones[model_config::n_in],
    const seg_valid_t seg_valid[model_config::n_in],
    const trk_qual_t& curr_trk_qual,
    const trk_patt_t& curr_trk_patt,
    const trk_col_t& curr_trk_col,
    const trk_zone_t& curr_trk_zone,
    const trk_tzone_t& curr_trk_tzone,
    trk_seg_t curr_trk_seg[num_emtf_sites],
    bool_t curr_trk_seg_v_from_ph[num_emtf_sites]
) {

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // Loop over sites manually
  trkbuilding_match_ph_site_op<m_site_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[0], curr_trk_seg_v_from_ph[0]);
  trkbuilding_match_ph_site_op<m_site_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[1], curr_trk_seg_v_from_ph[1]);
  trkbuilding_match_ph_site_op<m_site_2_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[2], curr_trk_seg_v_from_ph[2]);
  trkbuilding_match_ph_site_op<m_site_3_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[3], curr_trk_seg_v_from_ph[3]);
  trkbuilding_match_ph_site_op<m_site_4_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[4], curr_trk_seg_v_from_ph[4]);
  trkbuilding_match_ph_site_op<m_site_5_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[5], curr_trk_seg_v_from_ph[5]);
  trkbuilding_match_ph_site_op<m_site_6_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[6], curr_trk_seg_v_from_ph[6]);
  trkbuilding_match_ph_site_op<m_site_7_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[7], curr_trk_seg_v_from_ph[7]);
  trkbuilding_match_ph_site_op<m_site_8_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[8], curr_trk_seg_v_from_ph[8]);
  trkbuilding_match_ph_site_op<m_site_9_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[9], curr_trk_seg_v_from_ph[9]);
  trkbuilding_match_ph_site_op<m_site_10_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[10], curr_trk_seg_v_from_ph[10]);
  trkbuilding_match_ph_site_op<m_site_11_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg[11], curr_trk_seg_v_from_ph[11]);
}

template <typename T=void>
void trkbuilding_match_th_op(
    const emtf_theta1_t emtf_theta1[model_config::n_in],
    const emtf_theta2_t emtf_theta2[model_config::n_in],
    const trk_seg_t curr_trk_seg[num_emtf_sites],
    const bool_t curr_trk_seg_v_from_ph[num_emtf_sites],
    emtf_theta1_t& theta_median,
    emtf_theta1_t emtf_theta_best[num_emtf_sites],
    bool_t curr_trk_seg_v_from_th[num_emtf_sites]
) {

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef emtf_theta1_t emtf_theta_t;

  const int num_theta_values = 9;
  emtf_theta_t theta_values[num_theta_values];
  emtf_theta_t theta_values_s1[num_theta_values];  // special case for ME0/ME1-only tracks

#pragma HLS ARRAY_PARTITION variable=theta_values complete dim=0
#pragma HLS ARRAY_PARTITION variable=theta_values_s1 complete dim=0

  const emtf_theta_t invalid_marker_th = details::th_invalid;

  // Loop over the theta values manually
  const emtf_theta_t& theta_value_0_0 = details::choose_value_if(curr_trk_seg_v_from_ph[0], emtf_theta1[curr_trk_seg[0]], invalid_marker_th);
  const emtf_theta_t& theta_value_0_1 = details::choose_value_if(curr_trk_seg_v_from_ph[0], emtf_theta2[curr_trk_seg[0]], invalid_marker_th);
  const emtf_theta_t& theta_value_1_0 = details::choose_value_if(curr_trk_seg_v_from_ph[1], emtf_theta1[curr_trk_seg[1]], invalid_marker_th);
  const emtf_theta_t& theta_value_1_1 = details::choose_value_if(curr_trk_seg_v_from_ph[1], emtf_theta2[curr_trk_seg[1]], invalid_marker_th);
  const emtf_theta_t& theta_value_2_0 = details::choose_value_if(curr_trk_seg_v_from_ph[2], emtf_theta1[curr_trk_seg[2]], invalid_marker_th);
  const emtf_theta_t& theta_value_2_1 = details::choose_value_if(curr_trk_seg_v_from_ph[2], emtf_theta2[curr_trk_seg[2]], invalid_marker_th);
  const emtf_theta_t& theta_value_3_0 = details::choose_value_if(curr_trk_seg_v_from_ph[3], emtf_theta1[curr_trk_seg[3]], invalid_marker_th);
  const emtf_theta_t& theta_value_3_1 = details::choose_value_if(curr_trk_seg_v_from_ph[3], emtf_theta2[curr_trk_seg[3]], invalid_marker_th);
  const emtf_theta_t& theta_value_4_0 = details::choose_value_if(curr_trk_seg_v_from_ph[4], emtf_theta1[curr_trk_seg[4]], invalid_marker_th);
  const emtf_theta_t& theta_value_4_1 = details::choose_value_if(curr_trk_seg_v_from_ph[4], emtf_theta2[curr_trk_seg[4]], invalid_marker_th);
  const emtf_theta_t& theta_value_5_0 = details::choose_value_if(curr_trk_seg_v_from_ph[5], emtf_theta1[curr_trk_seg[5]], invalid_marker_th);
  const emtf_theta_t& theta_value_6_0 = details::choose_value_if(curr_trk_seg_v_from_ph[6], emtf_theta1[curr_trk_seg[6]], invalid_marker_th);
  const emtf_theta_t& theta_value_7_0 = details::choose_value_if(curr_trk_seg_v_from_ph[7], emtf_theta1[curr_trk_seg[7]], invalid_marker_th);
  const emtf_theta_t& theta_value_8_0 = details::choose_value_if(curr_trk_seg_v_from_ph[8], emtf_theta1[curr_trk_seg[8]], invalid_marker_th);
  const emtf_theta_t& theta_value_9_0 = details::choose_value_if(curr_trk_seg_v_from_ph[9], emtf_theta1[curr_trk_seg[9]], invalid_marker_th);
  const emtf_theta_t& theta_value_10_0 = details::choose_value_if(curr_trk_seg_v_from_ph[10], emtf_theta1[curr_trk_seg[10]], invalid_marker_th);
  const emtf_theta_t& theta_value_11_0 = details::choose_value_if(curr_trk_seg_v_from_ph[11], emtf_theta1[curr_trk_seg[11]], invalid_marker_th);

  theta_values[0] = theta_value_2_0;  // ME2 theta 1
  theta_values[1] = theta_value_3_0;  // ME3 theta 1
  theta_values[2] = theta_value_4_0;  // ME4 theta 1
  theta_values[3] = theta_value_2_1;  // ME2 theta 2
  theta_values[4] = theta_value_3_1;  // ME3 theta 2
  theta_values[5] = theta_value_4_1;  // ME4 theta 2
  theta_values[6] = (theta_value_6_0 != invalid_marker_th) ? theta_value_6_0 : theta_value_10_0;  // RE2 or GE2/1 theta
  theta_values[7] = theta_value_7_0;  // RE3 theta
  theta_values[8] = theta_value_8_0;  // RE4 theta

  theta_values_s1[0] = theta_value_1_0;  // ME1/2 theta 1
  theta_values_s1[1] = theta_value_0_0;  // ME1/1 theta 1
  theta_values_s1[2] = invalid_marker_th;
  theta_values_s1[3] = theta_value_1_1;  // ME1/2 theta 2
  theta_values_s1[4] = theta_value_0_1;  // ME1/1 theta 2
  theta_values_s1[5] = invalid_marker_th;
  theta_values_s1[6] = theta_value_5_0;  // RE1/2 theta
  theta_values_s1[7] = theta_value_9_0;  // GE1/1 theta
  theta_values_s1[8] = theta_value_11_0; // ME0 theta

  // Find theta_median
  emtf_theta_t theta_median_tmp_0 = invalid_marker_th;
  emtf_theta_t theta_median_tmp_1 = invalid_marker_th;
  trkbuilding_reduce_median_theta_op(theta_values, theta_median_tmp_0);
  trkbuilding_reduce_median_theta_op(theta_values_s1, theta_median_tmp_1);

  theta_median = (theta_median_tmp_0 != invalid_marker_th) ? theta_median_tmp_0 : theta_median_tmp_1;
  //emtf_assert(theta_median != invalid_marker_th);

  //std::cout << "[DEBUG] theta_values: [";
  //for (unsigned i = 0; i < num_theta_values; i++) {
  //  std::cout << theta_values[i] << ", ";
  //}
  //std::cout << "] theta_values_s1: [";
  //for (unsigned i = 0; i < num_theta_values; i++) {
  //  std::cout << theta_values_s1[i] << ", ";
  //}
  //std::cout << "] theta_median: " << theta_median << std::endl;

  // Find the most compatible theta values, also apply theta window cut
  trkbuilding_select_theta_op(theta_value_0_0, theta_value_0_1, theta_median, emtf_theta_best[0], curr_trk_seg_v_from_th[0]);
  trkbuilding_select_theta_op(theta_value_1_0, theta_value_1_1, theta_median, emtf_theta_best[1], curr_trk_seg_v_from_th[1]);
  trkbuilding_select_theta_op(theta_value_2_0, theta_value_2_1, theta_median, emtf_theta_best[2], curr_trk_seg_v_from_th[2]);
  trkbuilding_select_theta_op(theta_value_3_0, theta_value_3_1, theta_median, emtf_theta_best[3], curr_trk_seg_v_from_th[3]);
  trkbuilding_select_theta_op(theta_value_4_0, theta_value_4_1, theta_median, emtf_theta_best[4], curr_trk_seg_v_from_th[4]);
  trkbuilding_select_theta_op(theta_value_5_0, theta_median, emtf_theta_best[5], curr_trk_seg_v_from_th[5]);
  trkbuilding_select_theta_op(theta_value_6_0, theta_median, emtf_theta_best[6], curr_trk_seg_v_from_th[6]);
  trkbuilding_select_theta_op(theta_value_7_0, theta_median, emtf_theta_best[7], curr_trk_seg_v_from_th[7]);
  trkbuilding_select_theta_op(theta_value_8_0, theta_median, emtf_theta_best[8], curr_trk_seg_v_from_th[8]);
  trkbuilding_select_theta_op(theta_value_9_0, theta_median, emtf_theta_best[9], curr_trk_seg_v_from_th[9]);
  trkbuilding_select_theta_op(theta_value_10_0, theta_median, emtf_theta_best[10], curr_trk_seg_v_from_th[10]);
  trkbuilding_select_theta_op(theta_value_11_0, theta_median, emtf_theta_best[11], curr_trk_seg_v_from_th[11]);
}

template <typename T=void>
void trkbuilding_extract_features_op(
    const emtf_phi_t emtf_phi[model_config::n_in],
    const emtf_bend_t emtf_bend[model_config::n_in],
    const emtf_qual1_t emtf_qual1[model_config::n_in],
    const emtf_qual2_t emtf_qual2[model_config::n_in],
    const emtf_time_t emtf_time[model_config::n_in],
    const emtf_phi_t& phi_median,
    const emtf_theta1_t& theta_median,
    const emtf_theta1_t emtf_theta_best[num_emtf_sites],
    const trk_qual_t& curr_trk_qual,
    const trk_seg_t curr_trk_seg[num_emtf_sites],
    const trk_seg_v_t& curr_trk_seg_v,
    trk_feat_t curr_trk_feat[num_emtf_features]
) {

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

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
  // 4 additional features are: phi_median, theta_median, trk_qual, trk_bx

  // Set img center phi
  emtf_phi_t phi_img_center;
  const trk_col_t col_img_center = num_emtf_img_cols / 2;
  trkbuilding_select_phi_median_op(col_img_center, phi_img_center);

  int k = 0;

  // emtf_phi
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[0], details::calc_signed_diff(emtf_phi[curr_trk_seg[0]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[1], details::calc_signed_diff(emtf_phi[curr_trk_seg[1]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[2], details::calc_signed_diff(emtf_phi[curr_trk_seg[2]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[3], details::calc_signed_diff(emtf_phi[curr_trk_seg[3]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[4], details::calc_signed_diff(emtf_phi[curr_trk_seg[4]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[5], details::calc_signed_diff(emtf_phi[curr_trk_seg[5]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[6], details::calc_signed_diff(emtf_phi[curr_trk_seg[6]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[7], details::calc_signed_diff(emtf_phi[curr_trk_seg[7]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[8], details::calc_signed_diff(emtf_phi[curr_trk_seg[8]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[9], details::calc_signed_diff(emtf_phi[curr_trk_seg[9]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[10], details::calc_signed_diff(emtf_phi[curr_trk_seg[10]], phi_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[11], details::calc_signed_diff(emtf_phi[curr_trk_seg[11]], phi_median));

  // emtf_theta
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[0], details::calc_signed_diff(emtf_theta_best[0], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[1], details::calc_signed_diff(emtf_theta_best[1], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[2], details::calc_signed_diff(emtf_theta_best[2], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[3], details::calc_signed_diff(emtf_theta_best[3], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[4], details::calc_signed_diff(emtf_theta_best[4], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[5], details::calc_signed_diff(emtf_theta_best[5], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[6], details::calc_signed_diff(emtf_theta_best[6], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[7], details::calc_signed_diff(emtf_theta_best[7], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[8], details::calc_signed_diff(emtf_theta_best[8], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[9], details::calc_signed_diff(emtf_theta_best[9], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[10], details::calc_signed_diff(emtf_theta_best[10], theta_median));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[11], details::calc_signed_diff(emtf_theta_best[11], theta_median));

  // emtf_bend
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[0], emtf_bend[curr_trk_seg[0]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[1], emtf_bend[curr_trk_seg[1]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[2], emtf_bend[curr_trk_seg[2]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[3], emtf_bend[curr_trk_seg[3]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[4], emtf_bend[curr_trk_seg[4]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[11], emtf_bend[curr_trk_seg[11]]);

  // emtf_qual
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[0], emtf_qual1[curr_trk_seg[0]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[1], emtf_qual1[curr_trk_seg[1]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[2], emtf_qual1[curr_trk_seg[2]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[3], emtf_qual1[curr_trk_seg[3]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[4], emtf_qual1[curr_trk_seg[4]]);
  curr_trk_feat[k++] = details::take_value_if(curr_trk_seg_v[11], emtf_qual1[curr_trk_seg[11]]);

  // additional features
  curr_trk_feat[k++] = details::take_value_if(curr_trk_qual > 0, details::calc_signed_diff(phi_median, phi_img_center));
  curr_trk_feat[k++] = details::take_value_if(curr_trk_qual > 0, theta_median);
  curr_trk_feat[k++] = curr_trk_qual;
  curr_trk_feat[k++] = 0;  // unused
  emtf_assert(k == num_emtf_features);
}

// _____________________________________________________________________________
// Track building op

template <typename Zone, typename Timezone>
void trkbuilding_op(
    const emtf_phi_t        emtf_phi       [model_config::n_in],
    const emtf_bend_t       emtf_bend      [model_config::n_in],
    const emtf_theta1_t     emtf_theta1    [model_config::n_in],
    const emtf_theta2_t     emtf_theta2    [model_config::n_in],
    const emtf_qual1_t      emtf_qual1     [model_config::n_in],
    const emtf_qual2_t      emtf_qual2     [model_config::n_in],
    const emtf_time_t       emtf_time      [model_config::n_in],
    const seg_zones_t       seg_zones      [model_config::n_in],
    const seg_tzones_t      seg_tzones     [model_config::n_in],
    const seg_fr_t          seg_fr         [model_config::n_in],
    const seg_dl_t          seg_dl         [model_config::n_in],
    const seg_bx_t          seg_bx         [model_config::n_in],
    const seg_valid_t       seg_valid      [model_config::n_in],
    const trkbuilding_in_t& curr_trk_in    ,
    trk_seg_t               curr_trk_seg   [num_emtf_sites],
    trk_seg_v_t&            curr_trk_seg_v ,
    trk_feat_t              curr_trk_feat  [num_emtf_features],
    trk_valid_t&            curr_trk_valid
) {

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef emtf_theta1_t emtf_theta_t;

  // Unpack from curr_trk_in (a.k.a. trkbuilding_in[itrk])
  constexpr int bits_lo_0 = 0;
  constexpr int bits_lo_1 = bits_lo_0 + trk_qual_t::width;
  constexpr int bits_lo_2 = bits_lo_1 + trk_patt_t::width;
  constexpr int bits_lo_3 = bits_lo_2 + trk_col_t::width;
  constexpr int bits_lo_4 = bits_lo_3 + trk_zone_t::width;

  const trk_qual_t  curr_trk_qual  = curr_trk_in.range(bits_lo_1 - 1, bits_lo_0);
  const trk_patt_t  curr_trk_patt  = curr_trk_in.range(bits_lo_2 - 1, bits_lo_1);
  const trk_col_t   curr_trk_col   = curr_trk_in.range(bits_lo_3 - 1, bits_lo_2);
  const trk_zone_t  curr_trk_zone  = curr_trk_in.range(bits_lo_4 - 1, bits_lo_3);
  const trk_tzone_t curr_trk_tzone = details::timezone_traits<Timezone>::value;  // dummy

  //std::cout << "[DEBUG] " << curr_trk_qual << " " << curr_trk_patt << " " << curr_trk_col << " " << curr_trk_zone << " " << curr_trk_tzone << std::endl;

  // Intermediate arrays
  bool_t curr_trk_seg_v_from_ph[num_emtf_sites];
  bool_t curr_trk_seg_v_from_th[num_emtf_sites];
  emtf_theta_t emtf_theta_best[num_emtf_sites];

#pragma HLS ARRAY_PARTITION variable=curr_trk_seg_v_from_ph complete dim=0
#pragma HLS ARRAY_PARTITION variable=curr_trk_seg_v_from_th complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_theta_best complete dim=0

  emtf_phi_t phi_median;      // placeholder for result
  emtf_theta_t theta_median;  // placeholder for result

  // Find phi_median, select best segment indices (closest to phi_median)
  trkbuilding_select_phi_median_op(curr_trk_col, phi_median);
  trkbuilding_match_ph_op(emtf_phi, seg_zones, seg_tzones, seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone, curr_trk_seg, curr_trk_seg_v_from_ph);

  // Find theta_median, select best theta values (closest to theta_median)
  trkbuilding_match_th_op(emtf_theta1, emtf_theta2, curr_trk_seg, curr_trk_seg_v_from_ph, theta_median, emtf_theta_best, curr_trk_seg_v_from_th);

  // Set segment valid flag and track valid flag
  details::pack_boolean_values(curr_trk_seg_v_from_th, curr_trk_seg_v);
  curr_trk_valid = (bool) curr_trk_seg_v;  // logical OR reduced

  // Extract features
  trkbuilding_extract_features_op(emtf_phi, emtf_bend, emtf_qual1, emtf_qual2, emtf_time, phi_median, theta_median, emtf_theta_best, curr_trk_qual, curr_trk_seg, curr_trk_seg_v, curr_trk_feat);
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void trkbuilding_layer(
    const emtf_phi_t        emtf_phi       [model_config::n_in],
    const emtf_bend_t       emtf_bend      [model_config::n_in],
    const emtf_theta1_t     emtf_theta1    [model_config::n_in],
    const emtf_theta2_t     emtf_theta2    [model_config::n_in],
    const emtf_qual1_t      emtf_qual1     [model_config::n_in],
    const emtf_qual2_t      emtf_qual2     [model_config::n_in],
    const emtf_time_t       emtf_time      [model_config::n_in],
    const seg_zones_t       seg_zones      [model_config::n_in],
    const seg_tzones_t      seg_tzones     [model_config::n_in],
    const seg_fr_t          seg_fr         [model_config::n_in],
    const seg_dl_t          seg_dl         [model_config::n_in],
    const seg_bx_t          seg_bx         [model_config::n_in],
    const seg_valid_t       seg_valid      [model_config::n_in],
    const trkbuilding_in_t  trkbuilding_in [trkbuilding_config::n_in],
    trk_seg_t               trk_seg        [trkbuilding_config::n_out * num_emtf_sites],
    trk_seg_v_t             trk_seg_v      [trkbuilding_config::n_out],
    trk_feat_t              trk_feat       [trkbuilding_config::n_out * num_emtf_features],
    trk_valid_t             trk_valid      [trkbuilding_config::n_out]
) {

#pragma HLS PIPELINE II=trkbuilding_config::layer_target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS LATENCY max=trkbuilding_config::target_lat

  // Check assumptions
  static_assert(trkbuilding_config::n_in == num_emtf_tracks, "trkbuilding_config::n_in check failed");
  static_assert(trkbuilding_config::n_out == num_emtf_tracks, "trkbuilding_config::n_out check failed");
  static_assert(num_emtf_sites == 12, "num_emtf_sites must be 12");
  static_assert(num_emtf_features == 40, "num_emtf_features must be 40");
  static_assert(num_emtf_img_areas == 3, "num_emtf_areas must be 3");

  typedef m_timezone_0_tag Timezone;  // default timezone

  // Loop over tracks
  LOOP_TRK: for (unsigned itrk = 0; itrk < trkbuilding_config::n_in; itrk++) {

#pragma HLS UNROLL

    auto curr_trk_seg = &(trk_seg[itrk * num_emtf_sites]);
    auto curr_trk_feat = &(trk_feat[itrk * num_emtf_features]);

    trkbuilding_op<Zone, Timezone>(
        emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, emtf_qual2,
        emtf_time, seg_zones, seg_tzones, seg_fr, seg_dl, seg_bx,
        seg_valid, trkbuilding_in[itrk], curr_trk_seg, trk_seg_v[itrk], curr_trk_feat, trk_valid[itrk]
    );
  }  // end loop over tracks
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_TRKBUILDING_H__ not defined
