#ifndef __EMTF_HLSLIB_POOLING_H__
#define __EMTF_HLSLIB_POOLING_H__

namespace emtf {

// Function to apply activation
template <typename Category, typename T_IN, typename T_OUT>
void pooling_activate_op(const T_IN& in0, T_OUT& out) {
  static_assert(is_same<T_IN, dio_row_accum_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, trk_qual_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=pooling_config::activation_target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const unsigned int N = (1u << T_IN::width);

#if !defined(__SYNTHESIS__)
  static bool initialized = false;
  static int lookup_table[N];
#else
  bool initialized = false;
  int lookup_table[N];
#endif

  if (!initialized) {
    details::init_table_op<N>(lookup_table, details::get_pattern_activation_op<Category>());
    initialized = true;
  }

  // Lookup
  emtf_assert(in0 < N);
  out = static_cast<T_OUT>(lookup_table[in0]);
}

// Function to find max activation, keeping pattern number
template <typename T_IN, typename T_OUT>
void pooling_reduce_argmax_op(const T_IN& in0, T_OUT& out) {
  static_assert(is_same<T_IN, make_repeat<trk_qual_t, num_emtf_patterns>::type>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, make_concat<trk_patt_t, trk_qual_t>::type>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=pooling_config::reduction_target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  typedef trk_patt_t idx_t;  // encodes 0..6
  typedef trk_qual_t data_t;
  typedef details::argsort_pair<idx_t, data_t> pair_t;

  // Binary tree structure (N must be an even number)
  const unsigned int N = details::ceil_mul4<num_emtf_patterns>::value;  // round up to 8
  const unsigned int num_nodes_in = N;
  const unsigned int num_nodes_io = N - 1;
  const unsigned int num_nodes = num_nodes_in + num_nodes_io;

  pair_t nodes[num_nodes];

#pragma HLS DATA_PACK variable=nodes

#pragma HLS ARRAY_PARTITION variable=nodes complete dim=0

  // Fetch input
  // Note that in0[0] is inserted twice to round up to 8
  {
    unsigned int node_index = (num_nodes - 1);
    nodes[node_index--] = pair_t(idx_t(6), in0.range(((6 + 1) * data_t::width) - 1, (6 * data_t::width)));
    nodes[node_index--] = pair_t(idx_t(5), in0.range(((5 + 1) * data_t::width) - 1, (5 * data_t::width)));
    nodes[node_index--] = pair_t(idx_t(4), in0.range(((4 + 1) * data_t::width) - 1, (4 * data_t::width)));
    nodes[node_index--] = pair_t(idx_t(3), in0.range(((3 + 1) * data_t::width) - 1, (3 * data_t::width)));
    nodes[node_index--] = pair_t(idx_t(2), in0.range(((2 + 1) * data_t::width) - 1, (2 * data_t::width)));
    nodes[node_index--] = pair_t(idx_t(1), in0.range(((1 + 1) * data_t::width) - 1, (1 * data_t::width)));
    nodes[node_index--] = pair_t(idx_t(0), in0.range(((0 + 1) * data_t::width) - 1, (0 * data_t::width)));
    nodes[node_index--] = pair_t(idx_t(0), in0.range(((0 + 1) * data_t::width) - 1, (0 * data_t::width)));
  }

  // Binary-tree reduce
  LOOP_NOD: for (unsigned i = 0; i < num_nodes_io; i++) {

#pragma HLS UNROLL

    const unsigned int node_index = (num_nodes_io - 1) - i;
    const unsigned int child_l_index = (2 * node_index) + 1;
    const unsigned int child_r_index = (2 * node_index) + 2;
    emtf_assert((child_l_index < num_nodes) && (child_r_index < num_nodes));
    nodes[node_index] = (nodes[child_l_index] >= nodes[child_r_index]) ? nodes[child_l_index] : nodes[child_r_index];
  }

  // Output
  out = (nodes[0].first, nodes[0].second);
}

// _____________________________________________________________________________
// Apply the patterns at a particular column, and then apply activation.
// The column-wise operations are fused together.

template <typename Zone>
void pooling_fused_col_1_op(
    const typename details::select_pattern_fused_col_patch_type<Zone, 0>::type& patch_row_0,
    const typename details::select_pattern_fused_col_patch_type<Zone, 1>::type& patch_row_1,
    const typename details::select_pattern_fused_col_patch_type<Zone, 2>::type& patch_row_2,
    const typename details::select_pattern_fused_col_patch_type<Zone, 3>::type& patch_row_3,
    const typename details::select_pattern_fused_col_patch_type<Zone, 4>::type& patch_row_4,
    const typename details::select_pattern_fused_col_patch_type<Zone, 5>::type& patch_row_5,
    const typename details::select_pattern_fused_col_patch_type<Zone, 6>::type& patch_row_6,
    const typename details::select_pattern_fused_col_patch_type<Zone, 7>::type& patch_row_7,
    typename details::select_pattern_packed_activation_type<Zone>::type activations[pooling_config::fusion_factor]
) {

#pragma HLS PIPELINE II=pooling_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  typedef typename details::select_pattern_preactivation_type<Zone>::type preactivation_t;
  typedef typename details::select_pattern_activation_type<Zone>::type activation_t;
  const unsigned int fusion_factor = pooling_config::fusion_factor;

  int pattern_col_start_table[num_emtf_patterns * num_emtf_img_rows];
  details::init_2d_table_op<num_emtf_patterns, num_emtf_img_rows>(pattern_col_start_table, details::get_pattern_col_start_op<Zone>());

  int pattern_col_stop_table[num_emtf_patterns * num_emtf_img_rows];
  details::init_2d_table_op<num_emtf_patterns, num_emtf_img_rows>(pattern_col_stop_table, details::get_pattern_col_stop_op<Zone>());

  // Loop over patterns
  LOOP_PATT_1: for (unsigned patt = 0; patt < num_emtf_patterns; patt++) {

#pragma HLS UNROLL

    // Loop over columns
    LOOP_COL_1: for (unsigned col = 0; col < fusion_factor; col++) {

#pragma HLS UNROLL

      const unsigned int table_index = (patt * num_emtf_img_rows);

      // Preactivation at (col, patt)
      preactivation_t preactivation = 0;  // init as zero
      preactivation[0] = (bool) patch_row_0.range(col + pattern_col_stop_table[table_index + 0], col + pattern_col_start_table[table_index + 0]);
      preactivation[1] = (bool) patch_row_1.range(col + pattern_col_stop_table[table_index + 1], col + pattern_col_start_table[table_index + 1]);
      preactivation[2] = (bool) patch_row_2.range(col + pattern_col_stop_table[table_index + 2], col + pattern_col_start_table[table_index + 2]);
      preactivation[3] = (bool) patch_row_3.range(col + pattern_col_stop_table[table_index + 3], col + pattern_col_start_table[table_index + 3]);
      preactivation[4] = (bool) patch_row_4.range(col + pattern_col_stop_table[table_index + 4], col + pattern_col_start_table[table_index + 4]);
      preactivation[5] = (bool) patch_row_5.range(col + pattern_col_stop_table[table_index + 5], col + pattern_col_start_table[table_index + 5]);
      preactivation[6] = (bool) patch_row_6.range(col + pattern_col_stop_table[table_index + 6], col + pattern_col_start_table[table_index + 6]);
      preactivation[7] = (bool) patch_row_7.range(col + pattern_col_stop_table[table_index + 7], col + pattern_col_start_table[table_index + 7]);

      // Activation at (col, patt)
      activation_t activation = 0;  // init as zero
      pooling_activate_op<Zone>(preactivation, activation);

      // Output
      activations[col].range(((patt + 1) * activation_t::width) - 1, (patt * activation_t::width)) = activation;
    }  // end loop over columns
  }  // end loop over patterns
}

template <typename Zone>
void pooling_fused_col_2_op(
    const typename details::select_pattern_packed_activation_type<Zone>::type activations[pooling_config::fusion_factor],
    pooling_out_t pooling_out_fused_col_j[pooling_config::fusion_factor]
) {

#pragma HLS PIPELINE II=pooling_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const unsigned int fusion_factor = pooling_config::fusion_factor;

  // Loop over columns
  LOOP_COL_2: for (unsigned col = 0; col < fusion_factor; col++) {

#pragma HLS UNROLL

    // Find max activation
    pooling_reduce_argmax_op(activations[col], pooling_out_fused_col_j[col]);
  }  // end loop over columns
}

template <typename Zone>
void pooling_fused_col_op(
    const typename details::select_pattern_fused_col_patch_type<Zone, 0>::type& patch_row_0,
    const typename details::select_pattern_fused_col_patch_type<Zone, 1>::type& patch_row_1,
    const typename details::select_pattern_fused_col_patch_type<Zone, 2>::type& patch_row_2,
    const typename details::select_pattern_fused_col_patch_type<Zone, 3>::type& patch_row_3,
    const typename details::select_pattern_fused_col_patch_type<Zone, 4>::type& patch_row_4,
    const typename details::select_pattern_fused_col_patch_type<Zone, 5>::type& patch_row_5,
    const typename details::select_pattern_fused_col_patch_type<Zone, 6>::type& patch_row_6,
    const typename details::select_pattern_fused_col_patch_type<Zone, 7>::type& patch_row_7,
    pooling_out_t pooling_out_fused_col_j[pooling_config::fusion_factor]
) {

#pragma HLS PIPELINE II=pooling_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef typename details::select_pattern_packed_activation_type<Zone>::type packed_activation_t;
  const unsigned int fusion_factor = pooling_config::fusion_factor;

  packed_activation_t activations[fusion_factor];

#pragma HLS ARRAY_PARTITION variable=activations complete dim=0

  pooling_fused_col_1_op<Zone>(
      patch_row_0,
      patch_row_1,
      patch_row_2,
      patch_row_3,
      patch_row_4,
      patch_row_5,
      patch_row_6,
      patch_row_7,
      activations
  );

  pooling_fused_col_2_op<Zone>(activations, pooling_out_fused_col_j);
}

// _____________________________________________________________________________
// Pooling op

template <typename Zone>
void pooling_op(
    const pooling_in_t pooling_in[pooling_config::n_in],
    pooling_out_t pooling_out[pooling_config::n_out]
) {

#pragma HLS PIPELINE II=pooling_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef typename details::select_pattern_col_padding_type<Zone, 0>::type padding_row_0_t;
  typedef typename details::select_pattern_col_padding_type<Zone, 1>::type padding_row_1_t;
  typedef typename details::select_pattern_col_padding_type<Zone, 2>::type padding_row_2_t;
  typedef typename details::select_pattern_col_padding_type<Zone, 3>::type padding_row_3_t;
  typedef typename details::select_pattern_col_padding_type<Zone, 4>::type padding_row_4_t;
  typedef typename details::select_pattern_col_padding_type<Zone, 5>::type padding_row_5_t;
  typedef typename details::select_pattern_col_padding_type<Zone, 6>::type padding_row_6_t;
  typedef typename details::select_pattern_col_padding_type<Zone, 7>::type padding_row_7_t;

  typedef typename details::select_pattern_col_padded_type<Zone, 0>::type padded_row_0_t;
  typedef typename details::select_pattern_col_padded_type<Zone, 1>::type padded_row_1_t;
  typedef typename details::select_pattern_col_padded_type<Zone, 2>::type padded_row_2_t;
  typedef typename details::select_pattern_col_padded_type<Zone, 3>::type padded_row_3_t;
  typedef typename details::select_pattern_col_padded_type<Zone, 4>::type padded_row_4_t;
  typedef typename details::select_pattern_col_padded_type<Zone, 5>::type padded_row_5_t;
  typedef typename details::select_pattern_col_padded_type<Zone, 6>::type padded_row_6_t;
  typedef typename details::select_pattern_col_padded_type<Zone, 7>::type padded_row_7_t;

  // Add padding
  const padded_row_0_t padded_row_0 = (padding_row_0_t(0), pooling_in[0], padding_row_0_t(0));
  const padded_row_1_t padded_row_1 = (padding_row_1_t(0), pooling_in[1], padding_row_1_t(0));
  const padded_row_2_t padded_row_2 = (padding_row_2_t(0), pooling_in[2], padding_row_2_t(0));
  const padded_row_3_t padded_row_3 = (padding_row_3_t(0), pooling_in[3], padding_row_3_t(0));
  const padded_row_4_t padded_row_4 = (padding_row_4_t(0), pooling_in[4], padding_row_4_t(0));
  const padded_row_5_t padded_row_5 = (padding_row_5_t(0), pooling_in[5], padding_row_5_t(0));
  const padded_row_6_t padded_row_6 = (padding_row_6_t(0), pooling_in[6], padding_row_6_t(0));
  const padded_row_7_t padded_row_7 = (padding_row_7_t(0), pooling_in[7], padding_row_7_t(0));

  const unsigned int fusion_factor = pooling_config::fusion_factor;

  // Loop over columns with certain step size
  LOOP_COL_0: for (unsigned col = 0; col < num_emtf_img_cols; col += fusion_factor) {

#pragma HLS UNROLL

    pooling_fused_col_op<Zone>(
        padded_row_0.range(col + (fusion_factor - 1) + (padding_row_0_t::width * 2), col),
        padded_row_1.range(col + (fusion_factor - 1) + (padding_row_1_t::width * 2), col),
        padded_row_2.range(col + (fusion_factor - 1) + (padding_row_2_t::width * 2), col),
        padded_row_3.range(col + (fusion_factor - 1) + (padding_row_3_t::width * 2), col),
        padded_row_4.range(col + (fusion_factor - 1) + (padding_row_4_t::width * 2), col),
        padded_row_5.range(col + (fusion_factor - 1) + (padding_row_5_t::width * 2), col),
        padded_row_6.range(col + (fusion_factor - 1) + (padding_row_6_t::width * 2), col),
        padded_row_7.range(col + (fusion_factor - 1) + (padding_row_7_t::width * 2), col),
        &(pooling_out[col])
    );
  }  // end loop over columns
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void pooling_layer(
    const pooling_in_t pooling_in[pooling_config::n_in],
    pooling_out_t pooling_out[pooling_config::n_out]
) {

#pragma HLS PIPELINE II=pooling_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS LATENCY max=pooling_config::target_lat

  // Check assumptions
  static_assert(pooling_config::n_in == num_emtf_img_rows, "pooling_config::n_in check failed");
  static_assert(pooling_config::n_out == num_emtf_img_cols, "pooling_config::n_out check failed");
  static_assert(num_emtf_img_rows == 8, "num_emtf_img_rows must be 8");
  static_assert(num_emtf_img_cols == 288, "num_emtf_img_cols must be 288");
  static_assert(num_emtf_patterns == 7, "num_emtf_patterns must be 7");

  pooling_op<Zone>(pooling_in, pooling_out);
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_POOLING_H__ not defined
