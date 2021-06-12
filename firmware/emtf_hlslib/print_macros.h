#ifndef __EMTF_HLSLIB_PRINT_MACROS_H__
#define __EMTF_HLSLIB_PRINT_MACROS_H__

#if !defined(__SYNTHESIS__) && !defined(NDEBUG)
#include <iostream>
#include <sstream>
#endif  // __SYNTHESIS__ and NDEBUG not defined

#if !defined(__SYNTHESIS__) && !defined(NDEBUG)
#define PRINT_TOP_FN_ARRAYS_IN0                                                                                       \
  std::cout << "Printing 'in0' arrays:" << std::endl;                                                                 \
  for (unsigned iseg = 0; iseg < model_config::n_in; iseg++) {                                                        \
    if (seg_valid[iseg] == 1) {                                                                                       \
      std::cout << (iseg / num_emtf_segments) << " " << (iseg % num_emtf_segments) << " " << emtf_phi[iseg] << " "    \
                << emtf_bend[iseg] << " " << emtf_theta1[iseg] << " " << emtf_theta2[iseg] << " " << emtf_qual1[iseg] \
                << " " << emtf_qual2[iseg] << " " << emtf_time[iseg] << " " << seg_zones[iseg] << " "                 \
                << seg_tzones[iseg] << " " << seg_fr[iseg] << " " << seg_dl[iseg] << " " << seg_bx[iseg] << " "       \
                << seg_valid[iseg] << " " << std::endl;                                                               \
    }                                                                                                                 \
  }
#else
#define PRINT_TOP_FN_ARRAYS_IN0
#endif  // __SYNTHESIS__ and NDEBUG not defined

#if !defined(__SYNTHESIS__) && !defined(NDEBUG)
#define PRINT_TOP_FN_ARRAYS_OTHER                                                                                   \
  std::cout << "Printing 'seg_valid' non-zero indices:" << std::endl;                                               \
  std::cout << "iseg: [";                                                                                           \
  for (unsigned iseg = 0; iseg < model_config::n_in; iseg++) {                                                      \
    if (seg_valid[iseg] == 1) {                                                                                     \
      std::cout << iseg << ", ";                                                                                    \
    }                                                                                                               \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'zoning_0_out' non-zero elements:" << std::endl;                                           \
  std::cout << "row: [";                                                                                            \
  for (unsigned i = 0; i < zoning_config::n_out; i++) {                                                             \
    for (unsigned j = 0; j < zoning_out_t::width; j++) {                                                            \
      if (zoning_0_out[i].test(j))                                                                                  \
        std::cout << i << ", ";                                                                                     \
    }                                                                                                               \
  }                                                                                                                 \
  std::cout << "] col: [";                                                                                          \
  for (unsigned i = 0; i < zoning_config::n_out; i++) {                                                             \
    for (unsigned j = 0; j < zoning_out_t::width; j++) {                                                            \
      if (zoning_0_out[i].test(j))                                                                                  \
        std::cout << j << ", ";                                                                                     \
    }                                                                                                               \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'pooling_0_out' non-zero elements:" << std::endl;                                          \
  std::cout << "col: [";                                                                                            \
  for (unsigned i = 0; i < pooling_config::n_out; i++) {                                                            \
    if (pooling_0_out[i])                                                                                           \
      std::cout << i << ", ";                                                                                       \
  }                                                                                                                 \
  std::cout << "] value: [";                                                                                        \
  for (unsigned i = 0; i < pooling_config::n_out; i++) {                                                            \
    if (pooling_0_out[i])                                                                                           \
      std::cout << pooling_0_out[i].range(trk_qual_t::width - 1, 0) << ", ";                                        \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'zonesorting_0_out', 'zonesorting_1_out', 'zonesorting_2_out' all elements:" << std::endl; \
  std::cout << "value: [";                                                                                          \
  for (unsigned i = 0; i < zonesorting_config::n_out; i++) {                                                        \
    std::cout << zonesorting_0_out[i].range(trk_qual_t::width - 1, 0) << ", ";                                      \
  }                                                                                                                 \
  std::cout << "] value: [";                                                                                        \
  for (unsigned i = 0; i < zonesorting_config::n_out; i++) {                                                        \
    std::cout << zonesorting_1_out[i].range(trk_qual_t::width - 1, 0) << ", ";                                      \
  }                                                                                                                 \
  std::cout << "] value: [";                                                                                        \
  for (unsigned i = 0; i < zonesorting_config::n_out; i++) {                                                        \
    std::cout << zonesorting_2_out[i].range(trk_qual_t::width - 1, 0) << ", ";                                      \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'zonemerging_0_out' all elements:" << std::endl;                                           \
  std::cout << "value: [";                                                                                          \
  for (unsigned i = 0; i < zonemerging_config::n_out; i++) {                                                        \
    std::cout << zonemerging_0_out[i].range(trk_qual_t::width - 1, 0) << ", ";                                      \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'trk_qual', 'trk_patt', 'trk_col', 'trk_zone', 'trk_tzone' all elements:" << std::endl;    \
  std::cout << "trk_qual: [";                                                                                       \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                         \
    std::cout << trk_qual[i] << ", ";                                                                               \
  }                                                                                                                 \
  std::cout << "] trk_patt: [";                                                                                     \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                         \
    std::cout << trk_patt[i] << ", ";                                                                               \
  }                                                                                                                 \
  std::cout << "] trk_col: [";                                                                                      \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                         \
    std::cout << trk_col[i] << ", ";                                                                                \
  }                                                                                                                 \
  std::cout << "] trk_zone: [";                                                                                     \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                         \
    std::cout << trk_zone[i] << ", ";                                                                               \
  }                                                                                                                 \
  std::cout << "] trk_tzone: [";                                                                                    \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                         \
    std::cout << trk_tzone[i] << ", ";                                                                              \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'trk_seg', 'trk_seg_v' first-track elements:" << std::endl;                                \
  std::cout << "trk_seg: [";                                                                                        \
  for (unsigned i = 0; i < num_emtf_sites; i++) {                                                                   \
    std::cout << trk_seg[i] << ", ";                                                                                \
  }                                                                                                                 \
  std::cout << "] trk_seg_v: [";                                                                                    \
  for (unsigned i = 0; i < num_emtf_sites; i++) {                                                                   \
    std::cout << trk_seg_v[0][i] << ", ";                                                                           \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'trk_seg', 'trk_seg_v' second-track elements:" << std::endl;                               \
  std::cout << "trk_seg: [";                                                                                        \
  for (unsigned i = 0; i < num_emtf_sites; i++) {                                                                   \
    std::cout << trk_seg[(1 * num_emtf_sites) + i] << ", ";                                                         \
  }                                                                                                                 \
  std::cout << "] trk_seg_v: [";                                                                                    \
  for (unsigned i = 0; i < num_emtf_sites; i++) {                                                                   \
    std::cout << trk_seg_v[1][i] << ", ";                                                                           \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'trk_feat', 'trk_valid' first-track elements:" << std::endl;                               \
  std::cout << "trk_feat: [";                                                                                       \
  for (unsigned i = 0; i < num_emtf_features; i++) {                                                                \
    std::cout << trk_feat[i] << ", ";                                                                               \
  }                                                                                                                 \
  std::cout << "] trk_valid: [";                                                                                    \
  for (unsigned i = 0; i < trk_valid_t::width; i++) {                                                               \
    std::cout << trk_valid[i] << ", ";                                                                              \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'trk_feat_rm', 'trk_valid_rm' first-track elements:" << std::endl;                         \
  std::cout << "trk_feat_rm: [";                                                                                    \
  for (unsigned i = 0; i < num_emtf_features; i++) {                                                                \
    std::cout << trk_feat_rm[i] << ", ";                                                                            \
  }                                                                                                                 \
  std::cout << "] trk_valid_rm: [";                                                                                 \
  for (unsigned i = 0; i < trk_valid_t::width; i++) {                                                               \
    std::cout << trk_valid_rm[i] << ", ";                                                                           \
  }                                                                                                                 \
  std::cout << "]" << std::endl;                                                                                    \
  std::cout << "Printing 'trk_invpt' all elements:" << std::endl;                                                   \
  std::cout << "trk_invpt: [";                                                                                      \
  for (unsigned i = 0; i < num_emtf_tracks; i++) {                                                                  \
    std::cout << trk_invpt[i] << ", ";                                                                              \
  }                                                                                                                 \
  std::cout << "]" << std::endl;
#else
#define PRINT_TOP_FN_ARRAYS_OTHER
#endif  // __SYNTHESIS__ and NDEBUG not defined

#endif  // __EMTF_HLSLIB_PRINT_MACROS_H__ not defined
