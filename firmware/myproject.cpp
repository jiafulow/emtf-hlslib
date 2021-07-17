#include "myproject.h"

namespace {
using namespace emtf_hlslib::phase2;
}

namespace detail {
using namespace emtf_hlslib::phase2::detail;
}

// Top-level function implementation
void myproject(const top_in_t in0[TOP_N_IN], top_out_t out[TOP_N_OUT]) {
  // hls-pragmas begin
#pragma HLS PIPELINE II = model_config::target_ii
#pragma HLS INTERFACE ap_vld port = in0
#pragma HLS INTERFACE ap_vld port = out
#pragma HLS ARRAY_RESHAPE variable = in0 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = out complete dim = 0
  // hls-pragmas end

  // Unpack from in0
  // Note: the following are currently unused and will be synthesized away
  // - emtf_qual2, emtf_time, seg_cscfr, seg_gemdl, seg_bx
  emtf_phi_t emtf_phi[model_config::n_in];
  emtf_bend_t emtf_bend[model_config::n_in];
  emtf_theta1_t emtf_theta1[model_config::n_in];
  emtf_theta2_t emtf_theta2[model_config::n_in];
  emtf_qual1_t emtf_qual1[model_config::n_in];
  emtf_qual2_t emtf_qual2[model_config::n_in];
  emtf_time_t emtf_time[model_config::n_in];
  seg_zones_t seg_zones[model_config::n_in];
  seg_tzones_t seg_tzones[model_config::n_in];
  seg_cscfr_t seg_cscfr[model_config::n_in];
  seg_gemdl_t seg_gemdl[model_config::n_in];
  seg_bx_t seg_bx[model_config::n_in];
  seg_valid_t seg_valid[model_config::n_in];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = emtf_phi complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_bend complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_theta1 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_theta2 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_qual1 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_qual2 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = emtf_time complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_zones complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_tzones complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_cscfr complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_gemdl complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_bx complete dim = 0
#pragma HLS ARRAY_PARTITION variable = seg_valid complete dim = 0
  // hls-pragmas end

  // Loop over in0
LOOP_IN0:
  for (unsigned iseg = 0; iseg < model_config::n_in; iseg++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    emtf_phi[iseg] = in0[iseg].emtf_phi;
    emtf_bend[iseg] = in0[iseg].emtf_bend;
    emtf_theta1[iseg] = in0[iseg].emtf_theta1;
    emtf_theta2[iseg] = in0[iseg].emtf_theta2;
    emtf_qual1[iseg] = in0[iseg].emtf_qual1;
    emtf_qual2[iseg] = in0[iseg].emtf_qual2;
    emtf_time[iseg] = in0[iseg].emtf_time;
    seg_zones[iseg] = in0[iseg].seg_zones;
    seg_tzones[iseg] = in0[iseg].seg_tzones;
    seg_cscfr[iseg] = in0[iseg].seg_cscfr;
    seg_gemdl[iseg] = in0[iseg].seg_gemdl;
    seg_bx[iseg] = in0[iseg].seg_bx;
    seg_valid[iseg] = in0[iseg].seg_valid;
  }  // end loop over in0

  // This macro is defined in emtf_hlslib/print_macros.h
  PRINT_TOP_FN_ARRAYS_IN0

  // Check assumptions
  constexpr int num_trk_cols_with_safety = (((80 * 64) / emtf_img_col_factor) + (::detail::pattern_col_max_pad * 2));
  static_assert(is_same<seg_valid_t, bool_t>::value, "seg_valid_t type check failed");
  static_assert(trk_qual_t::width == max_emtf_pattern_activation_log2, "trk_qual_t type check failed");
  static_assert(trk_patt_t::width == ::detail::ceil_log2<num_emtf_patterns>::value, "trk_patt_t type check failed");
  static_assert(trk_col_t::width == ::detail::ceil_log2<num_trk_cols_with_safety>::value,
                "trk_col_t type check failed");
  static_assert(trk_zone_t::width == ::detail::ceil_log2<num_emtf_zones>::value, "trk_zone_t type check failed");
  static_assert(trk_tzone_t::width == ::detail::ceil_log2<num_emtf_timezones>::value, "trk_tzone_t type check failed");
  static_assert(trk_gate_t::width == ::detail::ceil_log2<num_emtf_img_gates>::value, "trk_gate_t type check failed");
  static_assert(trk_seg_t::width == ::detail::ceil_log2<model_config::n_in>::value, "trk_seg_t type check failed");
  static_assert(trk_seg_v_t::width == num_emtf_sites, "trk_seg_v_t type check failed");
  static_assert(trk_feat_t::width == emtf_phi_t::width, "trk_feat_t type check failed");
  static_assert(is_same<trk_valid_t, bool_t>::value, "trk_valid_t type check failed");
  static_assert(
      num_emtf_img_cols == (::detail::chamber_img_joined_col_stop - ::detail::chamber_img_joined_col_start + 1),
      "num_emtf_img_cols value check failed");
  static_assert(model_config::n_out_per_trk == (num_emtf_features + num_emtf_sites + 2),
                "model_config::n_out_per_trk value check failed");
  static_assert(model_config::n_out == (num_emtf_tracks * model_config::n_out_per_trk),
                "model_config::n_out value check failed");

  // Intermediate arrays (for layers 0..3)
  zoning_out_t zoning_0_out[zoning_config::n_out];
  zoning_out_t zoning_1_out[zoning_config::n_out];
  zoning_out_t zoning_2_out[zoning_config::n_out];
  pooling_out_t pooling_0_out[pooling_config::n_out];
  pooling_out_t pooling_1_out[pooling_config::n_out];
  pooling_out_t pooling_2_out[pooling_config::n_out];
  zonesorting_out_t zonesorting_0_out[zonesorting_config::n_out];
  zonesorting_out_t zonesorting_1_out[zonesorting_config::n_out];
  zonesorting_out_t zonesorting_2_out[zonesorting_config::n_out];
  zonemerging_out_t zonemerging_0_out[zonemerging_config::n_out];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = zoning_0_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = zoning_1_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = zoning_2_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = pooling_0_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = pooling_1_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = pooling_2_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = zonesorting_0_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = zonesorting_1_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = zonesorting_2_out complete dim = 0
#pragma HLS ARRAY_PARTITION variable = zonemerging_0_out complete dim = 0
  // hls-pragmas end

  // Layer 0 - Zoning

  zoning_layer<m_zone_any_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_0_out, zoning_1_out, zoning_2_out);

  // Layer 1 - Pooling

  pooling_layer<m_zone_0_tag>(zoning_0_out, pooling_0_out);
  pooling_layer<m_zone_1_tag>(zoning_1_out, pooling_1_out);
  pooling_layer<m_zone_2_tag>(zoning_2_out, pooling_2_out);

  // Layer 2 - Zone sorting

  zonesorting_layer<m_zone_any_tag>(pooling_0_out, zonesorting_0_out);
  zonesorting_layer<m_zone_any_tag>(pooling_1_out, zonesorting_1_out);
  zonesorting_layer<m_zone_any_tag>(pooling_2_out, zonesorting_2_out);

  // Layer 3 - Zone merging

  zonemerging_layer<m_zone_any_tag>(zonesorting_0_out, zonesorting_1_out, zonesorting_2_out, zonemerging_0_out);

  // Unpack from in1 (a.k.a. zonemerging_0_out)
  trk_qual_t trk_qual[trkbuilding_config::n_in];
  trk_patt_t trk_patt[trkbuilding_config::n_in];
  trk_col_t trk_col[trkbuilding_config::n_in];
  trk_zone_t trk_zone[trkbuilding_config::n_in];
  trk_tzone_t trk_tzone[trkbuilding_config::n_in];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = trk_qual complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_patt complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_col complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_zone complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_tzone complete dim = 0
  // hls-pragmas end

  // Loop over in1
LOOP_IN1:
  for (unsigned itrk = 0; itrk < trkbuilding_config::n_in; itrk++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const trkbuilding_in_t curr_trk_in = zonemerging_0_out[itrk];
    const trk_tzone_t curr_trk_tzone = ::detail::timezone_traits<m_timezone_0_tag>::value;  // default timezone

    constexpr int bits_lo_0 = 0;
    constexpr int bits_lo_1 = trk_qual_t::width;
    constexpr int bits_lo_2 = pooling_out_t::width;
    constexpr int bits_lo_3 = zonesorting_out_t::width;
    constexpr int bits_lo_4 = zonemerging_out_t::width;

    trk_qual[itrk] = curr_trk_in.range(bits_lo_1 - 1, bits_lo_0);
    trk_patt[itrk] = curr_trk_in.range(bits_lo_2 - 1, bits_lo_1);
    trk_col[itrk] = curr_trk_in.range(bits_lo_3 - 1, bits_lo_2);
    trk_zone[itrk] = curr_trk_in.range(bits_lo_4 - 1, bits_lo_3);
    trk_tzone[itrk] = curr_trk_tzone;
  }  // end loop over in1

  // Intermediate arrays (for layers 4..6)
  trk_seg_t trk_seg[trkbuilding_config::n_out * num_emtf_sites];
  trk_seg_v_t trk_seg_v[trkbuilding_config::n_out];
  trk_feat_t trk_feat[trkbuilding_config::n_out * num_emtf_features];
  trk_valid_t trk_valid[trkbuilding_config::n_out];
  trk_seg_t trk_seg_rm[duperemoval_config::n_out * num_emtf_sites];
  trk_seg_v_t trk_seg_rm_v[duperemoval_config::n_out];
  trk_feat_t trk_feat_rm[duperemoval_config::n_out * num_emtf_features];
  trk_valid_t trk_valid_rm[duperemoval_config::n_out];
  trk_invpt_t trk_invpt[fullyconnect_config::n_out];
  trk_phi_t trk_phi[fullyconnect_config::n_out];
  trk_eta_t trk_eta[fullyconnect_config::n_out];
  trk_d0_t trk_d0[fullyconnect_config::n_out];
  trk_z0_t trk_z0[fullyconnect_config::n_out];
  trk_beta_t trk_beta[fullyconnect_config::n_out];

  // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = trk_seg complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_v complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_feat complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_valid complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_rm complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_seg_rm_v complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_feat_rm complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_valid_rm complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_invpt complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_phi complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_eta complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_d0 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_z0 complete dim = 0
#pragma HLS ARRAY_PARTITION variable = trk_beta complete dim = 0
  // hls-pragmas end

  // Layer 4 - Track building

LOOP_TRK_1:
  for (unsigned itrk = 0; itrk < trkbuilding_config::n_in; itrk++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Intermediate arrays (for layer output)
    trk_seg_t curr_trk_seg[num_emtf_sites];
    trk_feat_t curr_trk_feat[num_emtf_features];

    // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = curr_trk_seg complete dim = 0
#pragma HLS ARRAY_PARTITION variable = curr_trk_feat complete dim = 0
    // hls-pragmas end

    trkbuilding_layer<m_zone_any_tag>(emtf_phi, emtf_bend, emtf_theta1, emtf_theta2, emtf_qual1, emtf_qual2, emtf_time,
                                      seg_zones, seg_tzones, seg_cscfr, seg_gemdl, seg_bx, seg_valid, trk_qual[itrk],
                                      trk_patt[itrk], trk_col[itrk], trk_zone[itrk], trk_tzone[itrk], curr_trk_seg,
                                      trk_seg_v[itrk], curr_trk_feat, trk_valid[itrk]);

    // Copy to arrays
    ::detail::copy_n_values<num_emtf_sites>(curr_trk_seg, &(trk_seg[itrk * num_emtf_sites]));
    ::detail::copy_n_values<num_emtf_features>(curr_trk_feat, &(trk_feat[itrk * num_emtf_features]));
  }  // end loop over tracks

  // Layer 5 - Duplicate removal

  duperemoval_layer<m_zone_any_tag>(trk_seg, trk_seg_v, trk_feat, trk_valid, trk_seg_rm, trk_seg_rm_v, trk_feat_rm,
                                    trk_valid_rm);

  // Layer 6 - Fully connected

LOOP_TRK_2:
  for (unsigned itrk = 0; itrk < fullyconnect_config::n_in; itrk++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    // Intermediate arrays (for layer input)
    trk_feat_t curr_trk_feat_rm[num_emtf_features];

    // hls-pragmas begin
#pragma HLS ARRAY_PARTITION variable = curr_trk_feat complete dim = 0
    // hls-pragmas end

    // Copy from arrays
    ::detail::copy_n_values<num_emtf_features>(&(trk_feat_rm[itrk * num_emtf_features]), curr_trk_feat_rm);

    fullyconnect_layer<m_zone_any_tag>(curr_trk_feat_rm, trk_invpt[itrk], trk_phi[itrk], trk_eta[itrk], trk_d0[itrk],
                                       trk_z0[itrk], trk_beta[itrk]);
  }  // end loop over tracks

  // Copy to output: trk_feat_rm, trk_seg_rm, trk_valid_rm, trk_invpt
LOOP_OUT:
  for (unsigned i = 0; i < model_config::n_out; i++) {
    // hls-pragmas begin
#pragma HLS UNROLL
    // hls-pragmas end

    const unsigned itrk = (i / model_config::n_out_per_trk);
    const unsigned ivar = (i % model_config::n_out_per_trk);

    auto curr_trk_seg_rm = &(trk_seg_rm[itrk * num_emtf_sites]);
    auto curr_trk_feat_rm = &(trk_feat_rm[itrk * num_emtf_features]);

    if (ivar < num_emtf_features) {
      out[i] = curr_trk_feat_rm[ivar];
    } else if (ivar < (num_emtf_features + num_emtf_sites + 0)) {
      const unsigned ivar_1 = (ivar - num_emtf_features);
      const trk_seg_t invalid_marker_trk_seg = model_config::n_in;
      out[i] = (trk_seg_rm_v[itrk][ivar_1]) ? curr_trk_seg_rm[ivar_1] : invalid_marker_trk_seg;
    } else if (ivar < (num_emtf_features + num_emtf_sites + 1)) {
      out[i] = trk_valid_rm[itrk];
    } else if (ivar < (num_emtf_features + num_emtf_sites + 2)) {
      out[i] = trk_invpt[itrk];
    }
  }  // end loop over out

  // This macro is defined in emtf_hlslib/print_macros.h
  PRINT_TOP_FN_ARRAYS_OTHER
}
