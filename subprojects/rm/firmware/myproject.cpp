#include "myproject.h"

using namespace emtf;

// Top-level function implementation
void myproject(
    const top_in_t in0[TOP_N_IN],
    top_out_t out[TOP_N_OUT]
) {

#pragma HLS PIPELINE II=duperemoval_config::target_ii

#pragma HLS INTERFACE ap_vld port=in0
#pragma HLS INTERFACE ap_vld port=out
//#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS ARRAY_RESHAPE variable=in0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=out complete dim=0

  // Prepare input and output variables
  trk_seg_t   trk_seg      [trkbuilding_config::n_out * num_emtf_sites];
  trk_seg_v_t trk_seg_v    [trkbuilding_config::n_out];
  trk_feat_t  trk_feat     [trkbuilding_config::n_out * num_emtf_features];
  trk_valid_t trk_valid    [trkbuilding_config::n_out];
  trk_seg_t   trk_seg_rm   [duperemoval_config::n_out * num_emtf_sites];
  trk_seg_v_t trk_seg_rm_v [duperemoval_config::n_out];
  trk_feat_t  trk_feat_rm  [duperemoval_config::n_out * num_emtf_features];
  trk_valid_t trk_valid_rm [duperemoval_config::n_out];

#pragma HLS ARRAY_PARTITION variable=trk_seg complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_seg_v complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_feat complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_valid complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_seg_rm complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_seg_rm_v complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_feat_rm complete dim=0
#pragma HLS ARRAY_PARTITION variable=trk_valid_rm complete dim=0

  // Loop over in0
  LOOP_IN0: for (unsigned i = 0; i < TOP_N_IN; i++) {

#pragma HLS UNROLL

    const unsigned n_in_per_trk = TOP_N_IN / num_emtf_tracks;
    const unsigned itrk = i / n_in_per_trk;
    const unsigned ivar = i % n_in_per_trk;

    auto curr_trk_seg = &(trk_seg[itrk * num_emtf_sites]);
    auto curr_trk_feat = &(trk_feat[itrk * num_emtf_features]);

    const trk_seg_t invalid_marker_ph_seg = model_config::n_in;

    if (ivar == 0) {
      trk_valid[itrk] = 0;
    }

    if (ivar < num_emtf_features) {
      curr_trk_feat[ivar] = in0[i];
      if (in0[i] > 0) {
        trk_valid[itrk] = 1;
      }
    } else if (ivar < (num_emtf_features + num_emtf_sites)) {
      const unsigned ivar_1 = (ivar - num_emtf_features);
      curr_trk_seg[ivar_1] = in0[i];
      trk_seg_v[itrk][ivar_1] = (
          (static_cast<trk_seg_t>(in0[i]) != invalid_marker_ph_seg) ?
          static_cast<bool_t>(1) : static_cast<bool_t>(0)
      );
    }
  }  // end loop over in0

  // Call
  duperemoval_layer<m_zone_any_tag>(
      trk_seg, trk_seg_v, trk_feat, trk_valid, trk_seg_rm, trk_seg_rm_v, trk_feat_rm, trk_valid_rm
  );

  // Copy to output: trk_feat_rm, trk_seg_rm
  LOOP_OUT: for (unsigned i = 0; i < TOP_N_OUT; i++) {

#pragma HLS UNROLL

    const unsigned n_out_per_trk = TOP_N_OUT / num_emtf_tracks;
    const unsigned itrk = i / n_out_per_trk;
    const unsigned ivar = i % n_out_per_trk;

    const auto curr_trk_seg_rm = &(trk_seg_rm[itrk * num_emtf_sites]);
    const auto curr_trk_feat_rm = &(trk_feat_rm[itrk * num_emtf_features]);

    const trk_seg_t invalid_marker_ph_seg = model_config::n_in;

    if (ivar < num_emtf_features) {
      out[i] = curr_trk_feat_rm[ivar];
    } else if (ivar < (num_emtf_features + num_emtf_sites)) {
      const unsigned ivar_1 = (ivar - num_emtf_features);
      out[i] = (trk_seg_rm_v[itrk][ivar_1]) ? curr_trk_seg_rm[ivar_1] : invalid_marker_ph_seg;
    }
  }  // end loop over out
}
