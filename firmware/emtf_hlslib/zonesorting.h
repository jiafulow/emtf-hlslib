#ifndef __EMTF_HLSLIB_ZONESORTING_H__
#define __EMTF_HLSLIB_ZONESORTING_H__

// Function hierarchy
//
// zonesorting_layer
// +-- zonesorting_op (INLINE)
//     |-- zonesorting_preprocess_op
//     |   |-- zonesorting_preprocess_suppress_op (INLINE)
//     |   |-- zonesorting_preprocess_mux_op (INLINE)
//     |   +-- zonesorting_preprocess_sort_op (INLINE)
//     +-- zonesorting_argmax_op

// EMTF HLS
#include "layer_helpers.h"
#include "sort_kernels.h"

namespace emtf {

namespace phase2 {

template <typename T_IN, typename T_OUT>
void zonesorting_preprocess_suppress_op(
    const T_IN in0[zonesorting_config::n_in],
    T_OUT suppression[zonesorting_config::n_in],
    bool_t suppression_v[zonesorting_config::n_in]
) {
  static_assert(is_same<T_IN, zonesorting_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonesorting_in_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const unsigned int n_in = zonesorting_config::n_in;
  typedef trk_qual_t data_t;

  constexpr int bits_lo = 0;
  constexpr int bits_hi = (data_t::width - 1);

  // Perform non-max suppression by comparing with left and right neighbors
  LOOP_PREPROC_1: for (unsigned i = 0; i < n_in; i++) {

#pragma HLS UNROLL

    const bool leftmost = (i == 0);
    const bool rightmost = (i == (n_in - 1));

    // Take the activation value
    const data_t ql = leftmost ? static_cast<data_t>(0) : in0[i - 1].range(bits_hi, bits_lo);
    const data_t qc = in0[i].range(bits_hi, bits_lo);
    const data_t qr = rightmost ? static_cast<data_t>(0) : in0[i + 1].range(bits_hi, bits_lo);

    // Suppress if not local maximum
    // Condition: (qc <= ql || qc < qr)
    const bool suppress = (qc <= ql) or (qc < qr);

    // Output
    suppression[i] = suppress ? static_cast<T_OUT>(0) : in0[i];
    suppression_v[i] = suppress ? static_cast<bool_t>(0) : static_cast<bool_t>(1);
  }  // end loop over columns
}

template <typename T_IN, typename T_OUT>
void zonesorting_preprocess_mux_op(
    const T_IN suppression[zonesorting_config::n_in],
    const bool_t suppression_v[zonesorting_config::n_in],
    T_OUT multiplexed[zonesorting_config::n_stage_0],
    bool_t multiplexed_v[zonesorting_config::n_stage_0]
) {
  static_assert(is_same<T_IN, zonesorting_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonesorting_in_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const unsigned int n_stage_0 = zonesorting_config::n_stage_0;

  // Perform mux for each pair of columns
  // After non-max suppression, the adjacent col next to the local-max col is always zero
  LOOP_PREPROC_2: for (unsigned i = 0; i < n_stage_0; i++) {

#pragma HLS UNROLL

    // x0 = suppression[(i * 2) + 0];
    // x1 = suppression[(i * 2) + 1];
    // If x1 is not suppressed, take x1, else take x0. If both are suppressed, take x0.
    const bool suppress = (suppression_v[(i * 2) + 1] == 0);

    // Output
    multiplexed[i] = suppress ? suppression[(i * 2) + 0] : suppression[(i * 2) + 1];
    multiplexed_v[i] = suppress ? static_cast<bool_t>(0) : static_cast<bool_t>(1);
  }  // end loop over columns
}

template <typename T_IN, typename T_OUT>
void zonesorting_preprocess_sort_op(
    const T_IN multiplexed[zonesorting_config::n_stage_0],
    const bool_t multiplexed_v[zonesorting_config::n_stage_0],
    T_OUT out[zonesorting_config::n_stage_0]
) {
  static_assert(is_same<T_IN, zonesorting_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonesorting_out_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const unsigned int n_stage_0 = zonesorting_config::n_stage_0;
  const unsigned int batch_size = 4;

  // trk_col_t is broken up as 2 pieces: trk_col_trunc_t and trk_col_short_t.
  // trk_col_short_t includes 2 bits to encode 0..3 in a batch + 1 bit inherited from
  // zonesorting_preprocess_mux_op().
  typedef ap_uint<detail::ceil_log2<(batch_size * 2) - 1>::value> trk_col_short_t;  // encodes 0..7
  typedef ap_uint<trk_col_t::width - trk_col_short_t::width> trk_col_trunc_t;
  typedef trk_qual_t data_t;
  typedef ap_uint<T_IN::width - data_t::width + trk_col_short_t::width> arg_t;
  typedef detail::argsort_pair<arg_t, data_t> pair_t;

  constexpr int bits_lo_0 = 0;
  constexpr int bits_lo_1 = data_t::width;
  constexpr int bits_lo_2 = T_IN::width;

  // Loop over columns with step size = batch_size
  LOOP_PREPROC_3: for (unsigned i = 0; i < n_stage_0; i += batch_size) {

#pragma HLS UNROLL

    const unsigned int num_nodes = (batch_size * 2);  // plus 4 output nodes

    pair_t nodes[num_nodes];

#pragma HLS ARRAY_PARTITION variable=nodes complete dim=0

#pragma HLS DATA_PACK variable=nodes

    // Loop over columns in the batch
    LOOP_PREPROC_3_1: for (unsigned j = 0; j < batch_size; j++) {

#pragma HLS UNROLL

      // col is broken up as 2 pieces: (i >> 2) and (j << 1) + b,
      // where b is the one bit inherited from zonesorting_preprocess_mux_op().
      //
      // col = ((i + j) << 1) + b
      //     = ((i >> 2) << 3) + (j << 1) + b
      const trk_col_trunc_t col_trunc = (i >> 2);
      const trk_col_short_t col_short = (j << 1) + multiplexed_v[i + j];
      emtf_assert(
          (col_trunc, col_short) == (((i + j) << 1) + multiplexed_v[i + j])
      );

      // Make pairs
      const data_t data = multiplexed[i + j].range(bits_lo_1 - 1, bits_lo_0);
      const arg_t arg = (col_short, multiplexed[i + j].range(bits_lo_2 - 1, bits_lo_1));
      nodes[j] = pair_t(arg, data);
    }  // end loop over columns in the batch

    // Sort a batch of 4 columns
    // nodes[0]..nodes[3] are the input nodes, nodes[4]..nodes[7] are the output nodes
    detail::sort_four_op(
        nodes[0], nodes[1], nodes[2], nodes[3],
        nodes[4], nodes[5], nodes[6], nodes[7]
    );

    // Sanity check
#ifndef __SYNTHESIS__
    const pair_t& r0 = nodes[4];
    const pair_t& r1 = nodes[5];
    const pair_t& r2 = nodes[6];
    const pair_t& r3 = nodes[7];
    pair_t r4, r5, r6, r7;
    detail::cpp_sort_four_op(
        nodes[0], nodes[1], nodes[2], nodes[3],
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

    // Output (incl column number)
    const trk_col_trunc_t col_trunc = (i >> 2);
    out[i + 0] = (col_trunc, nodes[4].first, nodes[4].second);
    out[i + 1] = (col_trunc, nodes[5].first, nodes[5].second);
    out[i + 2] = (col_trunc, nodes[6].first, nodes[6].second);
    out[i + 3] = (col_trunc, nodes[7].first, nodes[7].second);
  }  // end loop over columns
}

// _____________________________________________________________________________
template <typename T_IN, typename T_OUT>
void zonesorting_preprocess_op(
    const T_IN in0[zonesorting_config::n_in],
    T_OUT out[zonesorting_config::n_stage_0]
) {
  static_assert(is_same<T_IN, zonesorting_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonesorting_out_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const unsigned int n_in = zonesorting_config::n_in;
  const unsigned int n_stage_0 = zonesorting_config::n_stage_0;

  // Intermediate arrays
  T_IN   suppression   [n_in];
  bool_t suppression_v [n_in];
  T_IN   multiplexed   [n_stage_0];
  bool_t multiplexed_v [n_stage_0];

#pragma HLS ARRAY_PARTITION variable=suppression complete dim=0
#pragma HLS ARRAY_PARTITION variable=suppression_v complete dim=0
#pragma HLS ARRAY_PARTITION variable=multiplexed complete dim=0
#pragma HLS ARRAY_PARTITION variable=multiplexed_v complete dim=0

  zonesorting_preprocess_suppress_op(in0, suppression, suppression_v);

  zonesorting_preprocess_mux_op(suppression, suppression_v, multiplexed, multiplexed_v);

  zonesorting_preprocess_sort_op(multiplexed, multiplexed_v, out);
}

// _____________________________________________________________________________
template <typename T_IN, typename T_OUT>
void zonesorting_argmax_op(
    const T_IN in0[zonesorting_config::n_stage_0],
    T_OUT out[zonesorting_config::n_out]
) {
  static_assert(is_same<T_IN, zonesorting_out_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonesorting_out_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const unsigned int N = zonesorting_config::n_stage_0;
  typedef trk_qual_t data_t;
  typedef ap_uint<T_IN::width - data_t::width> arg_t;
  typedef detail::argsort_pair<arg_t, data_t> pair_t;

  constexpr int bits_lo_0 = 0;
  constexpr int bits_lo_1 = data_t::width;
  constexpr int bits_lo_2 = T_IN::width;

  // Octal tree structure (N must be an even number)
  const unsigned int num_nodes = (N * 2) - 4;

  pair_t octal_tree[num_nodes];

#pragma HLS ARRAY_PARTITION variable=octal_tree complete dim=0

#pragma HLS DATA_PACK variable=octal_tree

  // For N = 144, the octal tree is not balanced (as N is not a power of 8), we need to alter
  // the ordering of the nodes. By default:
  //     0 1 2 3 -> 4 .. 11 -> 12 .. 27 -> 28 .. 59 -> 60 .. 123 -> 124 .. 139 140 .. 251 -> 252 .. 283 X .. X
  // In order to preserve order:
  //     0 1 2 3 -> 4 .. 11 -> 12 .. 27 -> 28 .. 59 -> 60 .. 123 -> 124 .. 139 172 .. 283 -> 140 .. 171 X .. X
  // Need to rotate by 128 - (144 - 128) = 112

  // Fetch input
  LOOP_ARGMAX_1: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    const unsigned int node_index = (N - 4) + ((i + 112) % N);  // N-4 .. (N*2)-5 with rotation
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
    detail::merge_eight_op(
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
    detail::cpp_merge_eight_op(
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
// Zone sorting op

template <typename Zone>
void zonesorting_op(
    const zonesorting_in_t zonesorting_in[zonesorting_config::n_in],
    zonesorting_out_t zonesorting_out[zonesorting_config::n_out]
) {

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const unsigned int n_stage_0 = zonesorting_config::n_stage_0;

  // Intermediate arrays
  zonesorting_out_t stage_0_out[n_stage_0];

#pragma HLS ARRAY_PARTITION variable=stage_0_out complete dim=0

  zonesorting_preprocess_op(zonesorting_in, stage_0_out);

  zonesorting_argmax_op(stage_0_out, zonesorting_out);
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void zonesorting_layer(
    const zonesorting_in_t zonesorting_in[zonesorting_config::n_in],
    zonesorting_out_t zonesorting_out[zonesorting_config::n_out]
) {

#pragma HLS PIPELINE II=zonesorting_config::layer_target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

  // Check assumptions
  static_assert(zonesorting_config::n_in == num_emtf_img_cols, "zonesorting_config::n_in check failed");
  static_assert(zonesorting_config::n_out == num_emtf_tracks, "zonesorting_config::n_out check failed");

  zonesorting_op<Zone>(zonesorting_in, zonesorting_out);
}

}  // namespace phase2

}  // namespace emtf

#endif  // __EMTF_HLSLIB_ZONESORTING_H__ not defined
