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
                << seg_tzones[iseg] << " " << seg_cscfr[iseg] << " " << seg_gemdl[iseg] << " " << seg_bx[iseg] << " " \
                << seg_valid[iseg] << " " << std::endl;                                                               \
    }                                                                                                                 \
  }
#else
#define PRINT_TOP_FN_ARRAYS_IN0
#endif  // __SYNTHESIS__ and NDEBUG not defined

#if !defined(__SYNTHESIS__) && !defined(NDEBUG)
#define PRINT_TOP_FN_ARRAYS_OTHER                                                                                     \
  std::cout << "Printing 'seg_valid' non-zero indices:" << std::endl;                                                 \
  std::cout << ".. iseg: [";                                                                                          \
  for (unsigned iseg = 0; iseg < model_config::n_in; iseg++) {                                                        \
    if (seg_valid[iseg] == 1) {                                                                                       \
      std::cout << iseg << ", ";                                                                                      \
    }                                                                                                                 \
  }                                                                                                                   \
  std::cout << "]" << std::endl;                                                                                      \
  std::cout << "Printing 'zoning_0_out', 'zoning_1_out', 'zoning_2_out' non-zero elements:" << std::endl;             \
  for (unsigned i = 0; i < num_emtf_zones; i++) {                                                                     \
    std::cout << ".. row: [";                                                                                         \
    for (unsigned j = 0; j < zoning_config::n_out; j++) {                                                             \
      for (unsigned k = 0; k < zoning_out_t::width; k++) {                                                            \
        if (i == 0) {                                                                                                 \
          if (zoning_0_out[j].test(k))                                                                                \
            std::cout << j << ", ";                                                                                   \
        } else if (i == 1) {                                                                                          \
          if (zoning_1_out[j].test(k))                                                                                \
            std::cout << j << ", ";                                                                                   \
        } else if (i == 2) {                                                                                          \
          if (zoning_2_out[j].test(k))                                                                                \
            std::cout << j << ", ";                                                                                   \
        }                                                                                                             \
      }                                                                                                               \
    }                                                                                                                 \
    std::cout << "] col: [";                                                                                          \
    for (unsigned j = 0; j < zoning_config::n_out; j++) {                                                             \
      for (unsigned k = 0; k < zoning_out_t::width; k++) {                                                            \
        if (i == 0) {                                                                                                 \
          if (zoning_0_out[j].test(k))                                                                                \
            std::cout << k << ", ";                                                                                   \
        } else if (i == 1) {                                                                                          \
          if (zoning_1_out[j].test(k))                                                                                \
            std::cout << k << ", ";                                                                                   \
        } else if (i == 2) {                                                                                          \
          if (zoning_2_out[j].test(k))                                                                                \
            std::cout << k << ", ";                                                                                   \
        }                                                                                                             \
      }                                                                                                               \
    }                                                                                                                 \
    std::cout << "]" << std::endl;                                                                                    \
  }                                                                                                                   \
  std::cout << "Printing 'pooling_0_out', 'pooling_1_out', 'pooling_2_out' non-zero elements:" << std::endl;          \
  for (unsigned i = 0; i < num_emtf_zones; i++) {                                                                     \
    std::cout << ".. col: [";                                                                                         \
    for (unsigned j = 0; j < pooling_config::n_out; j++) {                                                            \
      if (i == 0) {                                                                                                   \
        if (pooling_0_out[j])                                                                                         \
          std::cout << j << ", ";                                                                                     \
      } else if (i == 1) {                                                                                            \
        if (pooling_1_out[j])                                                                                         \
          std::cout << j << ", ";                                                                                     \
      } else if (i == 2) {                                                                                            \
        if (pooling_2_out[j])                                                                                         \
          std::cout << j << ", ";                                                                                     \
      }                                                                                                               \
    }                                                                                                                 \
    std::cout << "] act: [";                                                                                          \
    for (unsigned j = 0; j < pooling_config::n_out; j++) {                                                            \
      if (i == 0) {                                                                                                   \
        if (pooling_0_out[j])                                                                                         \
          std::cout << pooling_0_out[j].range(trk_qual_t::width - 1, 0) << ", ";                                      \
      } else if (i == 1) {                                                                                            \
        if (pooling_1_out[j])                                                                                         \
          std::cout << pooling_1_out[j].range(trk_qual_t::width - 1, 0) << ", ";                                      \
      } else if (i == 2) {                                                                                            \
        if (pooling_2_out[j])                                                                                         \
          std::cout << pooling_2_out[j].range(trk_qual_t::width - 1, 0) << ", ";                                      \
      }                                                                                                               \
    }                                                                                                                 \
    std::cout << "]" << std::endl;                                                                                    \
  }                                                                                                                   \
  std::cout << "Printing 'zonesorting_0_out', 'zonesorting_1_out', 'zonesorting_2_out' all elements:" << std::endl;   \
  for (unsigned i = 0; i < num_emtf_zones; i++) {                                                                     \
    std::cout << ".. act: [";                                                                                         \
    for (unsigned j = 0; j < zonesorting_config::n_out; j++) {                                                        \
      if (i == 0) {                                                                                                   \
        std::cout << zonesorting_0_out[j].range(trk_qual_t::width - 1, 0) << ", ";                                    \
      } else if (i == 1) {                                                                                            \
        std::cout << zonesorting_1_out[j].range(trk_qual_t::width - 1, 0) << ", ";                                    \
      } else if (i == 2) {                                                                                            \
        std::cout << zonesorting_2_out[j].range(trk_qual_t::width - 1, 0) << ", ";                                    \
      }                                                                                                               \
    }                                                                                                                 \
    std::cout << "]" << std::endl;                                                                                    \
  }                                                                                                                   \
  std::cout << "Printing 'zonemerging_0_out' all elements:" << std::endl;                                             \
  std::cout << ".. act: [";                                                                                           \
  for (unsigned i = 0; i < zonemerging_config::n_out; i++) {                                                          \
    std::cout << zonemerging_0_out[i].range(trk_qual_t::width - 1, 0) << ", ";                                        \
  }                                                                                                                   \
  std::cout << "]" << std::endl;                                                                                      \
  std::cout << "Printing 'trk_qual', 'trk_patt', 'trk_col', 'trk_zone', 'trk_tzone' elements separately for ";        \
  std::cout << "each track" << std::endl;                                                                             \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                           \
    std::cout << ".. trk_qual: [" << trk_qual[i] << ", ";                                                             \
    std::cout << "] trk_patt: [" << trk_patt[i] << ", ";                                                              \
    std::cout << "] trk_col: [" << trk_col[i] << ", ";                                                                \
    std::cout << "] trk_zone: [" << trk_zone[i] << ", ";                                                              \
    std::cout << "] trk_tzone: [" << trk_tzone[i] << ", ";                                                            \
    std::cout << "]" << std::endl;                                                                                    \
  }                                                                                                                   \
  std::cout << "Printing 'trk_seg', 'trk_seg_v' elements separately for each track:" << std::endl;                    \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                           \
    std::cout << ".. trk_seg: [";                                                                                     \
    for (unsigned j = 0; j < num_emtf_sites; j++) {                                                                   \
      std::cout << trk_seg[(i * num_emtf_sites) + j] << ", ";                                                         \
    }                                                                                                                 \
    std::cout << "] trk_seg_v: [";                                                                                    \
    for (unsigned j = 0; j < num_emtf_sites; j++) {                                                                   \
      std::cout << trk_seg_v[i][j] << ", ";                                                                           \
    }                                                                                                                 \
    std::cout << "]" << std::endl;                                                                                    \
  }                                                                                                                   \
  std::cout << "Printing 'trk_feat', 'trk_valid' elements separately for each track:" << std::endl;                   \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                           \
    std::cout << ".. trk_feat: [";                                                                                    \
    for (unsigned j = 0; j < num_emtf_features; j++) {                                                                \
      std::cout << trk_feat[(i * num_emtf_features) + j] << ", ";                                                     \
    }                                                                                                                 \
    std::cout << "] trk_valid: [" << trk_valid[i] << ", ";                                                            \
    std::cout << "]" << std::endl;                                                                                    \
  }                                                                                                                   \
  std::cout << "Printing 'trk_feat_rm', 'trk_valid_rm' elements separately for each track:" << std::endl;             \
  for (unsigned i = 0; i < trkbuilding_config::n_in; i++) {                                                           \
    std::cout << ".. trk_feat_rm: [";                                                                                 \
    for (unsigned j = 0; j < num_emtf_features; j++) {                                                                \
      std::cout << trk_feat_rm[(i * num_emtf_features) + j] << ", ";                                                  \
    }                                                                                                                 \
    std::cout << "] trk_valid_rm: [" << trk_valid_rm[i] << ", ";                                                      \
    std::cout << "]" << std::endl;                                                                                    \
  }                                                                                                                   \
  std::cout << "Printing 'trk_invpt', 'trk_phi', 'trk_eta', 'trk_d0', 'trk_z0', 'trk_beta' elements separately for "; \
  std::cout << "each track:" << std::endl;                                                                            \
  for (unsigned i = 0; i < num_emtf_tracks; i++) {                                                                    \
    std::cout << ".. trk_invpt: [" << trk_invpt[i] << ", ";                                                           \
    std::cout << "] trk_phi: [" << trk_phi[i] << ", ";                                                                \
    std::cout << "] trk_eta: [" << trk_eta[i] << ", ";                                                                \
    std::cout << "] trk_d0: [" << trk_d0[i] << ", ";                                                                  \
    std::cout << "] trk_z0: [" << trk_z0[i] << ", ";                                                                  \
    std::cout << "] trk_beta: [" << trk_beta[i] << ", ";                                                              \
    std::cout << "]" << std::endl;                                                                                    \
  }
#else
#define PRINT_TOP_FN_ARRAYS_OTHER
#endif  // __SYNTHESIS__ and NDEBUG not defined

#endif  // __EMTF_HLSLIB_PRINT_MACROS_H__ not defined
