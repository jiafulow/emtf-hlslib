#ifndef __EMTF_HLSLIB_DUPEREMOVAL_H__
#define __EMTF_HLSLIB_DUPEREMOVAL_H__

namespace emtf {

template <typename T=void>
void duperemoval_preprocess_op(
    const trk_seg_t trk_seg[duperemoval_config::n_in * num_emtf_sites],
    const trk_seg_v_t trk_seg_v[duperemoval_config::n_in],
    trk_seg_t trk_seg_rm[duperemoval_config::n_in * num_emtf_sites_rm],
    trk_seg_v_t trk_seg_v_rm[duperemoval_config::n_in]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // Consider 5 sites for duplicate removal: ME1, ME2, ME3, ME4, ME0
  //
  // site (out) | site (in)
  // -----------|-------------------------------------------
  // ME1        | ME1/1, GE1/1, ME1/2, RE1/2
  // ME2        | ME2, GE2/1, RE2/2
  // ME3        | ME3, RE3
  // ME4        | ME4, RE4
  // ME0        | ME0

  // Loop over tracks
  LOOP_TRK_1: for (unsigned itrk = 0; itrk < duperemoval_config::n_in; itrk++) {

#pragma HLS UNROLL

    auto trk_seg_rhs = &(trk_seg[itrk * num_emtf_sites]);
    const trk_seg_v_t& vld_rhs = trk_seg_v[itrk];

    auto trk_seg_lhs = &(trk_seg_rm[itrk * num_emtf_sites_rm]);
    trk_seg_v_t& vld_lhs = trk_seg_v_rm[itrk];

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

    //std::cout << "[DEBUG] trk " << itrk << " seg: [";
    //for (unsigned k = 0; k < num_emtf_sites; k++) {
    //  std::cout << (vld_rhs[k] ? trk_seg_rhs[k] : static_cast<trk_seg_t>(model_config::n_in)) << ", ";
    //}
    //std::cout << "] seg (rm): [";
    //for (unsigned k = 0; k < num_emtf_sites_rm; k++) {
    //  std::cout << (vld_lhs[k] ? trk_seg_lhs[k] : static_cast<trk_seg_t>(model_config::n_in)) << ", ";
    //}
    //std::cout << "]" << std::endl;
  }
}

template <typename T=void>
void duperemoval_set_survivors_op(
    const trk_valid_t trk_valid[duperemoval_config::n_in],
    const trk_seg_t trk_seg_rm[duperemoval_config::n_in * num_emtf_sites_rm],
    const trk_seg_v_t trk_seg_v_rm[duperemoval_config::n_in],
    dio_survivor_t survivors[duperemoval_config::n_out]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // Find duplicates and mark them for removal
  ap_uint<details::ceil_log2<num_emtf_tracks>::value> cnt = 0;  // survivor count

  // Loop over pairs of tracks, and check 5 sites for each pair
  LOOP_TRK_2: for (unsigned i = 0; i < num_emtf_tracks; i++) {

#pragma HLS UNROLL

    bool killed = false;

    LOOP_TRK_2_1: for (unsigned j = 0; j < i; j++) {

#pragma HLS UNROLL

      bool tmp_killed = false;

      LOOP_TRK_2_1_1: for (unsigned k = 0; k < num_emtf_sites_rm; k++) {

#pragma HLS UNROLL

        const bool vld_i = trk_seg_v_rm[i][k];
        const bool vld_j = trk_seg_v_rm[j][k];
        const bool has_shared_seg = (trk_seg_rm[(i * num_emtf_sites_rm) + k] == trk_seg_rm[(j * num_emtf_sites_rm) + k]);
        if (vld_i and vld_j and has_shared_seg) {
          tmp_killed = true;
        }
      }  // end k loop

      if (tmp_killed) {
        killed = true;
      }
    }  // end j loop

    if (i == 0) {

      LOOP_TRK_2_2: for (unsigned j = 0; j < num_emtf_tracks; j++) {

#pragma HLS UNROLL

        survivors[j] = 0;  // init as zero
      }  // end j loop
    }  // end if i == 0

    if (!killed) {
      survivors[cnt][i] = 1;
      cnt++;
    }
  }  // end i loop
}

template <typename T=void>
void duperemoval_set_features_op(
    const trk_feat_t trk_feat[duperemoval_config::n_in * num_emtf_features],
    const dio_survivor_t survivors[duperemoval_config::n_out],
    trk_feat_t trk_feat_rm[duperemoval_config::n_out * num_emtf_features],
    trk_valid_t trk_valid_rm[duperemoval_config::n_out]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  //FIXME: also need to modify trk_seg_rm

  // Multiplex to output
  LOOP_TRK_3: for (unsigned i = 0; i < num_emtf_tracks; i++) {

#pragma HLS UNROLL

    const bool vld_i = (bool) survivors[i];  // OR reduced
    trk_valid_rm[i] = vld_i;
    emtf_assert((survivors[i] & (survivors[i]-1)) == 0);  // survivors[i] should be power of 2

    if (vld_i) {

      LOOP_TRK_3_1: for (unsigned j = i; j < num_emtf_tracks; j++) {

#pragma HLS UNROLL

        // At most one survivor
        const bool vld_j = survivors[i][j];

        if (vld_j) {
          // Copy to output.
          details::copy_n_values<num_emtf_features>(
              &(trk_feat[j * num_emtf_features]), &(trk_feat_rm[i * num_emtf_features])
          );
        }
      }  // end j loop
    } else {
      // Invalid track. Fill with zero.
      details::fill_n_values<num_emtf_features>(
          &(trk_feat_rm[i * num_emtf_features]), static_cast<trk_feat_t>(0)
      );
    }
  }  // end i loop
}

// _____________________________________________________________________________
// Dupe removal op

template <typename Zone>
void duperemoval_op(
    const trk_seg_t   trk_seg      [duperemoval_config::n_in * num_emtf_sites],
    const trk_seg_v_t trk_seg_v    [duperemoval_config::n_in],
    const trk_feat_t  trk_feat     [duperemoval_config::n_in * num_emtf_features],
    const trk_valid_t trk_valid    [duperemoval_config::n_in],
    trk_feat_t        trk_feat_rm  [duperemoval_config::n_out * num_emtf_features],
    trk_valid_t       trk_valid_rm [duperemoval_config::n_out]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  trk_seg_t trk_seg_rm[duperemoval_config::n_in * num_emtf_sites_rm];
  trk_seg_v_t trk_seg_v_rm[duperemoval_config::n_in];
  dio_survivor_t survivors[duperemoval_config::n_out];

#pragma HLS ARRAY_PARTITION variable=trk_seg_rm complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_seg_v_rm complete dim=0
#pragma HLS ARRAY_PARTITION variable=survivors complete dim=0

  duperemoval_preprocess_op(trk_seg, trk_seg_v, trk_seg_rm, trk_seg_v_rm);

  duperemoval_set_survivors_op(trk_valid, trk_seg_rm, trk_seg_v_rm, survivors);

  duperemoval_set_features_op(trk_feat, survivors, trk_feat_rm, trk_valid_rm);
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void duperemoval_layer(
    const trk_seg_t   trk_seg      [duperemoval_config::n_in * num_emtf_sites],
    const trk_seg_v_t trk_seg_v    [duperemoval_config::n_in],
    const trk_feat_t  trk_feat     [duperemoval_config::n_in * num_emtf_features],
    const trk_valid_t trk_valid    [duperemoval_config::n_in],
    trk_feat_t        trk_feat_rm  [duperemoval_config::n_out * num_emtf_features],
    trk_valid_t       trk_valid_rm [duperemoval_config::n_out]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

  // Check assumptions
  static_assert(duperemoval_config::n_in == num_emtf_tracks, "duperemoval_config::n_in check failed");
  static_assert(duperemoval_config::n_out == num_emtf_tracks, "duperemoval_config::n_out check failed");
  static_assert(num_emtf_sites == 12, "num_emtf_sites must be 12");
  static_assert(num_emtf_sites_rm == 5, "num_emtf_sites_rm must be 5");

  duperemoval_op<Zone>(trk_seg, trk_seg_v, trk_feat, trk_valid, trk_feat_rm, trk_valid_rm);
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_DUPEREMOVAL_H__ not defined
