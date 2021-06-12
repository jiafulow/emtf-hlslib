#ifndef __EMTF_HLSLIB_COPY_KERNELS_H__
#define __EMTF_HLSLIB_COPY_KERNELS_H__

#include <algorithm>  // provides std::copy, std::fill

// EMTF HLS
#include "layer_helpers.h"

namespace emtf {

namespace phase2 {

namespace detail {

template <unsigned int N, typename T>
void copy_n_values(const T in0[N], T out[N]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

#ifndef __SYNTHESIS__
  std::copy(in0, in0 + N, out);
#else
LOOP_COPY:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    out[i] = in0[i];
  }
#endif  // __SYNTHESIS__ not defined
}

template <unsigned int N, typename T>
void fill_n_values(T out[N], const T& value) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = trkbuilding_config::target_ii
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE
  // hls-pragmas end

#ifndef __SYNTHESIS__
  std::fill(out, out + N, value);
#else
LOOP_FILL:
  for (unsigned i = 0; i < N; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    out[i] = value;
  }
#endif  // __SYNTHESIS__ not defined
}

}  // namespace detail

}  // namespace phase2

}  // namespace emtf

#endif  // __EMTF_HLSLIB_COPY_KERNELS_H__ not defined
