#ifndef __EMTF_HLSLIB_ZONEMERGING_H__
#define __EMTF_HLSLIB_ZONEMERGING_H__

// Function hierarchy
//
// zonemerging_layer
// +-- zonemerging_op (INLINE)
//     |-- zonemerging_preprocess_op
//     +-- zonemerging_argmax_op

// EMTF HLS
#include "sort_kernels.h"

namespace emtf {

template <typename T_IN, typename T_OUT>
void zonemerging_preprocess_op(
    const T_IN in0[zonemerging_config::n_in],
    const T_IN in1[zonemerging_config::n_in],
    const T_IN in2[zonemerging_config::n_in],
    T_OUT out[zonemerging_config::n_stage_0]
) {
  static_assert(is_same<T_IN, zonemerging_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonemerging_out_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonemerging_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const unsigned int n_in = zonemerging_config::n_in;
  const unsigned int n_stage_0 = zonemerging_config::n_stage_0;

  // Loop over input
  LOOP_PREPROC: for (unsigned i = 0; i < n_stage_0; i++) {

#pragma HLS UNROLL

    T_IN x_i = 0;

    if ((i / n_in) == 0) {
      x_i = in0[(i % n_in)];
    } else if ((i / n_in) == 1) {
      x_i = in1[(i % n_in)];
    } else if ((i / n_in) == 2) {
      x_i = in2[(i % n_in)];
    }

    // Output (incl zone number)
    const trk_zone_t zone_i = (i / n_in);
    out[i] = (zone_i, x_i);
  }  // end loop over input
}

// _____________________________________________________________________________
template <typename T_IN, typename T_OUT>
void zonemerging_argmax_op(
    const T_IN in0[zonemerging_config::n_stage_0],
    T_OUT out[zonemerging_config::n_out]
) {
  static_assert(is_same<T_IN, zonemerging_out_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonemerging_out_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonemerging_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const unsigned int N = zonemerging_config::n_stage_0;
  typedef trk_qual_t data_t;
  typedef ap_uint<T_IN::width - data_t::width> arg_t;
  typedef details::argsort_pair<arg_t, data_t> pair_t;

  constexpr int bits_lo_0 = 0;
  constexpr int bits_lo_1 = data_t::width;
  constexpr int bits_lo_2 = T_IN::width;

  // Octal tree structure (N must be an even number)
  const unsigned int num_nodes = (N * 2) - 4;

  pair_t octal_tree[num_nodes];

#pragma HLS ARRAY_PARTITION variable=octal_tree complete dim=0

#pragma HLS DATA_PACK variable=octal_tree

  // For N = 12, the octal tree is not balanced (as N is not a power of 8), we need to alter
  // the ordering of the nodes. By default:
  //     0 1 2 3 -> 4 5 6 7 8 9 10 11 -> 12 13 14 15 16 17 18 19 X X X X X X X X
  // In order to preserve order:
  //     0 1 2 3 -> 4 5 6 7 16 17 18 19 -> 8 9 10 11 12 13 14 15 X X X X X X X X
  // Need to rotate by 8 - (12 - 8) = 4

  // Fetch input
  LOOP_ARGMAX_1: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const unsigned int node_index = (N - 4) + ((i + 4) % N);  // N-4 .. (N*2)-5 with rotation
    emtf_assert(node_index < num_nodes);

    // Make pairs
    const data_t data = in0[i].range(bits_lo_1 - 1, bits_lo_0);
    const arg_t arg = in0[i].range(bits_lo_2 - 1, bits_lo_1);
    octal_tree[node_index] = pair_t(arg, data);
  }  // end fetch input loop

  // Tree reduce
  LOOP_ARGMAX_2: for (int i = (N - 4) - 1; i >= 0; i -= 4) {

#pragma HLS UNROLL

    const unsigned int node_index = i - 3;  // 0 .. N-4 with step size 4 in reverse order
    const unsigned int child_index = (2 * node_index) + 4;  // step size 4
    emtf_assert(node_index < num_nodes);
    emtf_assert(((child_index + 0) < num_nodes) and ((child_index + 7) < num_nodes));

    // Merge 8 -> 4
    details::merge_eight_op(
        octal_tree[child_index + 0], octal_tree[child_index + 1], octal_tree[child_index + 2], octal_tree[child_index + 3],
        octal_tree[child_index + 4], octal_tree[child_index + 5], octal_tree[child_index + 6], octal_tree[child_index + 7],
        octal_tree[node_index + 0], octal_tree[node_index + 1], octal_tree[node_index + 2], octal_tree[node_index + 3]
    );

    // Sanity check
#ifndef __SYNTHESIS__
    const pair_t& r0 = octal_tree[node_index + 0];
    const pair_t& r1 = octal_tree[node_index + 1];
    const pair_t& r2 = octal_tree[node_index + 2];
    const pair_t& r3 = octal_tree[node_index + 3];
    pair_t r4, r5, r6, r7;
    details::cpp_merge_eight_op(
        octal_tree[child_index + 0], octal_tree[child_index + 1], octal_tree[child_index + 2], octal_tree[child_index + 3],
        octal_tree[child_index + 4], octal_tree[child_index + 5], octal_tree[child_index + 6], octal_tree[child_index + 7],
        r4, r5, r6, r7
    );
    auto cmp = [](const pair_t& lhs, const pair_t& rhs) -> bool {
      return lhs.second == rhs.second;
    };
    //auto str = [](std::stringstream& ss, const pair_t& a) -> std::stringstream& {
    //  ss << "(" << a.first << "," << a.second << ") ";
    //  return ss;
    //};
    //std::stringstream ss;
    //str(ss, r0);
    //str(ss, r1);
    //str(ss, r2);
    //str(ss, r3);
    //std::cout << "Got:\n" << ss.str() << std::endl;
    //ss.str("");
    //str(ss, r4);
    //str(ss, r5);
    //str(ss, r6);
    //str(ss, r7);
    //std::cout << "Expected:\n" << ss.str() << std::endl;
    emtf_assert(cmp(r0, r4) and cmp(r1, r5) and cmp(r2, r6) and cmp(r3, r7));
#endif  // __SYNTHESIS__ not defined

  }  // end tree reduce loop

  // Output
  out[0] = (octal_tree[0].first, octal_tree[0].second);
  out[1] = (octal_tree[1].first, octal_tree[1].second);
  out[2] = (octal_tree[2].first, octal_tree[2].second);
  out[3] = (octal_tree[3].first, octal_tree[3].second);
}

// _____________________________________________________________________________
// Zone merging op

template <typename Zone>
void zonemerging_op(
    const zonemerging_in_t zonemerging_in_0[zonemerging_config::n_in],
    const zonemerging_in_t zonemerging_in_1[zonemerging_config::n_in],
    const zonemerging_in_t zonemerging_in_2[zonemerging_config::n_in],
    zonemerging_out_t zonemerging_out[zonemerging_config::n_out]
) {

#pragma HLS PIPELINE II=zonemerging_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const unsigned int n_stage_0 = zonemerging_config::n_stage_0;

  // Intermediate arrays
  zonemerging_out_t stage_0_out[n_stage_0];

#pragma HLS ARRAY_PARTITION variable=stage_0_out complete dim=0

  zonemerging_preprocess_op(zonemerging_in_0, zonemerging_in_1, zonemerging_in_2, stage_0_out);

  zonemerging_argmax_op(stage_0_out, zonemerging_out);
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void zonemerging_layer(
    const zonemerging_in_t zonemerging_in_0[zonemerging_config::n_in],
    const zonemerging_in_t zonemerging_in_1[zonemerging_config::n_in],
    const zonemerging_in_t zonemerging_in_2[zonemerging_config::n_in],
    zonemerging_out_t zonemerging_out[zonemerging_config::n_out]
) {

#pragma HLS PIPELINE II=zonemerging_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

  // Check assumptions
  static_assert(zonemerging_config::n_in == num_emtf_tracks, "zonemerging_config::n_in check failed");
  static_assert(zonemerging_config::n_out == num_emtf_tracks, "zonemerging_config::n_out check failed");

  zonemerging_op<Zone>(zonemerging_in_0, zonemerging_in_1, zonemerging_in_2, zonemerging_out);
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_ZONEMERGING_H__ not defined
