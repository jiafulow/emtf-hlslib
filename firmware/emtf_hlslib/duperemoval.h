#ifndef __EMTF_HLSLIB_DUPEREMOVAL_H__
#define __EMTF_HLSLIB_DUPEREMOVAL_H__

// Function hierarchy
//
// duperemoval_layer
// +-- duperemoval_op (INLINE)
//     |-- duperemoval_preprocess_op (INLINE)
//     |-- duperemoval_find_dupes_op (INLINE)
//     +-- duperemoval_remove_dupes_op (INLINE)

// EMTF HLS
#include "layer_helpers.h"
#include "copy_kernels.h"

namespace emtf_hlslib {

namespace phase2 {

template <typename T = void>
void duperemoval_preprocess_op(const trk_seg_t trk_seg[duperemoval_config::n_in * num_emtf_sites],
                               const trk_seg_v_t trk_seg_v[duperemoval_config::n_in],
                               trk_seg_t trk_seg_reduced[duperemoval_config::n_in * num_emtf_sites_rm],
                               trk_seg_v_t trk_seg_reduced_v[duperemoval_config::n_in]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = duperemoval_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Reduce to 5 sites for duplicate removal: ME1, ME2, ME3, ME4, ME0
  //
  // site (out) | site (in)
  // -----------|-------------------------------------------
  // ME1        | ME1/1, GE1/1, ME1/2, RE1/2
  // ME2        | ME2, GE2/1, RE2/2
  // ME3        | ME3, RE3
  // ME4        | ME4, RE4
  // ME0        | ME0

  const unsigned int N = duperemoval_config::n_in;
  typedef trk_seg_t data_t;

  // Loop over tracks
LOOP_TRK_1:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Priority encoder (input at lower index has higher priority)
    //
    //  0,  9,  1,  5 -> 0
    //  2, 10,  6     -> 1
    //  3,  7         -> 2
    //  4,  8         -> 3
    // 11             -> 4

    const unsigned begin_index = (i * num_emtf_sites);
    const unsigned reduced_begin_index = (i * num_emtf_sites_rm);

    // Loop over trk_seg and trk_seg_v manually
    const data_t tmp_0 = trk_seg[begin_index + 0];
    const data_t tmp_1 = trk_seg[begin_index + 1];
    const data_t tmp_2 = trk_seg[begin_index + 2];
    const data_t tmp_3 = trk_seg[begin_index + 3];
    const data_t tmp_4 = trk_seg[begin_index + 4];
    const data_t tmp_5 = trk_seg[begin_index + 5];
    const data_t tmp_6 = trk_seg[begin_index + 6];
    const data_t tmp_7 = trk_seg[begin_index + 7];
    const data_t tmp_8 = trk_seg[begin_index + 8];
    const data_t tmp_9 = trk_seg[begin_index + 9];
    const data_t tmp_a = trk_seg[begin_index + 10];
    const data_t tmp_b = trk_seg[begin_index + 11];

    const bool_t vld_0 = trk_seg_v[i][0];
    const bool_t vld_1 = trk_seg_v[i][1];
    const bool_t vld_2 = trk_seg_v[i][2];
    const bool_t vld_3 = trk_seg_v[i][3];
    const bool_t vld_4 = trk_seg_v[i][4];
    const bool_t vld_5 = trk_seg_v[i][5];
    const bool_t vld_6 = trk_seg_v[i][6];
    const bool_t vld_7 = trk_seg_v[i][7];
    const bool_t vld_8 = trk_seg_v[i][8];
    const bool_t vld_9 = trk_seg_v[i][9];
    const bool_t vld_a = trk_seg_v[i][10];
    const bool_t vld_b = trk_seg_v[i][11];

    // Mux
    trk_seg_reduced[reduced_begin_index + 0] = vld_0 ? tmp_0 : (vld_9 ? tmp_9 : (vld_1 ? tmp_1 : tmp_5));
    trk_seg_reduced[reduced_begin_index + 1] = vld_2 ? tmp_2 : (vld_a ? tmp_a : tmp_6);
    trk_seg_reduced[reduced_begin_index + 2] = vld_3 ? tmp_3 : tmp_7;
    trk_seg_reduced[reduced_begin_index + 3] = vld_4 ? tmp_4 : tmp_8;
    trk_seg_reduced[reduced_begin_index + 4] = tmp_b;

    // Bitwise OR
    trk_seg_reduced_v[i] = 0;
    trk_seg_reduced_v[i][0] = vld_0 | vld_9 | vld_1 | vld_5;
    trk_seg_reduced_v[i][1] = vld_2 | vld_a | vld_6;
    trk_seg_reduced_v[i][2] = vld_3 | vld_7;
    trk_seg_reduced_v[i][3] = vld_4 | vld_8;
    trk_seg_reduced_v[i][4] = vld_b;
  }  // end loop over tracks
}

template <typename T = void>
void duperemoval_find_dupes_op(const trk_seg_t trk_seg_reduced[duperemoval_config::n_in * num_emtf_sites_rm],
                               const trk_seg_v_t trk_seg_reduced_v[duperemoval_config::n_in],
                               dio_survivor_t survivors[duperemoval_config::n_in]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = duperemoval_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Find duplicates by checking pairs of tracks and their associated segments. Basically:
  //
  // for i in tracks:
  //   for j in tracks:
  //     for k in range(num_emtf_sites_rm):
  //       has_shared_seg(...)

  const unsigned int N = duperemoval_config::n_in;
  const unsigned int N_sq = N * N;

  // survivors_tmp is used to keep the not-duplicate tracks
  bool_t survivors_tmp[N_sq];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = survivors_tmp complete dim = 0
  // hls-pragmas end

LOOP_TRK_2:
  for (unsigned i = 0; i < N_sq; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    survivors_tmp[i] = 0;  // init as zero

  }  // end i loop

  // Survivor count
  trk_origin_t cnt = 0;

  // trk 0 is not a duplicate by construction
  survivors_tmp[(static_cast<unsigned>(cnt) * N) + 0] = 1;  // set bit to 1
  cnt++;

  // Loop over pairs of tracks (trk 0 is skipped), and check all 5 sites for each pair
LOOP_TRK_3:
  for (unsigned i = 1; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Kill the track if it is a duplicate, or is invalid
    bool_t killed = !(trk_seg_reduced_v[i].range(num_emtf_sites_rm - 1, 0));

  LOOP_TRK_3_1:
    for (unsigned j = 0; j < i; j++) {
      // hls-pragmas begin
#pragma HLS UNROLL
      // hls-pragmas end

      bool_t killed_tmp = 0;

    LOOP_TRK_3_1_1:
      for (unsigned k = 0; k < num_emtf_sites_rm; k++) {
        // hls-pragmas begin
#pragma HLS UNROLL
        // hls-pragmas end

        const bool_t vld_i = trk_seg_reduced_v[i][k];
        const bool_t vld_j = trk_seg_reduced_v[j][k];
        const bool_t has_shared_seg =
            (trk_seg_reduced[(i * num_emtf_sites_rm) + k] == trk_seg_reduced[(j * num_emtf_sites_rm) + k]);
        killed_tmp |= (vld_i & vld_j & has_shared_seg);  // bitwise OR over all 5 sites

      }  // end k loop

      killed |= killed_tmp;  // bitwise OR over all tracks (j < i)

    }  // end j loop

    // Survive if not a duplicate
    if (!killed) {
      survivors_tmp[(static_cast<unsigned>(cnt) * N) + i] = 1;  // set bit to 1
      cnt++;
    }
  }  // end i loop

  // Output
LOOP_TRK_4:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Bit concatenation
    auto&& x_i =
        (survivors_tmp[(i * N) + 3], survivors_tmp[(i * N) + 2], survivors_tmp[(i * N) + 1], survivors_tmp[(i * N) + 0])
            .get();
    emtf_assert(x_i.length() == dio_survivor_t::width);

    survivors[i] = x_i;
  }  // end i loop

  // std::cout << "[DEBUG] trk_seg_reduced_v: " << std::endl;
  // for (unsigned i = 0; i < N; i++) {
  //   for (unsigned j = 0; j < num_emtf_sites_rm; j++) {
  //     std::cout << trk_seg_reduced_v[i][j] << " ";
  //   }
  //   std::cout << std::endl;
  // }
  // std::cout << "[DEBUG] survivors: " << std::endl;
  // for (unsigned i = 0; i < N; i++) {
  //   for (unsigned j = 0; j < N; j++) {
  //     std::cout << survivors[i][j] << " ";
  //   }
  //   std::cout << std::endl;
  // }
}

template <typename T = void>
void duperemoval_remove_dupes_op(const trk_seg_t trk_seg[duperemoval_config::n_in * num_emtf_sites],
                                 const trk_seg_v_t trk_seg_v[duperemoval_config::n_in],
                                 const trk_feat_t trk_feat[duperemoval_config::n_in * num_emtf_features],
                                 const trk_valid_t trk_valid[duperemoval_config::n_in],
                                 const dio_survivor_t survivors[duperemoval_config::n_in],
                                 trk_seg_t trk_seg_rm[duperemoval_config::n_out * num_emtf_sites],
                                 trk_seg_v_t trk_seg_rm_v[duperemoval_config::n_out],
                                 trk_feat_t trk_feat_rm[duperemoval_config::n_out * num_emtf_features],
                                 trk_valid_t trk_valid_rm[duperemoval_config::n_out],
                                 trk_origin_t trk_origin_rm[duperemoval_config::n_out]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = duperemoval_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Remove duplicates by saving only the survivors
  const trk_seg_t invalid_marker_trk_seg = model_config::n_in;
  const trk_feat_t invalid_marker_trk_feat = 0;

  // Multiplex to output
LOOP_TRK_5:
  for (unsigned i = 0; i < duperemoval_config::n_out; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Fill with default values
    trk_seg_rm_v[i] = 0;
    trk_valid_rm[i] = 0;
    trk_origin_rm[i] = 0;
    detail::fill_n_values<num_emtf_sites>(&(trk_seg_rm[i * num_emtf_sites]), invalid_marker_trk_seg);
    detail::fill_n_values<num_emtf_features>(&(trk_feat_rm[i * num_emtf_features]), invalid_marker_trk_feat);

    // Loop over possible tracks (j >= i)
  LOOP_TRK_5_1:
    for (unsigned j = i; j < duperemoval_config::n_in; j++) {
      // hls-pragmas begin
#pragma HLS UNROLL
      // hls-pragmas end

      // trk 0 is a special case where i == j by construction
      if ((i == 0) and (j != 0))
        continue;

      // Should be at most one survivor
      const bool_t survived = survivors[i][j];

      if (survived) {
        trk_seg_rm_v[i] = trk_seg_v[j];
        trk_valid_rm[i] = trk_valid[j];
        trk_origin_rm[i] = j;
        // Copy to arrays
        detail::copy_n_values<num_emtf_sites>(&(trk_seg[j * num_emtf_sites]), &(trk_seg_rm[i * num_emtf_sites]));
        detail::copy_n_values<num_emtf_features>(&(trk_feat[j * num_emtf_features]),
                                                 &(trk_feat_rm[i * num_emtf_features]));
      }
    }  // end j loop
  }    // end i loop

  // Sanity check
#ifndef __SYNTHESIS__
  auto is_power_of_two = [](unsigned x) -> bool { return ((x & (x - 1)) == 0); };
  for (unsigned i = 0; i < duperemoval_config::n_out; i++) {
    emtf_assert(is_power_of_two(static_cast<unsigned>(survivors[i])));  // make sure it is either zero or power of 2
  }
#endif  // __SYNTHESIS__ not defined
}

// _____________________________________________________________________________
// Duplicate removal op

template <typename Zone>
void duperemoval_op(const trk_seg_t trk_seg[duperemoval_config::n_in * num_emtf_sites],
                    const trk_seg_v_t trk_seg_v[duperemoval_config::n_in],
                    const trk_feat_t trk_feat[duperemoval_config::n_in * num_emtf_features],
                    const trk_valid_t trk_valid[duperemoval_config::n_in],
                    trk_seg_t trk_seg_rm[duperemoval_config::n_out * num_emtf_sites],
                    trk_seg_v_t trk_seg_rm_v[duperemoval_config::n_out],
                    trk_feat_t trk_feat_rm[duperemoval_config::n_out * num_emtf_features],
                    trk_valid_t trk_valid_rm[duperemoval_config::n_out],
                    trk_origin_t trk_origin_rm[duperemoval_config::n_out]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = duperemoval_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

  // Intermediate arrays
  trk_seg_t trk_seg_reduced[duperemoval_config::n_in * num_emtf_sites_rm];
  trk_seg_v_t trk_seg_reduced_v[duperemoval_config::n_in];
  dio_survivor_t survivors[duperemoval_config::n_in];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = trk_seg_reduced complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_reduced_v complete dim = 0
#pragma HLS ARRAY_PARTITION variable = survivors complete dim = 0
  // hls-pragmas end

  duperemoval_preprocess_op(trk_seg, trk_seg_v, trk_seg_reduced, trk_seg_reduced_v);

  duperemoval_find_dupes_op(trk_seg_reduced, trk_seg_reduced_v, survivors);

  duperemoval_remove_dupes_op(trk_seg, trk_seg_v, trk_feat, trk_valid, survivors, trk_seg_rm, trk_seg_rm_v, trk_feat_rm,
                              trk_valid_rm, trk_origin_rm);
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void duperemoval_layer(const trk_seg_t trk_seg[duperemoval_config::n_in * num_emtf_sites],
                       const trk_seg_v_t trk_seg_v[duperemoval_config::n_in],
                       const trk_feat_t trk_feat[duperemoval_config::n_in * num_emtf_features],
                       const trk_valid_t trk_valid[duperemoval_config::n_in],
                       trk_seg_t trk_seg_rm[duperemoval_config::n_out * num_emtf_sites],
                       trk_seg_v_t trk_seg_rm_v[duperemoval_config::n_out],
                       trk_feat_t trk_feat_rm[duperemoval_config::n_out * num_emtf_features],
                       trk_valid_t trk_valid_rm[duperemoval_config::n_out],
                       trk_origin_t trk_origin_rm[duperemoval_config::n_out]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = duperemoval_config::layer_target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
  // hls-pragmas end

  // Check assumptions
  static_assert(duperemoval_config::n_in == num_emtf_tracks, "duperemoval_config::n_in check failed");
  static_assert(duperemoval_config::n_out == num_emtf_tracks, "duperemoval_config::n_out check failed");
  static_assert(num_emtf_sites == 12, "num_emtf_sites must be 12");
  static_assert(num_emtf_sites_rm == 5, "num_emtf_sites_rm must be 5");
  static_assert(dio_survivor_t::width == duperemoval_config::n_in, "dio_survivor_t type check failed");

  duperemoval_op<Zone>(trk_seg, trk_seg_v, trk_feat, trk_valid, trk_seg_rm, trk_seg_rm_v, trk_feat_rm, trk_valid_rm,
                       trk_origin_rm);
}

}  // namespace phase2

}  // namespace emtf_hlslib

#endif  // __EMTF_HLSLIB_DUPEREMOVAL_H__ not defined
