#ifndef __EMTF_HLSLIB_TRAITS_H__
#define __EMTF_HLSLIB_TRAITS_H__

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace emtf {

// _____________________________________________________________________________
// Mock implementation of C++ type_traits for use in the HLS project

template <typename T, T v>
struct integral_constant { static const T value = v; };

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template <bool B, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> { typedef T type; };

template <bool B, typename T, typename F>
struct conditional {};

template <typename T, typename F>
struct conditional<true, T, F> { typedef T type; };

template <typename T, typename F>
struct conditional<false, T, F> { typedef F type; };

template <typename T, typename U>
struct is_same : false_type {};

template <typename T>
struct is_same<T, T> : true_type {};

// _____________________________________________________________________________
// Check for ap datatype
template <typename T>
struct is_ap_int_type : false_type {};

template <int N> struct is_ap_int_type<ap_int<N> > : true_type {};  // toggle to true
template <int N> struct is_ap_int_type<ap_uint<N> > : true_type {};

template <typename T>
struct is_ap_fixed_type : false_type {};

template <int W, int I> struct is_ap_fixed_type<ap_fixed<W, I> > : true_type {};  // toggle to true
template <int W, int I> struct is_ap_fixed_type<ap_ufixed<W, I> > : true_type {};

// Make ap datatype - ap_int<N> or ap_uint<N>
template <int N, bool S>
struct make_ap_int_type { typedef typename conditional<S, ap_int<N>, ap_uint<N> >::type type; };

// Make signed ap datatype
template <typename T>
struct make_signed { typedef T type; };

template <int N> struct make_signed<ap_uint<N> > { typedef ap_int<N> type; };

// Make unsigned ap datatype
template <typename T>
struct make_unsigned { typedef T type; };

template <int N> struct make_unsigned<ap_int<N> > { typedef ap_uint<N> type; };

// Make wider ap datatype
template <typename T>
struct make_wider { typedef T type; };

template <int N> struct make_wider<ap_int<N> > { typedef ap_int<N+1> type; };
template <int N> struct make_wider<ap_uint<N> > { typedef ap_uint<N+1> type; };

// Make narrower ap datatype
template <typename T>
struct make_narrower { typedef T type; };

template <int N> struct make_narrower<ap_int<N> > { typedef ap_int<N-1> type; };
template <int N> struct make_narrower<ap_uint<N> > { typedef ap_uint<N-1> type; };

// Make concatenated ap datatype
template <typename T, typename U>
struct make_concat {};

template <int M, int N> struct make_concat<ap_int<M>, ap_int<N> > { typedef ap_int<M+N> type; };
template <int M, int N> struct make_concat<ap_uint<M>, ap_uint<N> > { typedef ap_uint<M+N> type; };

// Make repeated ap datatype
template <typename T, int U>
struct make_repeat {};

template <int M, int N> struct make_repeat<ap_int<M>, N> { typedef ap_int<M*N> type; };
template <int M, int N> struct make_repeat<ap_uint<M>, N> { typedef ap_uint<M*N> type; };

// Find max allowed value for ap datatype
template <typename T>
struct find_ap_int_max_allowed {};

template <int N> struct find_ap_int_max_allowed<ap_int<N> > { static const int value = ((1 << N) / 2) - 1; };
template <int N> struct find_ap_int_max_allowed<ap_uint<N> > { static const int value = (1 << N) - 1; };

// Find fractional bit width for ap datatype
template <typename T>
struct find_ap_fixed_fwidth {};

template <int W, int I> struct find_ap_fixed_fwidth<ap_fixed<W, I> > { static const int value = W - I; };
template <int W, int I> struct find_ap_fixed_fwidth<ap_ufixed<W, I> > { static const int value = W - I; };

}  // namespace emtf

#endif  // __EMTF_HLSLIB_TRAITS_H__ not defined
