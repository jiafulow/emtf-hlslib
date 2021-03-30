#ifndef __EMTF_HLSLIB_ZONING_H__
#define __EMTF_HLSLIB_ZONING_H__

namespace emtf {

template <typename T_IN, typename T_OUT>
void zoning_join_10deg_chamber_images_op(const T_IN in0[details::num_chambers_10deg], T_OUT& out) {
  static_assert(is_same<T_IN, ap_uint<details::chamber_img_bw> >::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, ap_uint<details::chamber_img_joined_bw> >::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zoning_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // There are 14 edges that define 13 regions
  constexpr int col_start_f = 0;

  constexpr int col_start_0 = details::chamber_ph_init_10deg[6];
  constexpr int col_start_1 = details::chamber_ph_init_10deg[0];
  constexpr int col_start_3 = details::chamber_ph_init_10deg[1];
  constexpr int col_start_5 = details::chamber_ph_init_10deg[2];
  constexpr int col_start_7 = details::chamber_ph_init_10deg[3];
  constexpr int col_start_9 = details::chamber_ph_init_10deg[4];
  constexpr int col_start_11 = details::chamber_ph_init_10deg[5];

  constexpr int col_start_2 = details::chamber_ph_cover_10deg[6];
  constexpr int col_start_4 = details::chamber_ph_cover_10deg[0];
  constexpr int col_start_6 = details::chamber_ph_cover_10deg[1];
  constexpr int col_start_8 = details::chamber_ph_cover_10deg[2];
  constexpr int col_start_10 = details::chamber_ph_cover_10deg[3];
  constexpr int col_start_12 = details::chamber_ph_cover_10deg[4];
  constexpr int col_start_13 = details::chamber_ph_cover_10deg[5];

  // There are 13 regions that alternate between non-overlapping and overlapping
  out.range(col_start_0 - 1, col_start_f) = 0;
  out.range(col_start_1 - 1, col_start_0) = (in0[6].range(col_start_1 - col_start_0 - 1, col_start_0 - col_start_0));
  out.range(col_start_2 - 1, col_start_1) = (in0[6].range(col_start_2 - col_start_0 - 1, col_start_1 - col_start_0) | in0[0].range(col_start_2 - col_start_1 - 1, col_start_1 - col_start_1));
  out.range(col_start_3 - 1, col_start_2) = (in0[0].range(col_start_3 - col_start_1 - 1, col_start_2 - col_start_1));
  out.range(col_start_4 - 1, col_start_3) = (in0[0].range(col_start_4 - col_start_1 - 1, col_start_3 - col_start_1) | in0[1].range(col_start_4 - col_start_3 - 1, col_start_3 - col_start_3));
  out.range(col_start_5 - 1, col_start_4) = (in0[1].range(col_start_5 - col_start_3 - 1, col_start_4 - col_start_3));
  out.range(col_start_6 - 1, col_start_5) = (in0[1].range(col_start_6 - col_start_3 - 1, col_start_5 - col_start_3) | in0[2].range(col_start_6 - col_start_5 - 1, col_start_5 - col_start_5));
  out.range(col_start_7 - 1, col_start_6) = (in0[2].range(col_start_7 - col_start_5 - 1, col_start_6 - col_start_5));
  out.range(col_start_8 - 1, col_start_7) = (in0[2].range(col_start_8 - col_start_5 - 1, col_start_7 - col_start_5) | in0[3].range(col_start_8 - col_start_7 - 1, col_start_7 - col_start_7));
  out.range(col_start_9 - 1, col_start_8) = (in0[3].range(col_start_9 - col_start_7 - 1, col_start_8 - col_start_7));
  out.range(col_start_10 - 1, col_start_9) = (in0[3].range(col_start_10 - col_start_7 - 1, col_start_9 - col_start_7) | in0[4].range(col_start_10 - col_start_9 - 1, col_start_9 - col_start_9));
  out.range(col_start_11 - 1, col_start_10) = (in0[4].range(col_start_11 - col_start_9 - 1, col_start_10 - col_start_9));
  out.range(col_start_12 - 1, col_start_11) = (in0[4].range(col_start_12 - col_start_9 - 1, col_start_11 - col_start_9) | in0[5].range(col_start_12 - col_start_11 - 1, col_start_11 - col_start_11));
  out.range(col_start_13 - 1, col_start_12) = (in0[5].range(col_start_13 - col_start_11 - 1, col_start_12 - col_start_11));
}

template <typename T_IN, typename T_OUT>
void zoning_join_20deg_chamber_images_op(const T_IN in0[details::num_chambers_20deg], T_OUT& out) {
  static_assert(is_same<T_IN, ap_uint<details::chamber_img_bw> >::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, ap_uint<details::chamber_img_joined_bw> >::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zoning_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // There are 8 edges that define 7 regions
  constexpr int col_start_f = 0;

  constexpr int col_start_0 = details::chamber_ph_init_20deg[3];
  constexpr int col_start_1 = details::chamber_ph_init_20deg[0];
  constexpr int col_start_3 = details::chamber_ph_init_20deg[1];
  constexpr int col_start_5 = details::chamber_ph_init_20deg[2];

  constexpr int col_start_2 = details::chamber_ph_cover_20deg[3];
  constexpr int col_start_4 = details::chamber_ph_cover_20deg[0];
  constexpr int col_start_6 = details::chamber_ph_cover_20deg[1];
  constexpr int col_start_7 = details::chamber_ph_cover_20deg[2];

  // There are 7 regions that alternate between non-overlapping and overlapping
  emtf_assert(col_start_0 == col_start_f);
  out.range(col_start_1 - 1, col_start_0) = (in0[3].range(col_start_1 - col_start_0 - 1, col_start_0 - col_start_0));
  out.range(col_start_2 - 1, col_start_1) = (in0[3].range(col_start_2 - col_start_0 - 1, col_start_1 - col_start_0) | in0[0].range(col_start_2 - col_start_1 - 1, col_start_1 - col_start_1));
  out.range(col_start_3 - 1, col_start_2) = (in0[0].range(col_start_3 - col_start_1 - 1, col_start_2 - col_start_1));
  out.range(col_start_4 - 1, col_start_3) = (in0[0].range(col_start_4 - col_start_1 - 1, col_start_3 - col_start_1) | in0[1].range(col_start_4 - col_start_3 - 1, col_start_3 - col_start_3));
  out.range(col_start_5 - 1, col_start_4) = (in0[1].range(col_start_5 - col_start_3 - 1, col_start_4 - col_start_3));
  out.range(col_start_6 - 1, col_start_5) = (in0[1].range(col_start_6 - col_start_3 - 1, col_start_5 - col_start_3) | in0[2].range(col_start_6 - col_start_5 - 1, col_start_5 - col_start_5));
  out.range(col_start_7 - 1, col_start_6) = (in0[2].range(col_start_7 - col_start_5 - 1, col_start_6 - col_start_5));
}

template <typename T_IN, typename T_OUT>
void zoning_join_20deg_ext_chamber_images_op(const T_IN in0[details::num_chambers_20deg_ext], T_OUT& out) {
  static_assert(is_same<T_IN, ap_uint<details::chamber_img_bw> >::value, "T_IN type check failed");
  static_assert(is_same<T_OUT, ap_uint<details::chamber_img_joined_bw> >::value, "T_OUT type check failed");

#pragma HLS PIPELINE II=zoning_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // There are 14 edges that define 13 regions
  constexpr int col_start_f = 0;

  constexpr int col_start_0 = details::chamber_ph_init_10deg[6];
  constexpr int col_start_1 = details::chamber_ph_init_10deg[0];
  constexpr int col_start_3 = details::chamber_ph_init_10deg[1];
  constexpr int col_start_5 = details::chamber_ph_init_10deg[2];
  constexpr int col_start_7 = details::chamber_ph_init_10deg[3];
  constexpr int col_start_9 = details::chamber_ph_init_10deg[4];
  constexpr int col_start_11 = details::chamber_ph_init_10deg[5];

  constexpr int col_start_2 = details::chamber_ph_cover_10deg[6];
  constexpr int col_start_4 = details::chamber_ph_cover_10deg[0];
  constexpr int col_start_6 = details::chamber_ph_cover_10deg[1];
  constexpr int col_start_8 = details::chamber_ph_cover_10deg[2];
  constexpr int col_start_10 = details::chamber_ph_cover_10deg[3];
  constexpr int col_start_12 = details::chamber_ph_cover_10deg[4];
  constexpr int col_start_13 = details::chamber_ph_cover_10deg[5];

  // There are 13 regions that alternate between non-overlapping and overlapping
  // Note that col_start_0 is different for 10deg vs 20 deg chambers
  out.range(col_start_0 - 1, col_start_f) = (in0[0+3].range(col_start_0 - 1, col_start_f));
  out.range(col_start_1 - 1, col_start_0) = (in0[0+3].range(col_start_1 - col_start_f - 1, col_start_0 - col_start_f) |
                                             in0[4+6].range(col_start_1 - col_start_0 - 1, col_start_0 - col_start_0));
  out.range(col_start_2 - 1, col_start_1) = (in0[0+3].range(col_start_2 - col_start_f - 1, col_start_1 - col_start_f) | in0[0+0].range(col_start_2 - col_start_1 - 1, col_start_1 - col_start_1) |
                                             in0[4+6].range(col_start_2 - col_start_0 - 1, col_start_1 - col_start_0) | in0[4+0].range(col_start_2 - col_start_1 - 1, col_start_1 - col_start_1));
  out.range(col_start_3 - 1, col_start_2) = (in0[0+0].range(col_start_3 - col_start_1 - 1, col_start_2 - col_start_1) |
                                             in0[4+0].range(col_start_3 - col_start_1 - 1, col_start_2 - col_start_1));
  out.range(col_start_4 - 1, col_start_3) = (in0[0+0].range(col_start_4 - col_start_1 - 1, col_start_3 - col_start_1) |
                                             in0[4+0].range(col_start_4 - col_start_1 - 1, col_start_3 - col_start_1) | in0[4+1].range(col_start_4 - col_start_3 - 1, col_start_3 - col_start_3));
  out.range(col_start_5 - 1, col_start_4) = (in0[0+0].range(col_start_5 - col_start_1 - 1, col_start_4 - col_start_1) |
                                             in0[4+1].range(col_start_5 - col_start_3 - 1, col_start_4 - col_start_3));
  out.range(col_start_6 - 1, col_start_5) = (in0[0+0].range(col_start_6 - col_start_1 - 1, col_start_5 - col_start_1) | in0[0+1].range(col_start_6 - col_start_5 - 1, col_start_5 - col_start_5) |
                                             in0[4+1].range(col_start_6 - col_start_3 - 1, col_start_5 - col_start_3) | in0[4+2].range(col_start_6 - col_start_5 - 1, col_start_5 - col_start_5));
  out.range(col_start_7 - 1, col_start_6) = (in0[0+1].range(col_start_7 - col_start_5 - 1, col_start_6 - col_start_5) |
                                             in0[4+2].range(col_start_7 - col_start_5 - 1, col_start_6 - col_start_5));
  out.range(col_start_8 - 1, col_start_7) = (in0[0+1].range(col_start_8 - col_start_5 - 1, col_start_7 - col_start_5) |
                                             in0[4+2].range(col_start_8 - col_start_5 - 1, col_start_7 - col_start_5) | in0[4+3].range(col_start_8 - col_start_7 - 1, col_start_7 - col_start_7));
  out.range(col_start_9 - 1, col_start_8) = (in0[0+1].range(col_start_9 - col_start_5 - 1, col_start_8 - col_start_5) |
                                             in0[4+3].range(col_start_9 - col_start_7 - 1, col_start_8 - col_start_7));
  out.range(col_start_10 - 1, col_start_9) = (in0[0+1].range(col_start_10 - col_start_5 - 1, col_start_9 - col_start_5) | in0[0+2].range(col_start_10 - col_start_9 - 1, col_start_9 - col_start_9) |
                                              in0[4+3].range(col_start_10 - col_start_7 - 1, col_start_9 - col_start_7) | in0[4+4].range(col_start_10 - col_start_9 - 1, col_start_9 - col_start_9));
  out.range(col_start_11 - 1, col_start_10) = (in0[0+2].range(col_start_11 - col_start_9 - 1, col_start_10 - col_start_9) |
                                               in0[4+4].range(col_start_11 - col_start_9 - 1, col_start_10 - col_start_9));
  out.range(col_start_12 - 1, col_start_11) = (in0[0+2].range(col_start_12 - col_start_9 - 1, col_start_11 - col_start_9) |
                                               in0[4+4].range(col_start_12 - col_start_9 - 1, col_start_11 - col_start_9) | in0[4+5].range(col_start_12 - col_start_11 - 1, col_start_11 - col_start_11));
  out.range(col_start_13 - 1, col_start_12) = (in0[0+2].range(col_start_13 - col_start_9 - 1, col_start_12 - col_start_9) |
                                               in0[4+5].range(col_start_13 - col_start_11 - 1, col_start_12 - col_start_11));
}

// _____________________________________________________________________________
// Perform loop over chambers and all the segments in the chambers.
// Fill the chamber image for each chamber, then join all the chamber images.

template <typename Zone, typename Timezone, typename Row, typename SecondRow=Row>
void zoning_row_op(
    const emtf_phi_t emtf_phi[model_config::n_in],
    const seg_zones_t seg_zones[model_config::n_in],
    const seg_tzones_t seg_tzones[model_config::n_in],
    const seg_valid_t seg_valid[model_config::n_in],
    zoning_out_t& zoning_out_row_i
) {

#pragma HLS PIPELINE II=zoning_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef typename details::chamber_category_traits<Row>::chamber_category chamber_category;
  const unsigned int N = details::num_chambers_traits<chamber_category>::value;
  const int the_zone = details::zone_traits<Zone>::value;
  const int the_tzone = details::timezone_traits<Timezone>::value;
  emtf_assert(N == 4 or N == 7 or N == 11);

  // Make lookup tables
  int chamber_id_table[N];
  details::init_table_op<N>(chamber_id_table, details::get_chamber_id_op<Row>());

  int chamber_id_table_2nd[N];
  details::init_table_op<N>(chamber_id_table_2nd, details::get_chamber_id_op<SecondRow>());

  int chamber_ph_init_table[N];
  details::init_table_op<N>(chamber_ph_init_table, details::get_chamber_ph_init_op<chamber_category>());

  //int chamber_ph_cover_table[N];
  //details::init_table_op<N>(chamber_ph_cover_table, details::get_chamber_ph_cover_op<chamber_category>());

  // single-chamber image and N-chamber image
  typedef ap_uint<details::chamber_img_bw> chamber_img_t;
  typedef ap_uint<details::chamber_img_joined_bw> chamber_img_joined_t;

  chamber_img_t chamber_images[N];
  chamber_img_t chamber_images_2nd[N];

#pragma HLS ARRAY_PARTITION variable=chamber_images complete dim=0
#pragma HLS ARRAY_PARTITION variable=chamber_images_2nd complete dim=0

  // Loop over chambers
  LOOP_I_1: for (unsigned i = 0; i < N; i++) {

#pragma HLS UNROLL

    //std::cout << "[DEBUG] z " << details::zone_traits<Zone>::value << " chamber: " << chamber_id_table[i] << " ph_init: " << chamber_ph_init_table[i] << " ph_cover: " << chamber_ph_cover_table[i] << std::endl;

    chamber_images[i] = 0;  // init as zero

    // Loop over segments
    LOOP_J_1: for (unsigned j = 0; j < num_emtf_segments; j++) {

#pragma HLS UNROLL

      const trk_seg_t iseg = (chamber_id_table[i] * num_emtf_segments) + j;
      emtf_assert(chamber_id_table[i] < num_emtf_chambers);
      emtf_assert(iseg < (num_emtf_chambers * num_emtf_segments));

      // Fill the chamber image. A pixel at (row, col) is set to 1 if a segment is present.
      // Use condition: (valid && is_same_zone && is_same_timezone)
      if (
          (seg_valid[iseg] == 1) and \
          (seg_zones[iseg][(num_emtf_zones - 1) - the_zone] == 1) and \
          (seg_tzones[iseg][(num_emtf_timezones - 1) - the_tzone] == 1)
      ) {
        // Truncate last 4 bits (i.e. divide by 16), subtract offset
        constexpr int bits_to_shift = emtf_img_col_factor_log2;
        const trk_col_t offset = chamber_ph_init_table[i];
        const trk_col_t col = (emtf_phi[iseg] >> bits_to_shift) - offset;
        emtf_assert((emtf_phi[iseg] >> bits_to_shift) >= offset);
        emtf_assert(col < chamber_img_t::width);
        //std::cout << "[DEBUG] chamber: " << chamber_id_table[i] << " segment: " << j << " emtf_phi: " << emtf_phi[iseg] << " col: " << col << " offset: " << offset << std::endl;

        chamber_images[i][col] = 1;  // set bit to 1
      }
    }  // end loop over segments

    if (!is_same<Row, SecondRow>::value) {  // enable if Row and SecondRow are different

      chamber_images_2nd[i] = 0;  // init as zero

      // Loop over segments
      LOOP_J_2: for (unsigned j = 0; j < num_emtf_segments; j++) {

#pragma HLS UNROLL

        const trk_seg_t iseg = (chamber_id_table_2nd[i] * num_emtf_segments) + j;
        emtf_assert(chamber_id_table_2nd[i] < num_emtf_chambers);
        emtf_assert(iseg < (num_emtf_chambers * num_emtf_segments));

        // Fill the chamber image. A pixel at (row, col) is set to 1 if a segment is present.
        // Use condition: (valid && is_same_zone && is_same_timezone)
        if (
            (seg_valid[iseg] == 1) and \
            (seg_zones[iseg][(num_emtf_zones - 1) - the_zone] == 1) and \
            (seg_tzones[iseg][(num_emtf_timezones - 1) - the_tzone] == 1)
        ) {
          // Truncate last 4 bits (i.e. divide by 16), subtract offset
          constexpr int bits_to_shift = emtf_img_col_factor_log2;
          const trk_col_t offset = chamber_ph_init_table[i];
          const trk_col_t col = (emtf_phi[iseg] >> bits_to_shift) - offset;
          emtf_assert((emtf_phi[iseg] >> bits_to_shift) >= offset);
          emtf_assert(col < chamber_img_t::width);
          //std::cout << "[DEBUG] chamber: " << chamber_id_table_2nd[i] << " segment: " << j << " emtf_phi: " << emtf_phi[iseg] << " col: " << col << " offset: " << offset << std::endl;

          chamber_images_2nd[i][col] = 1;  // set bit to 1
        }
      }  // end loop over segments
    }  // end if
  }  // end loop over chambers

  // Join chamber images. Adjust the image size before output.
  constexpr int col_start_joined = details::chamber_img_joined_col_start;
  constexpr int col_stop_joined = details::chamber_img_joined_col_stop;

  chamber_img_joined_t chamber_img_joined;
  chamber_img_joined_t chamber_img_joined_2nd;

  if (!is_same<Row, SecondRow>::value) {  // enable if Row and SecondRow are different
    if (is_same<chamber_category, m_10deg_chamber_tag>::value) {
      zoning_join_10deg_chamber_images_op(chamber_images, chamber_img_joined);
      zoning_join_10deg_chamber_images_op(chamber_images_2nd, chamber_img_joined_2nd);
    } else if (is_same<chamber_category, m_20deg_chamber_tag>::value) {
      zoning_join_20deg_chamber_images_op(chamber_images, chamber_img_joined);
      zoning_join_20deg_chamber_images_op(chamber_images_2nd, chamber_img_joined_2nd);
    } else if (is_same<chamber_category, m_20deg_ext_chamber_tag>::value) {
      zoning_join_20deg_ext_chamber_images_op(chamber_images, chamber_img_joined);
      zoning_join_20deg_ext_chamber_images_op(chamber_images_2nd, chamber_img_joined_2nd);
    }
    zoning_out_row_i = (chamber_img_joined.range(col_stop_joined, col_start_joined) |
                        chamber_img_joined_2nd.range(col_stop_joined, col_start_joined));

  } else {  // enable if Row and Second are identical
    if (is_same<chamber_category, m_10deg_chamber_tag>::value) {
      zoning_join_10deg_chamber_images_op(chamber_images, chamber_img_joined);
    } else if (is_same<chamber_category, m_20deg_chamber_tag>::value) {
      zoning_join_20deg_chamber_images_op(chamber_images, chamber_img_joined);
    } else if (is_same<chamber_category, m_20deg_ext_chamber_tag>::value) {
      zoning_join_20deg_ext_chamber_images_op(chamber_images, chamber_img_joined);
    }
    zoning_out_row_i = chamber_img_joined.range(col_stop_joined, col_start_joined);
  }  // end if-else
}

// _____________________________________________________________________________
// Zoning op

template <typename Zone, typename Timezone>
void zoning_op(
    const emtf_phi_t emtf_phi[model_config::n_in],
    const seg_zones_t seg_zones[model_config::n_in],
    const seg_tzones_t seg_tzones[model_config::n_in],
    const seg_valid_t seg_valid[model_config::n_in],
    zoning_out_t zoning_out[zoning_config::n_out]
) {

#pragma HLS PIPELINE II=zoning_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // Loop over the rows manually
  if (is_same<Zone, m_zone_0_tag>::value) {  // enable if Zone 0
    zoning_row_op<Zone, Timezone, m_zone_0_row_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[0]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[1]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_2_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[2]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_3_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[3]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_4_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[4]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_5_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[5]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_6_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[6]);
    zoning_row_op<Zone, Timezone, m_zone_0_row_7_0_tag, m_zone_0_row_7_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[7]);

  } else if (is_same<Zone, m_zone_1_tag>::value) {  // enable if Zone 1
    zoning_row_op<Zone, Timezone, m_zone_1_row_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[0]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[1]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_2_0_tag, m_zone_1_row_2_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[2]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_3_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[3]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_4_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[4]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_5_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[5]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_6_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[6]);
    zoning_row_op<Zone, Timezone, m_zone_1_row_7_0_tag, m_zone_1_row_7_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[7]);

  } else if (is_same<Zone, m_zone_2_tag>::value) {  // enable if Zone 2
    zoning_row_op<Zone, Timezone, m_zone_2_row_0_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[0]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_1_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[1]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_2_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[2]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_3_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[3]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_4_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[4]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_5_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[5]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_6_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[6]);
    zoning_row_op<Zone, Timezone, m_zone_2_row_7_tag>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out[7]);
  }
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void zoning_layer(
    const emtf_phi_t emtf_phi[model_config::n_in],
    const seg_zones_t seg_zones[model_config::n_in],
    const seg_tzones_t seg_tzones[model_config::n_in],
    const seg_valid_t seg_valid[model_config::n_in],
    zoning_out_t zoning_out[zoning_config::n_out]
) {

#pragma HLS PIPELINE II=zoning_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS LATENCY max=zoning_config::target_lat

  // Check assumptions
  static_assert(zoning_config::n_out == num_emtf_img_rows, "zoning_config::n_out check failed");
  static_assert(num_emtf_img_rows == 8, "num_emtf_img_rows must be 8");
  static_assert(num_emtf_img_cols == 288, "num_emtf_img_cols must be 288");

  typedef m_timezone_0_tag Timezone;  // default timezone

  zoning_op<Zone, Timezone>(emtf_phi, seg_zones, seg_tzones, seg_valid, zoning_out);
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_ZONING_H__ not defined
