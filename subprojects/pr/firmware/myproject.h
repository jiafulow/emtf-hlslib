#ifndef __MYPROJECT_H__
#define __MYPROJECT_H__

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"
//#include "hls_stream.h"

// EMTF HLS
#include "emtf_hlslib.h"

// Defines
typedef emtf::model_in_t top_in_t;
typedef emtf::trk_feat_t top_out_t;
constexpr int TOP_N_IN = emtf::model_config::n_in;
constexpr int TOP_N_OUT = emtf::num_emtf_tracks * 4;  // trk_qual, trk_patt, trk_col, trk_zone

// Top-level function prototype
void myproject(
    const top_in_t in0[TOP_N_IN],
    top_out_t out[TOP_N_OUT]
);

#endif  // __MYPROJECT_H__ not defined
