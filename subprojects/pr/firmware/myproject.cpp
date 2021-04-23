#include "myproject.h"

using namespace emtf;

// Top-level function implementation
void myproject(
    const top_in_t in0[TOP_N_IN],
    top_out_t out[TOP_N_OUT]
) {

#pragma HLS PIPELINE II=model_config::target_ii

#pragma HLS INTERFACE ap_vld port=in0
#pragma HLS INTERFACE ap_vld port=out
//#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS ARRAY_RESHAPE variable=in0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=out complete dim=0

  // Unpack from in0
  // Note: the following are currently unused and will be synthesized away
  // - emtf_qual2, emtf_time, seg_fr, seg_dl, seg_bx
  emtf_phi_t    emtf_phi    [model_config::n_in];
  emtf_bend_t   emtf_bend   [model_config::n_in];
  emtf_theta1_t emtf_theta1 [model_config::n_in];
  emtf_theta2_t emtf_theta2 [model_config::n_in];
  emtf_qual1_t  emtf_qual1  [model_config::n_in];
  emtf_qual2_t  emtf_qual2  [model_config::n_in];
  emtf_time_t   emtf_time   [model_config::n_in];
  seg_zones_t   seg_zones   [model_config::n_in];
  seg_tzones_t  seg_tzones  [model_config::n_in];
  seg_fr_t      seg_fr      [model_config::n_in];
  seg_dl_t      seg_dl      [model_config::n_in];
  seg_bx_t      seg_bx      [model_config::n_in];
  seg_valid_t   seg_valid   [model_config::n_in];

#pragma HLS ARRAY_PARTITION variable=emtf_phi complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_bend complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_theta1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_theta2 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_qual1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_qual2 complete dim=0
#pragma HLS ARRAY_PARTITION variable=emtf_time complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_zones complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_tzones complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_fr complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_dl complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_bx complete dim=0
#pragma HLS ARRAY_PARTITION variable=seg_valid complete dim=0

  // Loop over in0
  LOOP_IN0: for (unsigned iseg = 0; iseg < model_config::n_in; iseg++) {

#pragma HLS UNROLL

    emtf_phi[iseg]    = in0[iseg].emtf_phi;
    emtf_bend[iseg]   = in0[iseg].emtf_bend;
    emtf_theta1[iseg] = in0[iseg].emtf_theta1;
    emtf_theta2[iseg] = in0[iseg].emtf_theta2;
    emtf_qual1[iseg]  = in0[iseg].emtf_qual1;
    emtf_qual2[iseg]  = in0[iseg].emtf_qual2;
    emtf_time[iseg]   = in0[iseg].emtf_time;
    seg_zones[iseg]   = in0[iseg].seg_zones;
    seg_tzones[iseg]  = in0[iseg].seg_tzones;
    seg_fr[iseg]      = in0[iseg].seg_fr;
    seg_dl[iseg]      = in0[iseg].seg_dl;
    seg_bx[iseg]      = in0[iseg].seg_bx;
    seg_valid[iseg]   = in0[iseg].seg_valid;
  }  // end loop over in0

  // This macro is defined in emtf_hlslib/helper.h
  //PRINT_TOP_FN_ARRAYS_IN0

  // Check assumptions
  constexpr int num_trk_cols_with_safety = (((80 * 64) / emtf_img_col_factor) + (max_emtf_img_col_pad * 2));
  static_assert(trk_qual_t::width == max_emtf_pattern_activation_log2, "trk_qual_t type check failed");
  static_assert(trk_patt_t::width == details::ceil_log2<num_emtf_patterns>::value, "trk_patt_t type check failed");
  static_assert(trk_col_t::width == details::ceil_log2<num_trk_cols_with_safety>::value, "trk_col_t type check failed");
  static_assert(trk_zone_t::width == details::ceil_log2<num_emtf_zones>::value, "trk_zone_t type check failed");
  static_assert(trk_tzone_t::width == details::ceil_log2<num_emtf_timezones>::value, "trk_tzone_t type check failed");
  static_assert(trk_seg_t::width == details::ceil_log2<model_config::n_in>::value, "trk_seg_t type check failed");
  static_assert(trk_seg_v_t::width == num_emtf_sites, "trk_seg_v_t type check failed");
  static_assert(trk_feat_t::width == emtf_phi_t::width, "trk_feat_t type check failed");
  static_assert(trk_valid_t::width == 1, "trk_valid_t type check failed");
  static_assert(
      model_config::n_out_per_trk == (num_emtf_features + num_emtf_sites + 2),
      "model_config::n_out_per_trk check failed"
  );
  static_assert(
      model_config::n_out == (num_emtf_tracks * model_config::n_out_per_trk),
      "model_config::n_out check failed"
  );

  // Intermediate arrays (for layers 0..4)
  zoning_out_t      zoning_0_out      [zoning_config::n_out];
  zoning_out_t      zoning_1_out      [zoning_config::n_out];
  zoning_out_t      zoning_2_out      [zoning_config::n_out];
  pooling_out_t     pooling_0_out     [pooling_config::n_out];
  pooling_out_t     pooling_1_out     [pooling_config::n_out];
  pooling_out_t     pooling_2_out     [pooling_config::n_out];
  suppression_out_t suppression_0_out [suppression_config::n_out];
  suppression_out_t suppression_1_out [suppression_config::n_out];
  suppression_out_t suppression_2_out [suppression_config::n_out];
  zonesorting_out_t zonesorting_0_out [zonesorting_config::n_out];
  zonesorting_out_t zonesorting_1_out [zonesorting_config::n_out];
  zonesorting_out_t zonesorting_2_out [zonesorting_config::n_out];
  zonemerging_out_t zonemerging_0_out [zonemerging_config::n_out];

#pragma HLS ARRAY_PARTITION variable=zoning_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zoning_1_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zoning_2_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=pooling_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=pooling_1_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=pooling_2_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=suppression_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=suppression_1_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=suppression_2_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonesorting_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonesorting_1_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonesorting_2_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=zonemerging_0_out complete dim=0

  // Layer 0 - Zoning

  zoning_layer<m_zone_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_0_out);
  zoning_layer<m_zone_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_1_out);
  zoning_layer<m_zone_2_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_2_out);

  // Layer 1 - Pooling

  pooling_layer<m_zone_0_tag>(zoning_0_out, pooling_0_out);
  pooling_layer<m_zone_1_tag>(zoning_1_out, pooling_1_out);
  pooling_layer<m_zone_2_tag>(zoning_2_out, pooling_2_out);

  // Layer 2 - Non-max suppression

  suppression_layer<m_zone_any_tag>(pooling_0_out, suppression_0_out);
  suppression_layer<m_zone_any_tag>(pooling_1_out, suppression_1_out);
  suppression_layer<m_zone_any_tag>(pooling_2_out, suppression_2_out);

  // Layer 3 - Zone sorting

  zonesorting_layer<m_zone_any_tag>(suppression_0_out, zonesorting_0_out);
  zonesorting_layer<m_zone_any_tag>(suppression_1_out, zonesorting_1_out);
  zonesorting_layer<m_zone_any_tag>(suppression_2_out, zonesorting_2_out);

  // Layer 4 - Zone merging

  zonemerging_layer<m_zone_any_tag>(
      zonesorting_0_out, zonesorting_1_out, zonesorting_2_out, zonemerging_0_out
  );

  // Copy to output: trk_qual, trk_patt, trk_col, trk_zone
  LOOP_OUT: for (unsigned i = 0; i < TOP_N_OUT; i++) {

#pragma HLS UNROLL

    const unsigned n_out_per_trk = TOP_N_OUT / num_emtf_tracks;
    const unsigned itrk = i / n_out_per_trk;
    const unsigned ivar = i % n_out_per_trk;

    const trkbuilding_in_t curr_trk_in = zonemerging_0_out[itrk];

    constexpr int bits_lo_0 = 0;
    constexpr int bits_lo_1 = bits_lo_0 + trk_qual_t::width;
    constexpr int bits_lo_2 = bits_lo_1 + trk_patt_t::width;
    constexpr int bits_lo_3 = bits_lo_2 + trk_col_t::width;
    constexpr int bits_lo_4 = bits_lo_3 + trk_zone_t::width;

    const trk_qual_t curr_trk_qual = curr_trk_in.range(bits_lo_1 - 1, bits_lo_0);
    const trk_patt_t curr_trk_patt = curr_trk_in.range(bits_lo_2 - 1, bits_lo_1);
    const trk_col_t  curr_trk_col  = curr_trk_in.range(bits_lo_3 - 1, bits_lo_2);
    const trk_zone_t curr_trk_zone = curr_trk_in.range(bits_lo_4 - 1, bits_lo_3);

    if (ivar == 0) {
      out[i] = curr_trk_qual;
    } else if (ivar == 1) {
      out[i] = curr_trk_patt;
    } else if (ivar == 2) {
      out[i] = curr_trk_col;
    } else if (ivar == 3) {
      out[i] = curr_trk_zone;
    }
  }  // end loop over out
}
