#ifndef __EMTF_HLSLIB_TRAITS_H__
#define __EMTF_HLSLIB_TRAITS_H__

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace emtf {

namespace phase2 {

// _____________________________________________________________________________
// Mock implementation of C++ type_traits for use in the HLS project
// - integral_constant
// - enable_if
// - conditional
// - is_same

template <typename T, T v>
struct integral_constant {
  static const T value = v;
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template <bool B, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
  typedef T type;
};

template <bool B, typename T, typename F>
struct conditional {};

template <typename T, typename F>
struct conditional<true, T, F> {
  typedef T type;
};

template <typename T, typename F>
struct conditional<false, T, F> {
  typedef F type;
};

template <typename T, typename U>
struct is_same : false_type {};

template <typename T>
struct is_same<T, T> : true_type {};

// _____________________________________________________________________________
// Check for ap datatype - ap_int or ap_uint
template <typename T>
struct is_ap_int_type : false_type {};

template <int W>
struct is_ap_int_type<ap_int<W> > : true_type {};  // toggle to true
template <int W>
struct is_ap_int_type<ap_uint<W> > : true_type {};

// Check for ap datatype - ap_fixed or ap_ufixed
template <typename T>
struct is_ap_fixed_type : false_type {};

template <int W, int I, ap_q_mode Q, ap_o_mode O>
struct is_ap_fixed_type<ap_fixed<W, I, Q, O> > : true_type {};  // toggle to true
template <int W, int I, ap_q_mode Q, ap_o_mode O>
struct is_ap_fixed_type<ap_ufixed<W, I, Q, O> > : true_type {};

// _____________________________________________________________________________
// Make ap datatype - ap_int or ap_uint
template <int W, bool S>
struct make_ap_int_type {
  typedef typename conditional<S, ap_int<W>, ap_uint<W> >::type type;
};

// Make ap datatype - ap_fixed or ap_ufixed
template <int W, int I, ap_q_mode Q, ap_o_mode O, bool S>
struct make_ap_fixed_type {
  typedef typename conditional<S, ap_fixed<W, I, Q, O>, ap_ufixed<W, I, Q, O> >::type type;
};

// Make signed ap datatype
template <typename T>
struct make_signed {
  typedef T type;
};

template <int W>
struct make_signed<ap_uint<W> > {
  typedef ap_int<W> type;
};
template <int W>
struct make_signed<const ap_uint<W> > {
  typedef const ap_int<W> type;
};

// Make unsigned ap datatype
template <typename T>
struct make_unsigned {
  typedef T type;
};

template <int W>
struct make_unsigned<ap_int<W> > {
  typedef ap_uint<W> type;
};
template <int W>
struct make_unsigned<const ap_int<W> > {
  typedef const ap_uint<W> type;
};

// Make wider ap datatype
template <typename T>
struct make_wider {
  typedef T type;
};

template <int W>
struct make_wider<ap_int<W> > {
  typedef ap_int<W + 1> type;
};
template <int W>
struct make_wider<ap_uint<W> > {
  typedef ap_uint<W + 1> type;
};
template <int W>
struct make_wider<const ap_int<W> > {
  typedef const ap_int<W + 1> type;
};
template <int W>
struct make_wider<const ap_uint<W> > {
  typedef const ap_uint<W + 1> type;
};

// Make narrower ap datatype
template <typename T>
struct make_narrower {
  typedef T type;
};

template <int W>
struct make_narrower<ap_int<W> > {
  typedef ap_int<W - 1> type;
};
template <int W>
struct make_narrower<ap_uint<W> > {
  typedef ap_uint<W - 1> type;
};
template <int W>
struct make_narrower<const ap_int<W> > {
  typedef const ap_int<W - 1> type;
};
template <int W>
struct make_narrower<const ap_uint<W> > {
  typedef const ap_uint<W - 1> type;
};

// Make concatenated ap datatype
template <typename T, typename U>
struct make_concat {};

template <int W1, int W2>
struct make_concat<ap_int<W1>, ap_int<W2> > {
  typedef ap_int<W1 + W2> type;
};
template <int W1, int W2>
struct make_concat<ap_uint<W1>, ap_uint<W2> > {
  typedef ap_uint<W1 + W2> type;
};

// Make repeated ap datatype
template <typename T, int U>
struct make_repeat {};

template <int W, int N>
struct make_repeat<ap_int<W>, N> {
  typedef ap_int<W * N> type;
};
template <int W, int N>
struct make_repeat<ap_uint<W>, N> {
  typedef ap_uint<W * N> type;
};

// _____________________________________________________________________________
// Find max allowed value for ap datatype
template <typename T>
struct find_ap_int_max_allowed {};

template <int W>
struct find_ap_int_max_allowed<ap_int<W> > {
  static const int value = ((1 << W) / 2) - 1;
};
template <int W>
struct find_ap_int_max_allowed<ap_uint<W> > {
  static const int value = (1 << W) - 1;
};

}  // namespace phase2

}  // namespace emtf

#endif  // __EMTF_HLSLIB_TRAITS_H__ not defined
