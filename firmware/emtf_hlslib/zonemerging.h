#ifndef __EMTF_HLSLIB_ZONEMERGING_H__
#define __EMTF_HLSLIB_ZONEMERGING_H__

namespace emtf {

template <typename T_IN, typename T_OUT>
void zonemerging_preprocess_twelve_op(const T_IN in0[4], const T_IN in1[4], const T_IN in2[4], T_OUT out[12]) {
  static_assert(is_same<T_IN, zonemerging_in_t>::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, zonemerging_out_t>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonemerging_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // Output (incl zone number)
  out[0] = (trk_zone_t(0), in0[0]);
  out[1] = (trk_zone_t(0), in0[1]);
  out[2] = (trk_zone_t(0), in0[2]);
  out[3] = (trk_zone_t(0), in0[3]);
  out[4] = (trk_zone_t(1), in1[0]);
  out[5] = (trk_zone_t(1), in1[1]);
  out[6] = (trk_zone_t(1), in1[2]);
  out[7] = (trk_zone_t(1), in1[3]);
  out[8] = (trk_zone_t(2), in2[0]);
  out[9] = (trk_zone_t(2), in2[1]);
  out[10] = (trk_zone_t(2), in2[2]);
  out[11] = (trk_zone_t(2), in2[3]);
}

template <typename T_IN, typename T_OUT>
void zonemerging_merge_twelve_op(const T_IN in0[12], T_OUT out[4]) {
  static_assert(is_same<T_IN, T_OUT>::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zonemerging_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef ap_uint<details::ceil_log2<11>::value> idx_t;  // encodes 0..11
  typedef trk_qual_t data_t;
  typedef details::argsort_pair<idx_t, data_t> pair_t;

  constexpr int bits_lo = 0;
  constexpr int bits_hi = (data_t::width - 1);

  // See zonesorting_merge_eight_op() for explanations. To merge 3 blocks of 4, it merges the
  // first two blocks, then merges the result with the last block.

  // Stage 0: concatenate index and data.
  const pair_t tmp_0_0(idx_t(0), in0[0].range(bits_hi, bits_lo));
  const pair_t tmp_0_1(idx_t(1), in0[1].range(bits_hi, bits_lo));
  const pair_t tmp_0_2(idx_t(2), in0[2].range(bits_hi, bits_lo));
  const pair_t tmp_0_3(idx_t(3), in0[3].range(bits_hi, bits_lo));
  const pair_t tmp_0_4(idx_t(4), in0[4].range(bits_hi, bits_lo));
  const pair_t tmp_0_5(idx_t(5), in0[5].range(bits_hi, bits_lo));
  const pair_t tmp_0_6(idx_t(6), in0[6].range(bits_hi, bits_lo));
  const pair_t tmp_0_7(idx_t(7), in0[7].range(bits_hi, bits_lo));
  const pair_t tmp_0_8(idx_t(8), in0[8].range(bits_hi, bits_lo));
  const pair_t tmp_0_9(idx_t(9), in0[9].range(bits_hi, bits_lo));
  const pair_t tmp_0_10(idx_t(10), in0[10].range(bits_hi, bits_lo));
  const pair_t tmp_0_11(idx_t(11), in0[11].range(bits_hi, bits_lo));

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

  // Stage 4 (similar to Stage 1)
  const pair_t tmp_4_0 = (tmp_1_0 < tmp_0_8) ? tmp_0_8 : tmp_1_0;
  const pair_t tmp_4_1 = (tmp_3_1 < tmp_0_9) ? tmp_0_9 : tmp_3_1;
  const pair_t tmp_4_2 = (tmp_3_2 < tmp_0_10) ? tmp_0_10 : tmp_3_2;
  const pair_t tmp_4_3 = (tmp_3_3 < tmp_0_11) ? tmp_0_11 : tmp_3_3;
  const pair_t tmp_4_4 = (tmp_1_0 < tmp_0_8) ? tmp_1_0 : tmp_0_8;
  const pair_t tmp_4_5 = (tmp_3_1 < tmp_0_9) ? tmp_3_1 : tmp_0_9;
  //const pair_t tmp_4_6 = (tmp_3_2 < tmp_0_10) ? tmp_3_2 : tmp_0_10;  // unused
  //const pair_t tmp_4_7 = (tmp_3_3 < tmp_0_11) ? tmp_3_3 : tmp_0_11;  // unused

  // Stage 5 (similar to Stage 2)
  const pair_t tmp_5_2 = (tmp_4_2 < tmp_4_4) ? tmp_4_4 : tmp_4_2;
  const pair_t tmp_5_3 = (tmp_4_3 < tmp_4_5) ? tmp_4_5 : tmp_4_3;
  const pair_t tmp_5_4 = (tmp_4_2 < tmp_4_4) ? tmp_4_2 : tmp_4_4;
  //const pair_t tmp_5_5 = (tmp_4_3 < tmp_4_5) ? tmp_4_3 : tmp_4_5;  // unused

  // Stage 6 (similar to Stage 3)
  const pair_t tmp_6_1 = (tmp_4_1 < tmp_5_2) ? tmp_5_2 : tmp_4_1;
  const pair_t tmp_6_2 = (tmp_4_1 < tmp_5_2) ? tmp_4_1 : tmp_5_2;
  const pair_t tmp_6_3 = (tmp_5_3 < tmp_5_4) ? tmp_5_4 : tmp_5_3;
  //const pair_t tmp_6_4 = (tmp_5_3 < tmp_5_4) ? tmp_5_3 : tmp_5_4;  // unused
  //const pair_t tmp_6_5 = (tmp_5_5 < tmp_4_6) ? tmp_4_6 : tmp_5_5;  // unused
  //const pair_t tmp_6_6 = (tmp_5_5 < tmp_4_6) ? tmp_5_5 : tmp_4_6;  // unused

#pragma HLS DATA_PACK variable=tmp_0_0
#pragma HLS DATA_PACK variable=tmp_0_1
#pragma HLS DATA_PACK variable=tmp_0_2
#pragma HLS DATA_PACK variable=tmp_0_3
#pragma HLS DATA_PACK variable=tmp_0_4
#pragma HLS DATA_PACK variable=tmp_0_5
#pragma HLS DATA_PACK variable=tmp_0_6
#pragma HLS DATA_PACK variable=tmp_0_7
#pragma HLS DATA_PACK variable=tmp_0_8
#pragma HLS DATA_PACK variable=tmp_0_9
#pragma HLS DATA_PACK variable=tmp_0_10
#pragma HLS DATA_PACK variable=tmp_0_11
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
#pragma HLS DATA_PACK variable=tmp_4_0
#pragma HLS DATA_PACK variable=tmp_4_1
#pragma HLS DATA_PACK variable=tmp_4_2
#pragma HLS DATA_PACK variable=tmp_4_3
#pragma HLS DATA_PACK variable=tmp_4_4
#pragma HLS DATA_PACK variable=tmp_4_5
#pragma HLS DATA_PACK variable=tmp_5_2
#pragma HLS DATA_PACK variable=tmp_5_3
#pragma HLS DATA_PACK variable=tmp_5_4
#pragma HLS DATA_PACK variable=tmp_6_1
#pragma HLS DATA_PACK variable=tmp_6_2
#pragma HLS DATA_PACK variable=tmp_6_3

  // Output
  const idx_t idx0 = tmp_4_0.first;
  const idx_t idx1 = tmp_6_1.first;
  const idx_t idx2 = tmp_6_2.first;
  const idx_t idx3 = tmp_6_3.first;

  out[0] = in0[idx0];
  out[1] = in0[idx1];
  out[2] = in0[idx2];
  out[3] = in0[idx3];
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

  zonemerging_out_t stage_0_out[zonemerging_config::n_stage_0];

#pragma HLS ARRAY_PARTITION variable=stage_0_out complete dim=0

  // Preprocessing and concatenate them.
  zonemerging_preprocess_twelve_op(zonemerging_in_0, zonemerging_in_1, zonemerging_in_2, stage_0_out);

  // Merge 3 blocks of 4.
  zonemerging_merge_twelve_op(stage_0_out, zonemerging_out);
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
  static_assert((zonemerging_config::n_stage_0 % 4) == 0, "n_stage_0 must be divisible by 4");

  zonemerging_op<Zone>(zonemerging_in_0, zonemerging_in_1, zonemerging_in_2, zonemerging_out);
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_ZONEMERGING_H__ not defined
