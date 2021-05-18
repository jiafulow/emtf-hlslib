#ifndef __MYPROJECT_H__
#define __MYPROJECT_H__

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"
//#include "hls_stream.h"

// EMTF HLS
#include "emtf_hlslib.h"

// Defines
typedef emtf::zonesorting_out_t top_in_t;
typedef emtf::zonemerging_out_t top_out_t;
constexpr int TOP_N_IN = emtf::zonesorting_config::n_out * emtf::num_emtf_zones;
constexpr int TOP_N_OUT = emtf::zonemerging_config::n_out;

// Top-level function prototype
void myproject(
    const top_in_t in0[TOP_N_IN],
    top_out_t out[TOP_N_OUT]
);

#endif  // __MYPROJECT_H__ not defined
