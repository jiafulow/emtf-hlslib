#ifndef __EMTF_HLSLIB_MODEL_CONFIGS_H__
#define __EMTF_HLSLIB_MODEL_CONFIGS_H__

// EMTF HLS
#include "common.h"

namespace emtf {

namespace phase2 {

// _____________________________________________________________________________
// Model configs

struct model_config {
  static const unsigned int n_in = num_emtf_chambers * num_emtf_segments;
  static const unsigned int n_out_per_trk = (num_emtf_features + num_emtf_sites + 2);
  static const unsigned int n_out = num_emtf_tracks * n_out_per_trk;

  // Set target clock freq to 4 times the input clock freq (= 4 x 40MHz).
  // Set target II to match the clock freq so that it still consumes 1 input
  // every input clock at 40 MHz.
  static const int target_clock = num_emtf_tracks;  // in unit of 40MHz
  static const int target_ii = target_clock;
};

// _____________________________________________________________________________
// Layer configs

struct zoning_config {
  static const unsigned int n_in = 0;
  static const unsigned int n_out = num_emtf_img_rows;
  static const int layer_target_ii = model_config::target_ii;
  static const int target_ii = 1;
};

struct pooling_config {
  static const unsigned int n_in = zoning_config::n_out;
  static const unsigned int n_out = num_emtf_img_cols;
  static const int layer_target_ii = model_config::target_ii;
  static const int target_ii = 1;

  // Fusion of column-wise operations
  static const int fusion_factor = 8;
};

struct zonesorting_config {
  static const unsigned int n_in = pooling_config::n_out;
  static const unsigned int n_out = num_emtf_tracks;
  static const int layer_target_ii = 1;
  static const int target_ii = 1;

  // Used for preprocessing
  static const unsigned int n_stage_0 = n_in / 2;
};

struct zonemerging_config {
  static const unsigned int n_in = zonesorting_config::n_out;
  static const unsigned int n_out = n_in;
  static const int layer_target_ii = 1;
  static const int target_ii = 1;

  // Used for preprocessing
  static const unsigned int n_stage_0 = n_in * num_emtf_zones;
};

struct trkbuilding_config {
  static const unsigned int n_in = zonemerging_config::n_out;
  static const unsigned int n_out = n_in;
  static const int layer_target_ii = 1;
  static const int target_ii = 1;
};

struct duperemoval_config {
  static const unsigned int n_in = trkbuilding_config::n_out;
  static const unsigned int n_out = n_in;
  static const int layer_target_ii = 1;
  static const int target_ii = 1;
};

struct fullyconnect_config {
  static const unsigned int n_in = duperemoval_config::n_out;
  static const unsigned int n_out = n_in;
  static const int layer_target_ii = 1;
  static const int target_ii = 1;
};

// _____________________________________________________________________________
// Various tags

// Subsystem
struct m_dt_subsystem_tag {};
struct m_csc_subsystem_tag {};
struct m_rpc_subsystem_tag {};
struct m_gem_subsystem_tag {};
struct m_me0_subsystem_tag {};

// Chamber
struct m_10deg_chamber_tag {};
struct m_20deg_chamber_tag {};
struct m_20deg_ext_chamber_tag {};

// Zone
struct m_zone_0_tag {};
struct m_zone_1_tag {};
struct m_zone_2_tag {};
struct m_zone_any_tag {};

// Timezone
struct m_timezone_0_tag {};
struct m_timezone_1_tag {};
struct m_timezone_2_tag {};
struct m_timezone_any_tag {};

// Row
struct m_zone_0_row_0_tag : m_zone_0_tag {};
struct m_zone_0_row_1_tag : m_zone_0_tag {};
struct m_zone_0_row_2_tag : m_zone_0_tag {};
struct m_zone_0_row_3_tag : m_zone_0_tag {};
struct m_zone_0_row_4_tag : m_zone_0_tag {};
struct m_zone_0_row_5_tag : m_zone_0_tag {};
struct m_zone_0_row_6_tag : m_zone_0_tag {};
struct m_zone_0_row_7_0_tag : m_zone_0_tag {};
struct m_zone_0_row_7_1_tag : m_zone_0_tag {};
//
struct m_zone_1_row_0_tag : m_zone_1_tag {};
struct m_zone_1_row_1_tag : m_zone_1_tag {};
struct m_zone_1_row_2_0_tag : m_zone_1_tag {};
struct m_zone_1_row_2_1_tag : m_zone_1_tag {};
struct m_zone_1_row_3_tag : m_zone_1_tag {};
struct m_zone_1_row_4_tag : m_zone_1_tag {};
struct m_zone_1_row_5_tag : m_zone_1_tag {};
struct m_zone_1_row_6_tag : m_zone_1_tag {};
struct m_zone_1_row_7_0_tag : m_zone_1_tag {};
struct m_zone_1_row_7_1_tag : m_zone_1_tag {};
//
struct m_zone_2_row_0_tag : m_zone_2_tag {};
struct m_zone_2_row_1_tag : m_zone_2_tag {};
struct m_zone_2_row_2_tag : m_zone_2_tag {};
struct m_zone_2_row_3_tag : m_zone_2_tag {};
struct m_zone_2_row_4_tag : m_zone_2_tag {};
struct m_zone_2_row_5_tag : m_zone_2_tag {};
struct m_zone_2_row_6_tag : m_zone_2_tag {};
struct m_zone_2_row_7_tag : m_zone_2_tag {};

// Site
struct m_site_0_tag : m_zone_any_tag {};
struct m_site_1_tag : m_zone_any_tag {};
struct m_site_2_tag : m_zone_any_tag {};
struct m_site_3_tag : m_zone_any_tag {};
struct m_site_4_tag : m_zone_any_tag {};
struct m_site_5_tag : m_zone_any_tag {};
struct m_site_6_tag : m_zone_any_tag {};
struct m_site_7_tag : m_zone_any_tag {};
struct m_site_8_tag : m_zone_any_tag {};
struct m_site_9_tag : m_zone_any_tag {};
struct m_site_10_tag : m_zone_any_tag {};
struct m_site_11_tag : m_zone_any_tag {};

// NN layer
struct m_nnet_0_layer_0_tag {};
struct m_nnet_0_layer_1_tag {};
struct m_nnet_0_layer_2_tag {};
struct m_nnet_0_layer_3_tag {};
struct m_nnet_0_layer_4_tag {};

// HLS directives
struct m_inline_directive {};

}  // namespace phase2

}  // namespace emtf

#endif  // __EMTF_HLSLIB_MODEL_CONFIGS_H__ not defined
