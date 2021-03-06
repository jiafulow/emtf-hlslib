#include "myproject.h"

namespace {
using namespace emtf_hlslib::phase2;
}

namespace detail {
using namespace emtf_hlslib::phase2::detail;
}

// Top-level function implementation
void myproject(const top_in_t in0[TOP_N_IN], top_out_t out[TOP_N_OUT]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = duperemoval_config::layer_target_ii
#pragma HLS INTERFACE ap_vld port = in0
#pragma HLS INTERFACE ap_vld port = out
#pragma HLS ARRAY_RESHAPE variable = in0 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = out complete dim = 0
  // hls-pragmas end

  // Prepare input and output variables
  trk_seg_t trk_seg[trkbuilding_config::n_out * num_emtf_sites];
  trk_seg_v_t trk_seg_v[trkbuilding_config::n_out];
  trk_feat_t trk_feat[trkbuilding_config::n_out * num_emtf_features];
  trk_valid_t trk_valid[trkbuilding_config::n_out];
  trk_seg_t trk_seg_rm[duperemoval_config::n_out * num_emtf_sites];
  trk_seg_v_t trk_seg_rm_v[duperemoval_config::n_out];
  trk_feat_t trk_feat_rm[duperemoval_config::n_out * num_emtf_features];
  trk_valid_t trk_valid_rm[duperemoval_config::n_out];
  trk_origin_t trk_origin_rm[duperemoval_config::n_out];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = trk_seg complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_v complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_feat complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_valid complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_rm complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_rm_v complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_feat_rm complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_valid_rm complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_origin_rm complete dim = 0
  // hls-pragmas end

  // Loop over in0
LOOP_IN0:
  for (unsigned i = 0; i < TOP_N_IN; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned n_in_per_trk = TOP_N_IN / num_emtf_tracks;
    const unsigned itrk = i / n_in_per_trk;
    const unsigned ivar = i % n_in_per_trk;

    auto curr_trk_seg = &(trk_seg[itrk * num_emtf_sites]);
    auto curr_trk_feat = &(trk_feat[itrk * num_emtf_features]);

    if (ivar == 0) {
      trk_valid[itrk] = 0;
    }

    if (ivar < num_emtf_features) {
      curr_trk_feat[ivar] = in0[i];
      if (in0[i] != 0) {
        trk_valid[itrk] = 1;
      }
    } else if (ivar < (num_emtf_features + num_emtf_sites)) {
      const unsigned ivar_1 = (ivar - num_emtf_features);
      const trk_seg_t invalid_marker_trk_seg = model_config::n_in;
      curr_trk_seg[ivar_1] = in0[i];
      trk_seg_v[itrk][ivar_1] = (static_cast<trk_seg_t>(in0[i]) != invalid_marker_trk_seg);
    }
  }  // end loop over in0

  // Call
  duperemoval_layer<m_zone_any_tag>(trk_seg, trk_seg_v, trk_feat, trk_valid, trk_seg_rm, trk_seg_rm_v, trk_feat_rm,
                                    trk_valid_rm, trk_origin_rm);

  // Copy to output: trk_feat_rm, trk_seg_rm
LOOP_OUT:
  for (unsigned i = 0; i < TOP_N_OUT; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned n_out_per_trk = TOP_N_OUT / num_emtf_tracks;
    const unsigned itrk = i / n_out_per_trk;
    const unsigned ivar = i % n_out_per_trk;

    auto curr_trk_seg_rm = &(trk_seg_rm[itrk * num_emtf_sites]);
    auto curr_trk_feat_rm = &(trk_feat_rm[itrk * num_emtf_features]);

    if (ivar < num_emtf_features) {
      out[i] = curr_trk_feat_rm[ivar];
    } else if (ivar < (num_emtf_features + num_emtf_sites)) {
      const unsigned ivar_1 = (ivar - num_emtf_features);
      const trk_seg_t invalid_marker_trk_seg = model_config::n_in;
      out[i] = (trk_seg_rm_v[itrk][ivar_1]) ? curr_trk_seg_rm[ivar_1] : invalid_marker_trk_seg;
    }
  }  // end loop over out
}
