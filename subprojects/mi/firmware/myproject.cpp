#include "myproject.h"

namespace {
  using namespace emtf::phase2;
}

namespace detail {
  using namespace emtf::phase2::detail;
}

// Top-level function implementation
void myproject(
    const top_in_t in0[TOP_N_IN],
    top_out_t out[TOP_N_OUT]
) {

#pragma HLS PIPELINE II=fullyconnect_config::layer_target_ii

#pragma HLS INTERFACE ap_vld port=in0
#pragma HLS INTERFACE ap_vld port=out
//#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS ARRAY_RESHAPE variable=in0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=out complete dim=0

  // Prepare input and output variables
  trk_feat_t  curr_trk_feat_rm [num_emtf_features];

#pragma HLS ARRAY_PARTITION variable=curr_trk_feat_rm complete dim=0

  trk_invpt_t curr_trk_invpt = 0;
  trk_phi_t   curr_trk_phi   = 0;
  trk_eta_t   curr_trk_eta   = 0;
  trk_d0_t    curr_trk_d0    = 0;
  trk_z0_t    curr_trk_z0    = 0;
  trk_beta_t  curr_trk_beta  = 0;

  // Loop over in0
  LOOP_IN0: for (unsigned i = 0; i < TOP_N_IN; i++) {

#pragma HLS UNROLL

    curr_trk_feat_rm[i] = in0[i];
  }  // end loop over in0

  // Call
  fullyconnect_layer<m_zone_any_tag>(
      curr_trk_feat_rm, curr_trk_invpt, curr_trk_phi, curr_trk_eta, curr_trk_d0, curr_trk_z0,
      curr_trk_beta
  );

  // Copy to output: curr_trk_invpt
  {
    out[0] = curr_trk_invpt;
  }
}
