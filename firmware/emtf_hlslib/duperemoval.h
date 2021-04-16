#ifndef __EMTF_HLSLIB_DUPEREMOVAL_H__
#define __EMTF_HLSLIB_DUPEREMOVAL_H__

namespace emtf {

template <typename T=void>
void duperemoval_preprocess_op(
    const trk_seg_t   trk_seg           [duperemoval_config::n_in * num_emtf_sites],
    const trk_seg_v_t trk_seg_v         [duperemoval_config::n_in],
    trk_seg_t         trk_seg_reduced   [duperemoval_config::n_in * num_emtf_sites_rm],
    trk_seg_v_t       trk_seg_reduced_v [duperemoval_config::n_in]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  // Reduce to 5 sites for duplicate removal: ME1, ME2, ME3, ME4, ME0
  //
  // site (out) | site (in)
  // -----------|-------------------------------------------
  // ME1        | ME1/1, GE1/1, ME1/2, RE1/2
  // ME2        | ME2, GE2/1, RE2/2
  // ME3        | ME3, RE3
  // ME4        | ME4, RE4
  // ME0        | ME0

  // Loop over tracks
  LOOP_TRK_1: for (unsigned i = 0; i < duperemoval_config::n_in; i++) {

#pragma HLS UNROLL

    const auto trk_seg_rhs = &(trk_seg[i * num_emtf_sites]);
    const trk_seg_v_t& vld_rhs = trk_seg_v[i];

    auto trk_seg_lhs = &(trk_seg_reduced[i * num_emtf_sites_rm]);
    trk_seg_v_t& vld_lhs = trk_seg_reduced_v[i];

    // Priority encoder (input at lower index has higher priority)
    //
    //  0,  9,  1,  5 -> 0
    //  2, 10,  6     -> 1
    //  3,  7         -> 2
    //  4,  8         -> 3
    // 11             -> 4

    // The following is basically a loop over trk_seg and trk_seg_v
    trk_seg_lhs[0] = vld_rhs[0] ? trk_seg_rhs[0] : (vld_rhs[9] ? trk_seg_rhs[9] : (vld_rhs[1] ? trk_seg_rhs[1] : trk_seg_rhs[5]));
    trk_seg_lhs[1] = vld_rhs[2] ? trk_seg_rhs[2] : (vld_rhs[10] ? trk_seg_rhs[10] : trk_seg_rhs[6]);
    trk_seg_lhs[2] = vld_rhs[3] ? trk_seg_rhs[3] : trk_seg_rhs[7];
    trk_seg_lhs[3] = vld_rhs[4] ? trk_seg_rhs[4] : trk_seg_rhs[8];
    trk_seg_lhs[4] = trk_seg_rhs[11];

    vld_lhs[0] = vld_rhs[0] | vld_rhs[9] | vld_rhs[1] | vld_rhs[5];
    vld_lhs[1] = vld_rhs[2] | vld_rhs[10] | vld_rhs[6];
    vld_lhs[2] = vld_rhs[3] | vld_rhs[7];
    vld_lhs[3] = vld_rhs[4] | vld_rhs[8];
    vld_lhs[4] = vld_rhs[11];

    //std::cout << "[DEBUG] trk " << i << " sites (in): [";
    //const trk_seg_t invalid_marker_ph_seg = model_config::n_in;
    //for (unsigned j = 0; j < num_emtf_sites; j++) {
    //  std::cout << (vld_rhs[j] ? trk_seg_rhs[j] : invalid_marker_ph_seg) << ", ";
    //}
    //std::cout << "] sites (out): [";
    //for (unsigned j = 0; j < num_emtf_sites_rm; j++) {
    //  std::cout << (vld_lhs[j] ? trk_seg_lhs[j] : invalid_marker_ph_seg) << ", ";
    //}
    //std::cout << "]" << std::endl;
  }
}

template <typename T=void>
void duperemoval_find_dupes_op(
    const trk_seg_t   trk_seg_reduced   [duperemoval_config::n_in * num_emtf_sites_rm],
    const trk_seg_v_t trk_seg_reduced_v [duperemoval_config::n_in],
    dio_survivor_t    survivors         [duperemoval_config::n_in]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  // Find duplicates by checking pairs of tracks and their associated segments

  // survivors_tmp is used to keep the not-duplicate tracks
  dio_survivor_t survivors_tmp[duperemoval_config::n_in];

#pragma HLS ARRAY_PARTITION variable=survivors_tmp complete dim=0

  typedef ap_uint<details::ceil_log2<duperemoval_config::n_in>::value> idx_t;
  idx_t cnt = 0;  // survivor count

  LOOP_TRK_2: for (unsigned i = 0; i < duperemoval_config::n_in; i++) {

#pragma HLS UNROLL

    survivors_tmp[i] = 0;  // init as zero
  }  // end i loop

  // trk 0 is not a duplicate by construction
  survivors_tmp[cnt][0] = static_cast<bool_t>(1);
  cnt++;

  // Loop over pairs of tracks (trk 0 is skipped), and check all 5 sites for each pair
  LOOP_TRK_3: for (unsigned i = 1; i < duperemoval_config::n_in; i++) {

#pragma HLS UNROLL

    // Kill the track if it is a duplicate
    bool_t killed = 0;

    LOOP_TRK_3_1: for (unsigned j = 0; j < i; j++) {

#pragma HLS UNROLL

      bool_t killed_tmp = 0;

      LOOP_TRK_3_1_1: for (unsigned k = 0; k < num_emtf_sites_rm; k++) {

#pragma HLS UNROLL

        const bool_t vld_i = trk_seg_reduced_v[i][k];
        const bool_t vld_j = trk_seg_reduced_v[j][k];
        const bool_t has_shared_seg = (
            (trk_seg_reduced[(i * num_emtf_sites_rm) + k] == trk_seg_reduced[(j * num_emtf_sites_rm) + k]) ?
            static_cast<bool_t>(1) : static_cast<bool_t>(0)
        );
        killed_tmp |= (vld_i & vld_j & has_shared_seg);  // logical OR over all 5 sites
      }  // end k loop

      killed |= killed_tmp;  // logical OR over all tracks (j < i)
    }  // end j loop

    // Survive if not a duplicate
    if (!killed) {
      survivors_tmp[cnt][i] = static_cast<bool_t>(1);
      cnt++;
    }
  }  // end i loop

  // Copy survivors_tmp to survivors
  LOOP_TRK_4: for (unsigned i = 0; i < duperemoval_config::n_in; i++) {

#pragma HLS UNROLL

    survivors[i] = survivors_tmp[i];
  }  // end i loop

  //std::cout << "[DEBUG] trk_seg_reduced_v: " << std::endl;
  //for (unsigned i = 0; i < duperemoval_config::n_in; i++) {
  //  for (unsigned j = 0; j < num_emtf_sites_rm; j++) {
  //    std::cout << trk_seg_reduced_v[i][j] << " ";
  //  }
  //  std::cout << std::endl;
  //}
  //std::cout << "[DEBUG] survivors: " << std::endl;
  //for (unsigned i = 0; i < duperemoval_config::n_in; i++) {
  //  for (unsigned j = 0; j < duperemoval_config::n_in; j++) {
  //    std::cout << survivors[i][j] << " ";
  //  }
  //  std::cout << std::endl;
  //}
}

template <typename T=void>
void duperemoval_remove_dupes_op(
    const trk_seg_t      trk_seg      [duperemoval_config::n_in * num_emtf_sites],
    const trk_seg_v_t    trk_seg_v    [duperemoval_config::n_in],
    const trk_feat_t     trk_feat     [duperemoval_config::n_in * num_emtf_features],
    const trk_valid_t    trk_valid    [duperemoval_config::n_in],
    const dio_survivor_t survivors    [duperemoval_config::n_in],
    trk_seg_t            trk_seg_rm   [duperemoval_config::n_out * num_emtf_sites],
    trk_seg_v_t          trk_seg_rm_v [duperemoval_config::n_out],
    trk_feat_t           trk_feat_rm  [duperemoval_config::n_out * num_emtf_features],
    trk_valid_t          trk_valid_rm [duperemoval_config::n_out]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  // Remove duplicates by saving only the survivors
  const trk_seg_t invalid_marker_ph_seg = model_config::n_in;

  // Multiplex to output
  LOOP_TRK_5: for (unsigned i = 0; i < duperemoval_config::n_out; i++) {

#pragma HLS UNROLL

    // Fill with default values
    trk_valid_rm[i] = static_cast<trk_valid_t>(0);
    trk_seg_rm_v[i] = static_cast<trk_seg_v_t>(0);
    details::fill_n_values<num_emtf_sites>(
        &(trk_seg_rm[i * num_emtf_sites]), invalid_marker_ph_seg
    );
    details::fill_n_values<num_emtf_features>(
        &(trk_feat_rm[i * num_emtf_features]), static_cast<trk_feat_t>(0)
    );

    // Loop over possible tracks (j >= i)
    LOOP_TRK_5_1: for (unsigned j = i; j < duperemoval_config::n_in; j++) {

#pragma HLS UNROLL

      // trk 0 is a special case where i == j by construction
      if ((i == 0) and (j != 0))
        continue;

      // Should be at most one survivor
      const bool_t survived = survivors[i][j];

      if (survived) {
        trk_valid_rm[i] = trk_valid[j];
        trk_seg_rm_v[i] = trk_seg_v[j];
        details::copy_n_values<num_emtf_sites>(
            &(trk_seg[j * num_emtf_sites]), &(trk_seg_rm[i * num_emtf_sites])
        );
        details::copy_n_values<num_emtf_features>(
            &(trk_feat[j * num_emtf_features]), &(trk_feat_rm[i * num_emtf_features])
        );
      }
    }  // end j loop
  }  // end i loop

  // Sanity check
#ifndef __SYNTHESIS__
  auto is_power_of_two = [](unsigned x) -> bool {
    return ((x & (x - 1)) == 0);
  };
  for (unsigned i = 0; i < duperemoval_config::n_out; i++) {
    emtf_assert(is_power_of_two(survivors[i]));  // make sure is either zero or power of 2
  }
#endif

}

// _____________________________________________________________________________
// Duplicate removal op

template <typename Zone>
void duperemoval_op(
    const trk_seg_t   trk_seg      [duperemoval_config::n_in * num_emtf_sites],
    const trk_seg_v_t trk_seg_v    [duperemoval_config::n_in],
    const trk_feat_t  trk_feat     [duperemoval_config::n_in * num_emtf_features],
    const trk_valid_t trk_valid    [duperemoval_config::n_in],
    trk_seg_t         trk_seg_rm   [duperemoval_config::n_out * num_emtf_sites],
    trk_seg_v_t       trk_seg_rm_v [duperemoval_config::n_out],
    trk_feat_t        trk_feat_rm  [duperemoval_config::n_out * num_emtf_features],
    trk_valid_t       trk_valid_rm [duperemoval_config::n_out]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  trk_seg_t trk_seg_reduced[duperemoval_config::n_in * num_emtf_sites_rm];
  trk_seg_v_t trk_seg_reduced_v[duperemoval_config::n_in];
  dio_survivor_t survivors[duperemoval_config::n_in];

#pragma HLS ARRAY_PARTITION variable=trk_seg_reduced complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_seg_reduced_v complete dim=0
#pragma HLS ARRAY_PARTITION variable=survivors complete dim=0

  duperemoval_preprocess_op(trk_seg, trk_seg_v, trk_seg_reduced, trk_seg_reduced_v);

  duperemoval_find_dupes_op(trk_seg_reduced, trk_seg_reduced_v, survivors);

  duperemoval_remove_dupes_op(
      trk_seg, trk_seg_v, trk_feat, trk_valid, survivors, trk_seg_rm, trk_seg_rm_v, trk_feat_rm, trk_valid_rm
  );
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void duperemoval_layer(
    const trk_seg_t   trk_seg      [duperemoval_config::n_in * num_emtf_sites],
    const trk_seg_v_t trk_seg_v    [duperemoval_config::n_in],
    const trk_feat_t  trk_feat     [duperemoval_config::n_in * num_emtf_features],
    const trk_valid_t trk_valid    [duperemoval_config::n_in],
    trk_seg_t         trk_seg_rm   [duperemoval_config::n_out * num_emtf_sites],
    trk_seg_v_t       trk_seg_rm_v [duperemoval_config::n_out],
    trk_feat_t        trk_feat_rm  [duperemoval_config::n_out * num_emtf_features],
    trk_valid_t       trk_valid_rm [duperemoval_config::n_out]
) {

#pragma HLS PIPELINE II=duperemoval_config::layer_target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

  // Check assumptions
  static_assert(duperemoval_config::n_in == num_emtf_tracks, "duperemoval_config::n_in check failed");
  static_assert(duperemoval_config::n_out == num_emtf_tracks, "duperemoval_config::n_out check failed");
  static_assert(num_emtf_sites == 12, "num_emtf_sites must be 12");
  static_assert(num_emtf_sites_rm == 5, "num_emtf_sites_rm must be 5");

  duperemoval_op<Zone>(
      trk_seg, trk_seg_v, trk_feat, trk_valid, trk_seg_rm, trk_seg_rm_v, trk_feat_rm, trk_valid_rm
  );
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_DUPEREMOVAL_H__ not defined
