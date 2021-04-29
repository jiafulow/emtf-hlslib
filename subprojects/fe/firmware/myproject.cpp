#include "myproject.h"

using namespace emtf;

// Top-level function implementation
void myproject(
    const top_in0_t in0[TOP_N_IN0],
    const top_in1_t in1[TOP_N_IN1],
    top_out_t out[TOP_N_OUT]
) {

#pragma HLS PIPELINE II=trkbuilding_config::target_ii

#pragma HLS INTERFACE ap_vld port=in0
#pragma HLS INTERFACE ap_vld port=in1
#pragma HLS INTERFACE ap_vld port=out
//#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS ARRAY_RESHAPE variable=in0 complete dim=0
#pragma HLS ARRAY_RESHAPE variable=in1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=out complete dim=0

  // Unpack from in0
  // Note: the following are currently unused and will be synthesized away
  // - emtf_qual2, emtf_time, seg_fr, seg_dl, seg_bx
  emtf_phi_t    emtf_phi    [model_config::n_in];
  emtf_bend_t   emtf_bend   [model_config::n_in];
  emtf_theta1_t emtf_theta1 [model_config::n_in];
  emtf_theta2_t emtf_theta2 [model_config::n_in];
  emtf_qual1_t  emtf_qual1  [model_config::n_in];
  emtf_qual2_t  emtf_qual2  [model_config::n_in];
  emtf_time_t   emtf_time   [model_config::n_in];
  seg_zones_t   seg_zones   [model_config::n_in];
  seg_tzones_t  seg_tzones  [model_config::n_in];
  seg_fr_t      seg_fr      [model_config::n_in];
  seg_dl_t      seg_dl      [model_config::n_in];
  seg_bx_t      seg_bx      [model_config::n_in];
  seg_valid_t   seg_valid   [model_config::n_in];

#pragma HLS ARRAY_PARTITION variable=emtf_phi complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_bend complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_theta1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_theta2 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_qual1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_qual2 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_time complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_zones complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_tzones complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_fr complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_dl complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_bx complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_valid complete dim=0

  // Loop over in0
  LOOP_IN0: for (unsigned iseg = 0; iseg < model_config::n_in; iseg++) {

#pragma HLS UNROLL

    emtf_phi[iseg]    = in0[iseg].emtf_phi;
    emtf_bend[iseg]   = in0[iseg].emtf_bend;
    emtf_theta1[iseg] = in0[iseg].emtf_theta1;
    emtf_theta2[iseg] = in0[iseg].emtf_theta2;
    emtf_qual1[iseg]  = in0[iseg].emtf_qual1;
    emtf_qual2[iseg]  = in0[iseg].emtf_qual2;
    emtf_time[iseg]   = in0[iseg].emtf_time;
    seg_zones[iseg]   = in0[iseg].seg_zones;
    seg_tzones[iseg]  = in0[iseg].seg_tzones;
    seg_fr[iseg]      = in0[iseg].seg_fr;
    seg_dl[iseg]      = in0[iseg].seg_dl;
    seg_bx[iseg]      = in0[iseg].seg_bx;
    seg_valid[iseg]   = in0[iseg].seg_valid;
  }  // end loop over in0

  // Check assumptions
  static_assert(TOP_N_IN1 == 4, "TOP_N_IN1 check failed");

  // Prepare input and output variables
  trk_feat_t  curr_trk_feat [num_emtf_features];
  trk_seg_t   curr_trk_seg  [num_emtf_sites];

#pragma HLS ARRAY_PARTITION variable=curr_trk_feat complete dim=0
#pragma HLS ARRAY_PARTITION variable=curr_trk_seg complete dim=0

  trk_qual_t  curr_trk_qual  = in1[0];
  trk_patt_t  curr_trk_patt  = in1[1];
  trk_col_t   curr_trk_col   = in1[2];
  trk_zone_t  curr_trk_zone  = in1[3];
  trk_tzone_t curr_trk_tzone = details::timezone_traits<m_timezone_0_tag>::value;  // default timezone
  trk_seg_v_t curr_trk_seg_v = 0;
  trk_valid_t curr_trk_valid = 0;

  // Call
  trkbuilding_op<m_zone_any_tag>(
      emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, emtf_qual2,
      emtf_time, seg_zones, seg_tzones, seg_fr, seg_dl, seg_bx,
      seg_valid, curr_trk_qual, curr_trk_patt, curr_trk_col, curr_trk_zone, curr_trk_tzone,
      curr_trk_seg, curr_trk_seg_v, curr_trk_feat, curr_trk_valid
  );

  // Copy to output: curr_trk_feat, curr_trk_seg
  LOOP_OUT: for (unsigned i = 0; i < TOP_N_OUT; i++) {

#pragma HLS UNROLL

    const unsigned n_out_per_trk = TOP_N_OUT;
    const unsigned itrk = i / n_out_per_trk;
    const unsigned ivar = i % n_out_per_trk;
    emtf_unused(itrk);

    const trk_seg_t invalid_marker_ph_seg = model_config::n_in;

    if (ivar < num_emtf_features) {
      out[i] = curr_trk_feat[ivar];
    } else if (ivar < (num_emtf_features + num_emtf_sites)) {
      const unsigned ivar_1 = (ivar - num_emtf_features);
      out[i] = (curr_trk_seg_v[ivar_1]) ? curr_trk_seg[ivar_1] : invalid_marker_ph_seg;
    }
  }  // end loop over out
}
