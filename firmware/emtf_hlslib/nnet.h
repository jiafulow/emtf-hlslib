#ifndef __EMTF_HLSLIB_NNET_H__
#define __EMTF_HLSLIB_NNET_H__

namespace emtf {

template <typename Category, typename T_IN, typename T_OUT>
void nnet_activation_op(
    const T_IN in0[details::nnet_num_outbound_nodes_traits<Category>::value],
    T_OUT out[details::nnet_num_outbound_nodes_traits<Category>::value]
) {

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  const int N = details::nnet_num_outbound_nodes_traits<Category>::value;

  details::vector_activate<N>(in0, out);
}

template <typename Category, typename T_IN, typename T_OUT>
void nnet_preprocessing_op(
    const T_IN in0[details::nnet_num_inbound_nodes_traits<Category>::value],
    T_OUT out[details::nnet_num_outbound_nodes_traits<Category>::value]
) {

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  typedef typename details::select_nnet_weight_type<Category>::type weight_t;
  const int M = details::nnet_num_inbound_nodes_traits<Category>::value;
  const int N = details::nnet_num_outbound_nodes_traits<Category>::value;

  int weights_int[N];
  details::init_table_op<N>(weights_int, details::get_nnet_weights_op<Category>());

  weight_t weights[N];
  details::init_nnet_weights_op<N>(weights_int, weights);

  ap_fixed<T_IN::width, T_IN::width> in0_cast[N];  // cast from ap_int to ap_fixed

#pragma HLS ARRAY_PARTITION variable=weights complete dim=0
#pragma HLS ARRAY_PARTITION variable=in0_cast complete dim=0

  emtf_assert(M == N);
  details::vector_cast<N>(in0, in0_cast);
  details::vector_normalize<N>(weights, in0_cast, out);
}

template <typename Category, typename T_IN, typename T_OUT>
void nnet_dense_op(
    const T_IN in0[details::nnet_num_inbound_nodes_traits<Category>::value],
    T_OUT out[details::nnet_num_outbound_nodes_traits<Category>::value]
) {

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  typedef typename details::select_nnet_weight_type<Category>::type weight_t;
  typedef typename details::select_nnet_weight_type<Category>::type bias_t;
  const int M = details::nnet_num_inbound_nodes_traits<Category>::value;
  const int N = details::nnet_num_outbound_nodes_traits<Category>::value;

  int weights_int[M * N];
  details::init_table_op<M * N>(weights_int, details::get_nnet_weights_op<Category>());

  int biases_int[N];
  details::init_table_op<N>(biases_int, details::get_nnet_biases_op<Category>());

  weight_t weights[M * N];
  details::init_nnet_weights_op<M * N>(weights_int, weights);

  bias_t biases[N];
  details::init_nnet_weights_op<N>(biases_int, biases);

#pragma HLS ARRAY_PARTITION variable=weights complete dim=0
#pragma HLS ARRAY_PARTITION variable=biases complete dim=0

  details::matrix_vector_mult_biasadd<M, N>(weights, in0, biases, out);
}

template <typename Category, typename T_IN, typename T_OUT>
void nnet_dense_final_op(
    const T_IN in0[details::nnet_num_inbound_nodes_traits<Category>::value],
    T_OUT out[details::nnet_num_outbound_nodes_traits<Category>::value]
) {

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

//#pragma HLS INLINE

  typedef typename details::select_nnet_weight_type<Category>::type weight_t;
  typedef typename details::select_nnet_weight_type<Category>::type bias_t;
  const int M = details::nnet_num_inbound_nodes_traits<Category>::value;
  const int N = details::nnet_num_outbound_nodes_traits<Category>::value;

  int weights_int[M * N];
  details::init_table_op<M * N>(weights_int, details::get_nnet_weights_op<Category>());

  int biases_int[N];
  details::init_table_op<N>(biases_int, details::get_nnet_biases_op<Category>());

  weight_t weights[M * N];
  details::init_nnet_weights_op<M * N>(weights_int, weights);

  bias_t biases[N];
  details::init_nnet_weights_op<N>(biases_int, biases);

#pragma HLS ARRAY_PARTITION variable=weights complete dim=0
#pragma HLS ARRAY_PARTITION variable=biases complete dim=0

  emtf_assert(N == 1);
  details::vector_vector_mult_biasadd<M>(weights, in0, biases[0], out[0]);
}


// _____________________________________________________________________________
// Neural network op

template <typename Zone, typename Timezone>
void nnet_op(
    const trk_feat_t   curr_trk_feat  [num_emtf_features],
    trk_invpt_t&       curr_trk_invpt ,
    trk_phi_t&         curr_trk_phi   ,
    trk_eta_t&         curr_trk_eta   ,
    trk_d0_t&          curr_trk_d0    ,
    trk_z0_t&          curr_trk_z0    ,
    trk_beta_t&        curr_trk_beta
) {

#pragma HLS PIPELINE II=nnet_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  typedef details::select_nnet_activation_type<m_nnet_0_layer_0_tag>::type layer_0_out_t;
  typedef details::select_nnet_activation_type<m_nnet_0_layer_1_tag>::type layer_1_out_t;
  typedef details::select_nnet_activation_type<m_nnet_0_layer_2_tag>::type layer_2_out_t;
  typedef details::select_nnet_activation_type<m_nnet_0_layer_3_tag>::type layer_3_out_t;
  typedef details::select_nnet_activation_type<m_nnet_0_layer_4_tag>::type layer_4_out_t;

  // Preactivation is only needed for the hidden layers
  typedef details::select_nnet_preactivation_type<m_nnet_0_layer_1_tag>::type layer_1_preact_t;
  typedef details::select_nnet_preactivation_type<m_nnet_0_layer_2_tag>::type layer_2_preact_t;
  typedef details::select_nnet_preactivation_type<m_nnet_0_layer_3_tag>::type layer_3_preact_t;

  const int n_layer_0 = details::nnet_num_outbound_nodes_traits<m_nnet_0_layer_0_tag>::value;
  const int n_layer_1 = details::nnet_num_outbound_nodes_traits<m_nnet_0_layer_1_tag>::value;
  const int n_layer_2 = details::nnet_num_outbound_nodes_traits<m_nnet_0_layer_2_tag>::value;
  const int n_layer_3 = details::nnet_num_outbound_nodes_traits<m_nnet_0_layer_3_tag>::value;
  const int n_layer_4 = details::nnet_num_outbound_nodes_traits<m_nnet_0_layer_4_tag>::value;

  layer_0_out_t layer_0_out[n_layer_0];
  layer_1_out_t layer_1_out[n_layer_1];
  layer_2_out_t layer_2_out[n_layer_2];
  layer_3_out_t layer_3_out[n_layer_3];
  layer_4_out_t layer_4_out[n_layer_4];

  layer_1_preact_t layer_1_preact[n_layer_1];
  layer_2_preact_t layer_2_preact[n_layer_2];
  layer_3_preact_t layer_3_preact[n_layer_3];

#pragma HLS ARRAY_PARTITION variable=layer_0_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=layer_1_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=layer_2_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=layer_3_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=layer_4_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=layer_1_preact complete dim=0
#pragma HLS ARRAY_PARTITION variable=layer_2_preact complete dim=0
#pragma HLS ARRAY_PARTITION variable=layer_3_preact complete dim=0

  // Layer 0 - preprocessing
  nnet_preprocessing_op<m_nnet_0_layer_0_tag>(curr_trk_feat, layer_0_out);

  // Layer 1 - dense + activation
  nnet_dense_op<m_nnet_0_layer_1_tag>(layer_0_out, layer_1_preact);
  nnet_activation_op<m_nnet_0_layer_1_tag>(layer_1_preact, layer_1_out);

  // Layer 2 - dense_1 + activation_1
  nnet_dense_op<m_nnet_0_layer_2_tag>(layer_1_out, layer_2_preact);
  nnet_activation_op<m_nnet_0_layer_2_tag>(layer_2_preact, layer_2_out);

  // Layer 3 - dense_2 + activation_2
  nnet_dense_op<m_nnet_0_layer_3_tag>(layer_2_out, layer_3_preact);
  nnet_activation_op<m_nnet_0_layer_3_tag>(layer_3_preact, layer_3_out);

  // Layer 4 - dense_final
  nnet_dense_final_op<m_nnet_0_layer_4_tag>(layer_3_out, layer_4_out);

  // Output
  curr_trk_invpt.range() = layer_4_out[0].range();
  curr_trk_phi.range()   = 0;
  curr_trk_eta.range()   = 0;
  curr_trk_d0.range()    = 0;
  curr_trk_z0.range()    = 0;
  curr_trk_beta.range()  = 0;

  //// Debug
  //std::cout << "[DEBUG] layer 0: ";
  //for (int i = 0; i < n_layer_0; i++) {
  //  std::cout << layer_0_out[i] * (1<<10) << " ";
  //}
  //std::cout << std::endl;
  //std::cout << "[DEBUG] layer 1: ";
  //for (int i = 0; i < n_layer_1; i++) {
  //  std::cout << layer_1_out[i] * (1<<13) << " ";
  //}
  //std::cout << std::endl;
  //std::cout << "[DEBUG] layer 2: ";
  //for (int i = 0; i < n_layer_2; i++) {
  //  std::cout << layer_2_out[i] * (1<<13) << " ";
  //}
  //std::cout << std::endl;
  //std::cout << "[DEBUG] layer 3: ";
  //for (int i = 0; i < n_layer_3; i++) {
  //  std::cout << layer_3_out[i] * (1<<13) << " ";
  //}
  //std::cout << std::endl;
  //std::cout << "[DEBUG] layer 4: ";
  //for (int i = 0; i < n_layer_4; i++) {
  //  std::cout << layer_4_out[i] * (1<<13) << " ";
  //}
  //std::cout << std::endl;
}

// _____________________________________________________________________________
// Entry point

template <typename Zone>
void nnet_layer(
    const trk_feat_t  trk_feat  [nnet_config::n_in * num_emtf_features],
    trk_invpt_t       trk_invpt [nnet_config::n_out],
    trk_phi_t         trk_phi   [nnet_config::n_out],
    trk_eta_t         trk_eta   [nnet_config::n_out],
    trk_d0_t          trk_d0    [nnet_config::n_out],
    trk_z0_t          trk_z0    [nnet_config::n_out],
    trk_beta_t        trk_beta  [nnet_config::n_out]
) {

#pragma HLS PIPELINE II=nnet_config::layer_target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS LATENCY max=nnet_config::target_lat

  // Check assumptions
  static_assert(nnet_config::n_in == num_emtf_tracks, "nnet_config::n_in check failed");
  static_assert(nnet_config::n_out == num_emtf_tracks, "nnet_config::n_out check failed");
  static_assert(num_emtf_features == 40, "num_emtf_features must be 40");

  typedef m_timezone_0_tag Timezone;  // default timezone

  // Loop over tracks
  LOOP_TRK: for (unsigned itrk = 0; itrk < nnet_config::n_in; itrk++) {

#pragma HLS UNROLL

    const auto curr_trk_feat = &(trk_feat[itrk * num_emtf_features]);

    nnet_op<Zone, Timezone>(
        curr_trk_feat, trk_invpt[itrk], trk_phi[itrk], trk_eta[itrk],
        trk_d0[itrk], trk_z0[itrk], trk_beta[itrk]
    );
  }  // end loop over tracks
}

}  // namespace emtf

#endif  // __EMTF_HLSLIB_NNET_H__ not defined
