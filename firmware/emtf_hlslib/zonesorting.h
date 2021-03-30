#ifndef __EMTF_HLSLIB_ZONESORTING_H__
#define __EMTF_HLSLIB_ZONESORTING_H__

namespace emtf {

// Perform mux for each pair of cols, and sort each block of 4 cols
template <typename T_IN, typename T_OUT>
void zonesorting_preprocess_eight_op(const trk_col_t col, const T_IN in0[8], T_OUT out[4]) {
  static_assert(is_same<T_IN, zonesorting_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonesorting_out_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef ap_uint<details::ceil_log2<7>::value> idx_t;  // encodes 0..7
  typedef trk_qual_t data_t;
  typedef details::argsort_pair<idx_t, data_t> pair_t;

  constexpr int bits_lo = 0;
  constexpr int bits_hi = (data_t::width - 1);

  // Stage 0: implement mux for each pair of cols, concatenate index and data.
  const pair_t tmp_f_0(idx_t(0), in0[0].range(bits_hi, bits_lo));
  const pair_t tmp_f_1(idx_t(1), in0[1].range(bits_hi, bits_lo));
  const pair_t tmp_f_2(idx_t(2), in0[2].range(bits_hi, bits_lo));
  const pair_t tmp_f_3(idx_t(3), in0[3].range(bits_hi, bits_lo));
  const pair_t tmp_f_4(idx_t(4), in0[4].range(bits_hi, bits_lo));
  const pair_t tmp_f_5(idx_t(5), in0[5].range(bits_hi, bits_lo));
  const pair_t tmp_f_6(idx_t(6), in0[6].range(bits_hi, bits_lo));
  const pair_t tmp_f_7(idx_t(7), in0[7].range(bits_hi, bits_lo));

  const pair_t tmp_0_0 = (tmp_f_0.second != 0) ? tmp_f_0 : tmp_f_1;
  const pair_t tmp_0_1 = (tmp_f_2.second != 0) ? tmp_f_2 : tmp_f_3;
  const pair_t tmp_0_2 = (tmp_f_4.second != 0) ? tmp_f_4 : tmp_f_5;
  const pair_t tmp_0_3 = (tmp_f_6.second != 0) ? tmp_f_6 : tmp_f_7;

#pragma HLS DATA_PACK variable=tmp_f_0
#pragma HLS DATA_PACK variable=tmp_f_1
#pragma HLS DATA_PACK variable=tmp_f_2
#pragma HLS DATA_PACK variable=tmp_f_3
#pragma HLS DATA_PACK variable=tmp_f_4
#pragma HLS DATA_PACK variable=tmp_f_5
#pragma HLS DATA_PACK variable=tmp_f_6
#pragma HLS DATA_PACK variable=tmp_f_7
#pragma HLS DATA_PACK variable=tmp_0_0
#pragma HLS DATA_PACK variable=tmp_0_1
#pragma HLS DATA_PACK variable=tmp_0_2
#pragma HLS DATA_PACK variable=tmp_0_3

  // The following implements the bubble sorting network, which is a stable sort algo.
  // There are 4 wires (0,1,2,3 from top to bottom), and there are 5 stages (not counting
  // Stage 0 which is used as input).
  // In each stage, compare a pair of wires (connected by a vertical line in the diagram).
  // If the value of the top wire is less than that of the bottom wire, swap the wires.
  // Thus, the smallest value moves to the bottom. At the end, all the values are sorted
  // (descending from top to bottom).
  //
  // 0   1   2   3   4   5
  // ----o-------o-------o----
  //     |       |       |
  // ----o---o---o---o---o----
  //         |       |
  // --------o---o---o--------
  //             |
  // ------------o------------

  // Stage 1: compare-swap if (wire[i] < wire[j]) swap(wire[j], wire[i])
  const pair_t tmp_1_0 = (tmp_0_0 < tmp_0_1) ? tmp_0_1 : tmp_0_0;
  const pair_t tmp_1_1 = (tmp_0_0 < tmp_0_1) ? tmp_0_0 : tmp_0_1;

  // Stage 2
  const pair_t tmp_2_1 = (tmp_1_1 < tmp_0_2) ? tmp_0_2 : tmp_1_1;
  const pair_t tmp_2_2 = (tmp_1_1 < tmp_0_2) ? tmp_1_1 : tmp_0_2;

  // Stage 3
  const pair_t tmp_3_0 = (tmp_1_0 < tmp_2_1) ? tmp_2_1 : tmp_1_0;
  const pair_t tmp_3_1 = (tmp_1_0 < tmp_2_1) ? tmp_1_0 : tmp_2_1;
  const pair_t tmp_3_2 = (tmp_2_2 < tmp_0_3) ? tmp_0_3 : tmp_2_2;
  const pair_t tmp_3_3 = (tmp_2_2 < tmp_0_3) ? tmp_2_2 : tmp_0_3;

  // Stage 4
  const pair_t tmp_4_1 = (tmp_3_1 < tmp_3_2) ? tmp_3_2 : tmp_3_1;
  const pair_t tmp_4_2 = (tmp_3_1 < tmp_3_2) ? tmp_3_1 : tmp_3_2;

  // Stage 5
  const pair_t tmp_5_0 = (tmp_3_0 < tmp_4_1) ? tmp_4_1 : tmp_3_0;
  const pair_t tmp_5_1 = (tmp_3_0 < tmp_4_1) ? tmp_3_0 : tmp_4_1;

#pragma HLS DATA_PACK variable=tmp_1_0
#pragma HLS DATA_PACK variable=tmp_1_1
#pragma HLS DATA_PACK variable=tmp_2_1
#pragma HLS DATA_PACK variable=tmp_2_2
#pragma HLS DATA_PACK variable=tmp_3_0
#pragma HLS DATA_PACK variable=tmp_3_1
#pragma HLS DATA_PACK variable=tmp_3_2
#pragma HLS DATA_PACK variable=tmp_3_3
#pragma HLS DATA_PACK variable=tmp_4_1
#pragma HLS DATA_PACK variable=tmp_4_2
#pragma HLS DATA_PACK variable=tmp_5_0
#pragma HLS DATA_PACK variable=tmp_5_1

  // Output (incl column number)
  const idx_t idx0 = tmp_5_0.first;
  const idx_t idx1 = tmp_5_1.first;
  const idx_t idx2 = tmp_4_2.first;
  const idx_t idx3 = tmp_3_3.first;

  out[0] = (trk_col_t(col + idx0), in0[idx0]);
  out[1] = (trk_col_t(col + idx1), in0[idx1]);
  out[2] = (trk_col_t(col + idx2), in0[idx2]);
  out[3] = (trk_col_t(col + idx3), in0[idx3]);
}

// Merge each pair of blocks (a block has 4 entries)
template <typename T_IN, typename T_OUT>
void zonesorting_merge_eight_op(const T_IN in0[8], T_OUT out[4]) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef ap_uint<details::ceil_log2<7>::value> idx_t;  // encodes 0..7
  typedef trk_qual_t data_t;
  typedef details::argsort_pair<idx_t, data_t> pair_t;

  constexpr int bits_lo = 0;
  constexpr int bits_hi = (data_t::width - 1);

  // The following implements the odd-even merge sorting network, which is *not* a stable sort algo.
  // There are 8 wires (0..7 from top to bottom), and there are 3 stages (not counting
  // Stage 0 which is used as input). Each set of top 4 and bottom 4 wires is assumed to
  // be sorted.
  // In each stage, compare a pair of wires (connected by a vertical line in the diagram).
  // If the value of the top wire is less than that of the bottom wire, swap the wires.
  // Thus, the smallest value moves to the bottom. At the end, all the values are sorted
  // (descending from top to bottom). Only top 4 are needed.
  //
  // 0     1         2       3
  // ----o-------------------------
  //     |
  // ----|-o-----------------o-----
  //     | |                 |
  // ----|-|-o-------o-------o-----
  //     | | |       |
  // ----|-|-|-o-----|-o-----o-----
  //     | | | |     | |     |
  // ----o-|-|-|-----o-|-----o-----
  //       | | |       |
  // ------o-|-|-------o-----o-----
  //         | |             |
  // --------o-|-------------o-----
  //           |
  // ----------o-------------------

  // Stage 0: concatenate index and data.
  const pair_t tmp_0_0(idx_t(0), in0[0].range(bits_hi, bits_lo));
  const pair_t tmp_0_1(idx_t(1), in0[1].range(bits_hi, bits_lo));
  const pair_t tmp_0_2(idx_t(2), in0[2].range(bits_hi, bits_lo));
  const pair_t tmp_0_3(idx_t(3), in0[3].range(bits_hi, bits_lo));
  const pair_t tmp_0_4(idx_t(4), in0[4].range(bits_hi, bits_lo));
  const pair_t tmp_0_5(idx_t(5), in0[5].range(bits_hi, bits_lo));
  const pair_t tmp_0_6(idx_t(6), in0[6].range(bits_hi, bits_lo));
  const pair_t tmp_0_7(idx_t(7), in0[7].range(bits_hi, bits_lo));

  // Stage 1: compare-swap if (wire[i] < wire[j]) swap(wire[j], wire[i])
  const pair_t tmp_1_0 = (tmp_0_0 < tmp_0_4) ? tmp_0_4 : tmp_0_0;
  const pair_t tmp_1_1 = (tmp_0_1 < tmp_0_5) ? tmp_0_5 : tmp_0_1;
  const pair_t tmp_1_2 = (tmp_0_2 < tmp_0_6) ? tmp_0_6 : tmp_0_2;
  const pair_t tmp_1_3 = (tmp_0_3 < tmp_0_7) ? tmp_0_7 : tmp_0_3;
  const pair_t tmp_1_4 = (tmp_0_0 < tmp_0_4) ? tmp_0_0 : tmp_0_4;
  const pair_t tmp_1_5 = (tmp_0_1 < tmp_0_5) ? tmp_0_1 : tmp_0_5;
  //const pair_t tmp_1_6 = (tmp_0_2 < tmp_0_6) ? tmp_0_2 : tmp_0_6;  // unused
  //const pair_t tmp_1_7 = (tmp_0_3 < tmp_0_7) ? tmp_0_3 : tmp_0_7;  // unused

  // Stage 2
  const pair_t tmp_2_2 = (tmp_1_2 < tmp_1_4) ? tmp_1_4 : tmp_1_2;
  const pair_t tmp_2_3 = (tmp_1_3 < tmp_1_5) ? tmp_1_5 : tmp_1_3;
  const pair_t tmp_2_4 = (tmp_1_2 < tmp_1_4) ? tmp_1_2 : tmp_1_4;
  //const pair_t tmp_2_5 = (tmp_1_3 < tmp_1_5) ? tmp_1_3 : tmp_1_5;  // unused

  // Stage 3
  const pair_t tmp_3_1 = (tmp_1_1 < tmp_2_2) ? tmp_2_2 : tmp_1_1;
  const pair_t tmp_3_2 = (tmp_1_1 < tmp_2_2) ? tmp_1_1 : tmp_2_2;
  const pair_t tmp_3_3 = (tmp_2_3 < tmp_2_4) ? tmp_2_4 : tmp_2_3;
  //const pair_t tmp_3_4 = (tmp_2_3 < tmp_2_4) ? tmp_2_3 : tmp_2_4;  // unused
  //const pair_t tmp_3_5 = (tmp_2_5 < tmp_1_6) ? tmp_1_6 : tmp_2_5;  // unused
  //const pair_t tmp_3_6 = (tmp_2_5 < tmp_1_6) ? tmp_2_5 : tmp_1_6;  // unused

#pragma HLS DATA_PACK variable=tmp_0_0
#pragma HLS DATA_PACK variable=tmp_0_1
#pragma HLS DATA_PACK variable=tmp_0_2
#pragma HLS DATA_PACK variable=tmp_0_3
#pragma HLS DATA_PACK variable=tmp_0_4
#pragma HLS DATA_PACK variable=tmp_0_5
#pragma HLS DATA_PACK variable=tmp_0_6
#pragma HLS DATA_PACK variable=tmp_0_7
#pragma HLS DATA_PACK variable=tmp_1_0
#pragma HLS DATA_PACK variable=tmp_1_1
#pragma HLS DATA_PACK variable=tmp_1_2
#pragma HLS DATA_PACK variable=tmp_1_3
#pragma HLS DATA_PACK variable=tmp_1_4
#pragma HLS DATA_PACK variable=tmp_1_5
#pragma HLS DATA_PACK variable=tmp_2_2
#pragma HLS DATA_PACK variable=tmp_2_3
#pragma HLS DATA_PACK variable=tmp_2_4
#pragma HLS DATA_PACK variable=tmp_3_1
#pragma HLS DATA_PACK variable=tmp_3_2
#pragma HLS DATA_PACK variable=tmp_3_3

  // Output
  const idx_t idx0 = tmp_1_0.first;
  const idx_t idx1 = tmp_3_1.first;
  const idx_t idx2 = tmp_3_2.first;
  const idx_t idx3 = tmp_3_3.first;

  out[0] = in0[idx0];
  out[1] = in0[idx1];
  out[2] = in0[idx2];
  out[3] = in0[idx3];
}

// _____________________________________________________________________________
// A wrapper for preprocessing (incl sorting)
template <unsigned int N_IN, unsigned int N_OUT0, unsigned int N_OUT1, typename T_IN, typename T_OUT>
void zonesorting_preprocess_stage_op(const T_IN in0[N_IN], T_OUT out0[N_OUT0], T_OUT out1[N_OUT1]) {
  static_assert(is_same<T_IN, zonesorting_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonesorting_out_t>::value, "T_OUT type check failed");
  static_assert((N_IN / 2) == (N_OUT0 + N_OUT1), "Inconsistent N_IN, N_OUT0, and N_OUT1");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const int batch_size = zonesorting_config::batch_size;

  // Loop with step size 8
  LOOP_PRE: for (unsigned i = 0; i < N_IN; i += batch_size) {

#pragma HLS UNROLL

    if ((i/2) < N_OUT0) {
      zonesorting_preprocess_eight_op(i, &(in0[i]), &(out0[i/2]));
    } else {
      // Special case for the last 16 cols
      emtf_assert(((i/2) - N_OUT0) < N_OUT1);
      zonesorting_preprocess_eight_op(i, &(in0[i]), &(out1[(i/2) - N_OUT0]));
    }
  }
}

// A wrapper for merging
template <unsigned int N_IN, unsigned int N_OUT=N_IN/2, typename T_IN, typename T_OUT>
void zonesorting_merge_stage_op(const T_IN in0[N_IN], T_OUT out[N_OUT]) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const int batch_size = zonesorting_config::batch_size;

  // Loop with step size 8
  LOOP_MGE: for (unsigned i = 0; i < N_IN; i += batch_size) {

#pragma HLS UNROLL

    zonesorting_merge_eight_op(&(in0[i]), &(out[i/2]));
  }
}

// A wrapper for merging
template <unsigned int N_IN, unsigned int N_OUT=N_IN/2, typename T_IN, typename T_OUT>
void zonesorting_merge_stage_op(const T_IN in0[N_IN], T_OUT out[N_OUT], m_inline_directive) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  const int batch_size = zonesorting_config::batch_size;

  // Loop with step size 8
  LOOP_MGE_INLINE: for (unsigned i = 0; i < N_IN; i += batch_size) {

#pragma HLS UNROLL

    zonesorting_merge_eight_op(&(in0[i]), &(out[i/2]));
  }
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

  const int n_stage_0 = zonesorting_config::n_stage_0;
  const int n_stage_1 = zonesorting_config::n_stage_1;
  const int n_stage_2 = zonesorting_config::n_stage_2;
  const int n_stage_3 = zonesorting_config::n_stage_3;
  const int n_stage_4 = zonesorting_config::n_stage_4;
  const int n_stage_5 = zonesorting_config::n_stage_5;

  zonesorting_out_t stage_0_out[n_stage_0];
  zonesorting_out_t stage_1_out[n_stage_1];
  zonesorting_out_t stage_2_out[n_stage_2];
  zonesorting_out_t stage_3_out[n_stage_3];
  zonesorting_out_t stage_4_out[n_stage_4];
  zonesorting_out_t stage_5_out[n_stage_5];

#pragma HLS ARRAY_PARTITION variable=stage_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=stage_1_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=stage_2_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=stage_3_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=stage_4_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=stage_5_out complete dim=0

  // Stage 0: perform mux for each pair of cols, and sort each block of 4 cols. N: 288 -> 144
  // Stage 0.5: the last 16 cols are excluded from stage 0 output, but instead copied to stage 3 output. N: 144 -> 128
  // Stage 1: merge each pair of blocks. N: 128 -> 64
  // Stage 2: merge each pair of blocks. N: 64 -> 32
  // Stage 3: merge each pair of blocks, include the 16 cols that were excluded. N: 32 (-> 16) -> 32
  // Stage 4: merge each pair of blocks. N: 32 -> 16
  // Stage 5: merge each pair of blocks. N: 16 -> 8
  // Stage 6: merge each pair of blocks, write to output. 8 -> 4

  // Implement mux for each pair of cols.
  // After non-max suppression, the adjacent col next to the local-max col is always zero.
  // Then, sort each block of 4 cols.
  // Take care of the special case for the last 16 cols.
  const int n_skipped_stage_0 = zonesorting_config::n_skipped_stage_0;
  zonesorting_preprocess_stage_op<zonesorting_config::n_in, n_stage_0, n_skipped_stage_0>(
      zonesorting_in, stage_0_out, &(stage_3_out[n_skipped_stage_0])
  );

  // Merge each pair of blocks.
  zonesorting_merge_stage_op<n_stage_0>(stage_0_out, stage_1_out);
  zonesorting_merge_stage_op<n_stage_1>(stage_1_out, stage_2_out);
  zonesorting_merge_stage_op<n_stage_2>(stage_2_out, stage_3_out);
  zonesorting_merge_stage_op<n_stage_3>(stage_3_out, stage_4_out);
  zonesorting_merge_stage_op<n_stage_4>(stage_4_out, stage_5_out, m_inline_directive());
  zonesorting_merge_stage_op<n_stage_5>(stage_5_out, zonesorting_out, m_inline_directive());
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void zonesorting_layer(
    const zonesorting_in_t zonesorting_in[zonesorting_config::n_in],
    zonesorting_out_t zonesorting_out[zonesorting_config::n_out]
) {

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

  // Check assumptions
  static_assert(zonesorting_config::n_in == num_emtf_img_cols, "zonesorting_config::n_in check failed");
  static_assert(zonesorting_config::n_out == num_emtf_tracks, "zonesorting_config::n_out check failed");
  static_assert(zonesorting_config::batch_size == 8, "batch_size must be 8");
  static_assert((zonesorting_config::n_stage_0 % 8) == 0, "n_stage_0 must be divisible by 8");
  static_assert((zonesorting_config::n_stage_1 % 8) == 0, "n_stage_1 must be divisible by 8");
  static_assert((zonesorting_config::n_stage_2 % 8) == 0, "n_stage_2 must be divisible by 8");
  static_assert((zonesorting_config::n_stage_3 % 8) == 0, "n_stage_3 must be divisible by 8");
  static_assert((zonesorting_config::n_stage_4 % 8) == 0, "n_stage_4 must be divisible by 8");
  static_assert((zonesorting_config::n_stage_5 % 8) == 0, "n_stage_5 must be divisible by 8");
  static_assert((zonesorting_config::n_stage_6) == num_emtf_tracks, "n_stage_6 must be equal to num_emtf_tracks");

  zonesorting_op<Zone>(zonesorting_in, zonesorting_out);
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_ZONESORTING_H__ not defined
