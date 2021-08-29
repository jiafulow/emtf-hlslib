#ifndef __EMTF_HLSLIB_TRAITS_H__
#define __EMTF_HLSLIB_TRAITS_H__

#include <cfloat>  // provides FLT_EPSILON, FLT_MANT_DIG

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace emtf_hlslib {

namespace phase2 {

// _____________________________________________________________________________
// Mock implementation of C++ type_traits for use in the HLS project
// - integral_constant
// - enable_if
// - conditional
// - is_same
// - remove_cv
// - remove_pointer
// - remove_reference

template <typename T, T v>
struct integral_constant {
  constexpr static const T value = v;
  using value_type = T;
  using type = integral_constant;
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

template <typename T>
struct remove_cv {
  typedef T type;
};

template <typename T>
struct remove_cv<const T> {
  typedef T type;
};
template <typename T>
struct remove_cv<volatile T> {
  typedef T type;
};
template <typename T>
struct remove_cv<const volatile T> {
  typedef T type;
};

template <typename T>
struct remove_pointer {
  typedef T type;
};

template <typename T>
struct remove_pointer<T*> {
  typedef T type;
};
template <typename T>
struct remove_pointer<T* const> {
  typedef T type;
};

template <typename T>
struct remove_reference {
  typedef T type;
};

template <typename T>
struct remove_reference<T&> {
  typedef T type;
};
template <typename T>
struct remove_reference<T&&> {
  typedef T type;
};

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

// According to the UG902 manual, ap_[u]fixed is defined as:
// ap_[u]fixed<int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
#ifndef ap_sat_bits
typedef int ap_sat_bits;
#endif  // ap_sat_bits not defined

template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct is_ap_fixed_type<ap_fixed<W, I, Q, O, N> > : true_type {};  // toggle to true
template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct is_ap_fixed_type<ap_ufixed<W, I, Q, O, N> > : true_type {};

// _____________________________________________________________________________
// Make ap datatype - ap_int or ap_uint
template <int W, bool S>
struct make_ap_int_type {
  typedef typename conditional<S, ap_int<W>, ap_uint<W> >::type type;
};

// Make ap datatype - ap_fixed or ap_ufixed
template <int W, int I, bool S>
struct make_ap_fixed_type {
  typedef typename conditional<S, ap_fixed<W, I>, ap_ufixed<W, I> >::type type;
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
// Find type properties for ap datatype
template <typename T>
struct ap_int_props {};

template <int W>
struct ap_int_props<ap_int<W> > {
  constexpr static const bool is_integral_v = true;
  constexpr static const bool is_floating_point_v = false;
  constexpr static const bool is_fixed_point_v = false;
  constexpr static const bool is_arithmetic_v = true;
  constexpr static const bool is_signed_v = true;
};

template <int W>
struct ap_int_props<ap_uint<W> > {
  constexpr static const bool is_integral_v = true;
  constexpr static const bool is_floating_point_v = false;
  constexpr static const bool is_fixed_point_v = false;
  constexpr static const bool is_arithmetic_v = true;
  constexpr static const bool is_signed_v = false;
};

template <typename T>
struct ap_fixed_props {};

template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct ap_fixed_props<ap_fixed<W, I, Q, O, N> > {
  constexpr static const bool is_integral_v = false;
  constexpr static const bool is_floating_point_v = false;
  constexpr static const bool is_fixed_point_v = true;
  constexpr static const bool is_arithmetic_v = true;
  constexpr static const bool is_signed_v = true;
};

template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct ap_fixed_props<ap_ufixed<W, I, Q, O, N> > {
  constexpr static const bool is_integral_v = false;
  constexpr static const bool is_floating_point_v = false;
  constexpr static const bool is_fixed_point_v = true;
  constexpr static const bool is_arithmetic_v = true;
  constexpr static const bool is_signed_v = false;
};

// _____________________________________________________________________________
// Find widths of integer and fractional parts for ap datatype
template <typename T>
struct ap_int_widths {};

template <int W>
struct ap_int_widths<ap_int<W> > {
  constexpr static const int width = W;
  constexpr static const int iwidth = W;
  constexpr static const int fwidth = 0;
};

template <int W>
struct ap_int_widths<ap_uint<W> > {
  constexpr static const int width = W;
  constexpr static const int iwidth = W;
  constexpr static const int fwidth = 0;
};

template <typename T>
struct ap_fixed_widths {};

template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct ap_fixed_widths<ap_fixed<W, I, Q, O, N> > {
  constexpr static const int width = W;
  constexpr static const int iwidth = I;
  constexpr static const int fwidth = W - I;
};

template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct ap_fixed_widths<ap_ufixed<W, I, Q, O, N> > {
  constexpr static const int width = W;
  constexpr static const int iwidth = I;
  constexpr static const int fwidth = W - I;
};

// _____________________________________________________________________________
// Find min, max allowed values for ap datatype
template <typename T>
struct ap_int_limits {};

// The following implementation only works correctly up to 31 integer bits.
// For signed int, max_value = ~min_value and mask_value = (min_value | max_value).
// For unsigned int, min_value = 0, max_value = ~min_value and mask_value = max_value.
// For reference:
// - INT_MAX = 2147483647
// - INT_MIN = -2147483648
// - UINT_MAX = 4294967295
template <int W>
struct ap_int_limits<ap_int<W> > {
  constexpr static const int epsilon_value = 1;
  constexpr static const int max_value = ((1 << (W - 1)) - 1);
  constexpr static const int min_value = -(1 << (W - 1));
  constexpr static const int lowest_value = min_value;
  constexpr static const unsigned int mask_value = ((1u << W) - 1);
};

template <int W>
struct ap_int_limits<ap_uint<W> > {
  constexpr static const int epsilon_value = 1;
  constexpr static const int max_value = ((1 << W) - 1);
  constexpr static const int min_value = 0;
  constexpr static const int lowest_value = min_value;
  constexpr static const unsigned int mask_value = ((1u << W) - 1);
};

template <typename T>
struct ap_fixed_limits {};

// The following implementation only works correctly up to 31 integer bits and
// up to 23 fractional bits.
// For reference:
// - FLT_RADIX = 2, which means base-2 representation.
// - FLT_MANT_DIG = 24, which is number of bits in the mantissa, including
//   1 integer bit (implicit) and 23 fractional bits.
// - FLT_EPSILON = 1.19209e-07, which is std::ldexp(1.0f, -23).
template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct ap_fixed_limits<ap_fixed<W, I, Q, O, N> > {
  constexpr static const int _flt_mant_dig_sub_1 = FLT_MANT_DIG - 1;
  constexpr static const float _flt_epsilon = FLT_EPSILON;
  constexpr static const float epsilon_value = _flt_epsilon * (1 << (_flt_mant_dig_sub_1 - (W - I)));
  constexpr static const float max_value = epsilon_value * ((1 << (W - 1)) - 1);
  constexpr static const float min_value = epsilon_value * -(1 << (W - 1));
  constexpr static const float lowest_value = min_value;
  constexpr static const float mask_value = epsilon_value * ((1u << W) - 1);
};

template <int W, int I, ap_q_mode Q, ap_o_mode O, ap_sat_bits N>
struct ap_fixed_limits<ap_ufixed<W, I, Q, O, N> > {
  constexpr static const int _flt_mant_dig_sub_1 = FLT_MANT_DIG - 1;
  constexpr static const float _flt_epsilon = FLT_EPSILON;
  constexpr static const float epsilon_value = _flt_epsilon * (1 << (_flt_mant_dig_sub_1 - (W - I)));
  constexpr static const float max_value = epsilon_value * ((1 << W) - 1);
  constexpr static const float min_value = epsilon_value * 0;
  constexpr static const float lowest_value = min_value;
  constexpr static const float mask_value = epsilon_value * ((1u << W) - 1);
};

// _____________________________________________________________________________
// Make return types for arithmetic operations with ap dataype
// (only works for ap_fixed)
template <typename T, typename U>
struct make_arith_return_type {};

template <int W1, int I1, int W2, int I2>
struct make_arith_return_type<ap_fixed<W1, I1>, ap_fixed<W2, I2> > {
  constexpr static const int _W1 = W1;
  constexpr static const int _I1 = I1;
  constexpr static const int _F1 = W1 - I1;
  constexpr static const bool _S1 = true;
  constexpr static const int _W2 = W2;
  constexpr static const int _I2 = I2;
  constexpr static const int _F2 = W2 - I2;
  constexpr static const bool _S2 = true;

  typedef ap_fixed<AP_MAX(_I1 + (_S2 && !_S1), _I2 + (_S1 && !_S2)) + AP_MAX(_F1, _F2) + 1,
                   AP_MAX(_I1 + (_S2 && !_S1), _I2 + (_S1 && !_S2)) + 1>
      add_type;
  typedef add_type sub_type;
  typedef ap_fixed<add_type::width - 1, add_type::iwidth - 1> logic_type;
  typedef ap_fixed<_W1 + _W2, _I1 + _I2> mult_type;
  typedef ap_fixed<_W1 + AP_MAX(_I2, 0) + AP_MAX(_F2, 0) + (_S2 && !_S1), _I1 + _F2 + (_S2 && !_S1)> div_type;
  typedef ap_fixed<mult_type::width + 1, mult_type::iwidth + 1> madd_type;
};

}  // namespace phase2

}  // namespace emtf_hlslib

#endif  // __EMTF_HLSLIB_TRAITS_H__ not defined
