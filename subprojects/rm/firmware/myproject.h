#ifndef __MYPROJECT_H__
#define __MYPROJECT_H__

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"
//#include "hls_stream.h"

// EMTF HLS
#include "emtf_hlslib.h"

// Defines
typedef emtf_hlslib::phase2::trk_feat_t top_in_t;
typedef top_in_t top_out_t;
constexpr int TOP_N_IN = emtf_hlslib::phase2::num_emtf_tracks *
                         (emtf_hlslib::phase2::num_emtf_features + emtf_hlslib::phase2::num_emtf_sites);
constexpr int TOP_N_OUT = TOP_N_IN;

// Top-level function prototype
void myproject(const top_in_t in0[TOP_N_IN], top_out_t out[TOP_N_OUT]);

#endif  // __MYPROJECT_H__ not defined
