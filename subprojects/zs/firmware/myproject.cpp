#include "myproject.h"

using namespace emtf;

// Top-level function implementation
void myproject(
    const top_in_t in0[TOP_N_IN],
    top_out_t out[TOP_N_OUT]
) {

#pragma HLS PIPELINE II=zonesorting_config::layer_target_ii

#pragma HLS INTERFACE ap_vld port=in0
#pragma HLS INTERFACE ap_vld port=out
//#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS ARRAY_RESHAPE variable=in0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=out complete dim=0

  // Prepare input and output variables
  pooling_out_t     pooling_0_out     [pooling_config::n_out];
  zonesorting_out_t zonesorting_0_out [zonesorting_config::n_out];

#pragma HLS ARRAY_PARTITION variable=pooling_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonesorting_0_out complete dim=0

  // Loop over in0
  LOOP_IN0: for (unsigned i = 0; i < TOP_N_IN; i++) {

#pragma HLS UNROLL

    pooling_0_out[i] = in0[i];
  }  // end loop over in0

  // Call
  zonesorting_layer<m_zone_any_tag>(pooling_0_out, zonesorting_0_out);

  // Copy to output
  LOOP_OUT: for (unsigned i = 0; i < TOP_N_OUT; i++) {

#pragma HLS UNROLL

    out[i] = zonesorting_0_out[i];
  }  // end loop over out
}
