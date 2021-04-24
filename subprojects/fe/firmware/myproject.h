#ifndef __MYPROJECT_H__
#define __MYPROJECT_H__

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"
//#include "hls_stream.h"

// EMTF HLS
#include "emtf_hlslib.h"

// Defines
typedef emtf::model_in_t top_in0_t;
typedef emtf::trk_feat_t top_in1_t;
typedef emtf::trk_feat_t top_out_t;
constexpr int TOP_N_IN0 = emtf::model_config::n_in;
constexpr int TOP_N_IN1 = 4;  // trk_qual, trk_patt, trk_col, trk_zone
constexpr int TOP_N_OUT = (emtf::num_emtf_features + emtf::num_emtf_sites);

// Top-level function prototype
void myproject(
    const top_in0_t in0[TOP_N_IN0],
    const top_in1_t in1[TOP_N_IN1],
    top_out_t out[TOP_N_OUT]
);

#endif  // __MYPROJECT_H__ not defined
