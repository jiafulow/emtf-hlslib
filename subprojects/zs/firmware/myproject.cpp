#include "myproject.h"

namespace {
using namespace emtf::phase2;
}

namespace detail {
using namespace emtf::phase2::detail;
}

// Top-level function implementation
void myproject(const top_in_t in0[TOP_N_IN], top_out_t out[TOP_N_OUT]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = zonesorting_config::layer_target_ii
#pragma HLS INTERFACE ap_vld port = in0
#pragma HLS INTERFACE ap_vld port = out
#pragma HLS ARRAY_RESHAPE variable = in0 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = out complete dim = 0
  // hls-pragmas end

  // Prepare input and output variables
  pooling_out_t pooling_0_out[pooling_config::n_out];
  zonesorting_out_t zonesorting_0_out[zonesorting_config::n_out];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = pooling_0_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = zonesorting_0_out complete dim = 0
  // hls-pragmas end

  // Loop over in0
LOOP_IN0:
  for (unsigned i = 0; i < TOP_N_IN; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    pooling_0_out[i] = in0[i];
  }  // end loop over in0

  // Call
  zonesorting_layer<m_zone_any_tag>(pooling_0_out, zonesorting_0_out);

  // Copy to output
LOOP_OUT:
  for (unsigned i = 0; i < TOP_N_OUT; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    out[i] = zonesorting_0_out[i];
  }  // end loop over out
}
