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

#pragma HLS PIPELINE II=zonemerging_config::layer_target_ii

#pragma HLS INTERFACE ap_vld port=in0
#pragma HLS INTERFACE ap_vld port=out
//#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS ARRAY_RESHAPE variable=in0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=out complete dim=0

  // Prepare input and output variables
  zonesorting_out_t zonesorting_0_out [zonesorting_config::n_out];
  zonesorting_out_t zonesorting_1_out [zonesorting_config::n_out];
  zonesorting_out_t zonesorting_2_out [zonesorting_config::n_out];
  zonemerging_out_t zonemerging_0_out [zonemerging_config::n_out];

#pragma HLS ARRAY_PARTITION variable=zonesorting_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonesorting_1_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonesorting_2_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonemerging_0_out complete dim=0

  // Loop over in0
  LOOP_IN0: for (unsigned i = 0; i < TOP_N_IN; i++) {

#pragma HLS UNROLL

    const unsigned n_in_per_zone = TOP_N_IN / num_emtf_zones;
    if ((i / n_in_per_zone) == 0) {
      zonesorting_0_out[(i % n_in_per_zone)] = in0[i];
    } else if ((i / n_in_per_zone) == 1) {
      zonesorting_1_out[(i % n_in_per_zone)] = in0[i];
    } else if ((i / n_in_per_zone) == 2) {
      zonesorting_2_out[(i % n_in_per_zone)] = in0[i];
    }
  }  // end loop over in0

  // Call
  zonemerging_layer<m_zone_any_tag>(
      zonesorting_0_out, zonesorting_1_out, zonesorting_2_out, zonemerging_0_out
  );

  // Copy to output
  LOOP_OUT: for (unsigned i = 0; i < TOP_N_OUT; i++) {

#pragma HLS UNROLL

    out[i] = zonemerging_0_out[i];
  }  // end loop over out
}
