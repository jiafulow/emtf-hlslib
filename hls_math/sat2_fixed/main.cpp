#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

// Xilinx HLS
#include "ap_int.h"
#include "ap_fixed.h"

namespace {

constexpr bool debug = false;

// ap_fixed<W1, I1> is source datatype, ap_fixed<W2, I2> is target datatype
template <int W1, int I1, int W2, int I2>
ap_fixed<W2, I2> sat2_fixed(ap_fixed<W1, I1> x, ap_fixed<W2, I2> unused) {
  constexpr int F1 = W1 - I1;
  constexpr int F2 = W2 - I2;
  // The amount of bits shifted out (quantization or overflow)
  constexpr int FSHR = AP_MAX(F1 - F2, 0);
  constexpr int FSHL = AP_MAX(F2 - F1, 0);
  constexpr int ISHL = AP_MAX(I1 - I2, 0);

  // Handle quantization
  ap_uint<W2> target = 0;
  bool carry = false;

  if ((F1 - F2) <= 0) {  // move bits left
    if (FSHL >= W2) {    // shift amount is more than target width
      // Do nothing
    } else if ((W2 - 1) < (W1 + FSHL)) {  // source width is more than target width
      target.range(W2 - 1, FSHL) = x.range(W2 - FSHL - 1, 0);
      // carry = ((x[W1 - 1] == 0) and (target[W2 - 1] == 1));  // set carry bit
    } else {  // source width is less than or equal to target width
      target.range(W1 + FSHL - 1, FSHL) = x.range();
      if (x[W1 - 1] == 1) {  // sign extension
        target.range(W2 - 1, W1 + FSHL) = -1;
      }
    }
  } else {  // move bits right
    // Add 0.5 to the LSB
    ap_fixed<W1, I1> half_lsb = 0;
    half_lsb[FSHR - 1] = 1;
    ap_fixed<W1 + 1, I1 + 1> x_rnd = x + half_lsb;

    if (FSHR >= W1) {  // shift amount is more than source width
      // Round toward positive infinity, so sign extension is not needed
      // if (x[W1 - 1] == 1) {  // sign extension
      //   target.range() = -1;
      // }
    } else if ((W2 - 1) < (W1 - FSHR)) {  // source width is more than target width
      target.range() = x_rnd.range(W2 + FSHR - 1, FSHR);
      carry = ((x_rnd[W1 - 0] == 0) and (target[W2 - 1] == 1));  // set carry bit
    } else {  // source width is less than or equal to target width
      target.range(W1 - FSHR - 1, 0) = x_rnd.range(W1 - 1, FSHR);
      if (x_rnd[W1 - 0] == 1) {  // sign extension
        target.range(W2 - 1, W1 - FSHR) = -1;
      }
    }
  }

  // Handle overflow/underflow
  if ((I1 - I2) <= 0) {
    // Saturate
    const ap_uint<W2> min = (1 << (W2 - 1));
    const ap_uint<W2> max = ~min;
    target = carry ? max : target;
  } else {
    if (ISHL >= W1) {  // shift amount is more than source width
      // Fill with zero
      target = 0;
    } else {
      const ap_uint<ISHL + 1> deleted = x.range(W1 - 1, W1 - ISHL - 1);
      const ap_uint<ISHL + 1> zero = 0;
      const ap_uint<ISHL + 1> mask = ~zero;
      // Saturate
      const ap_uint<W2> min = (1 << (W2 - 1));
      const ap_uint<W2> max = ~min;
      const bool overflow = ((x[W1 - 1] == 0) and (deleted != zero));
      const bool underflow = ((x[W1 - 1] == 1) and (deleted != mask));
      target = (overflow or carry) ? max : underflow ? min : target;
    }
  }

  // Output
  ap_fixed<W2, I2> x_sat = 0;
  x_sat.range() = target;
  return x_sat;
}

template <typename T, typename C>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, C cmp) {
  return cmp(v, lo) ? lo : cmp(hi, v) ? hi : v;
}

template <typename T, typename R>
void sat2_fixed_test() {
  constexpr int N = (1 << T::width);

  T x = 0;
  for (unsigned i = 0; i < N; ++i) {
    x.range() = i;  // copy bits
    // Call function
    auto y = sat2_fixed(x, R{});
    if (debug)
      std::cout << x << " " << y << std::endl;
    ap_fixed<R::width, R::iwidth, AP_RND, AP_SAT> y_1 = x;
    // Check result
    float x_f32 = static_cast<float>(x);
    float y_lo_f32 = -std::ldexp(1.0f, R::width - 1) * std::ldexp(1.0f, -(R::width - R::iwidth));
    float y_hi_f32 = (std::ldexp(1.0f, R::width - 1) - 1.0f) * std::ldexp(1.0f, -(R::width - R::iwidth));
    float y_f32 = clamp(x_f32, y_lo_f32, y_hi_f32, [](const float& lhs, const float& rhs) { return lhs < rhs; });
    if ((R::width - R::iwidth) < (T::width - T::iwidth)) {
      float q_f32 = std::ldexp(1.0f, -(R::width - R::iwidth));
      y_f32 = q_f32 * std::floor(y_f32 / q_f32 + 0.5f);  // round toward positive infinity
    }
    float y_1_f32 = static_cast<float>(y_1);
    assert(y_lo_f32 < y_hi_f32);
    assert(y_f32 == y_1_f32);
    assert(y_f32 == static_cast<float>(y));
  }  // end loop
}

}  // anonymous namespace

// Main function
int main() {
  std::cout << "Test 1 ...";
  typedef ap_fixed<4, 2> T1;
  typedef ap_fixed<4, 2> R1;
  sat2_fixed_test<T1, R1>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 2 ...";
  typedef ap_fixed<4, 2> T2;
  typedef ap_fixed<4, 1> R2;
  sat2_fixed_test<T2, R2>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 3 ...";
  typedef ap_fixed<4, 2> T3;
  typedef ap_fixed<4, 0> R3;
  sat2_fixed_test<T3, R3>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 4 ...";
  typedef ap_fixed<4, 2> T4;
  typedef ap_fixed<3, 1> R4;
  sat2_fixed_test<T4, R4>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 5 ...";
  typedef ap_fixed<4, 2> T5;
  typedef ap_fixed<2, 1> R5;
  sat2_fixed_test<T5, R5>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 6 ...";
  typedef ap_fixed<4, 2> T6;
  typedef ap_fixed<4, 3> R6;
  sat2_fixed_test<T6, R6>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 7 ...";
  typedef ap_fixed<4, 2> T7;
  typedef ap_fixed<5, 3> R7;
  sat2_fixed_test<T7, R7>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 8 ...";
  typedef ap_fixed<4, -2> T8;
  typedef ap_fixed<4, -4> R8;
  sat2_fixed_test<T8, R8>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 9 ...";
  typedef ap_fixed<4, -2> T9;
  typedef ap_fixed<2, -2> R9;
  sat2_fixed_test<T9, R9>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 10 ...";
  typedef ap_fixed<4, -2> T10;
  typedef ap_fixed<2, -4> R10;
  sat2_fixed_test<T10, R10>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 11 ...";
  typedef ap_fixed<4, -2> T11;
  typedef ap_fixed<2, 0> R11;
  sat2_fixed_test<T11, R11>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 12 ...";
  typedef ap_fixed<4, -2> T12;
  typedef ap_fixed<6, 0> R12;
  sat2_fixed_test<T12, R12>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 13 ...";
  typedef ap_fixed<4, -2> T13;
  typedef ap_fixed<6, -4> R13;
  sat2_fixed_test<T13, R13>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 14 ...";
  typedef ap_fixed<4, 0> T14;
  typedef ap_fixed<2, -1> R14;
  sat2_fixed_test<T14, R14>();
  std::cout << " OK" << std::endl;

  std::cout << "Test 15 ...";
  typedef ap_fixed<4, 0> T15;
  typedef ap_fixed<4, 4> R15;
  sat2_fixed_test<T15, R15>();
  std::cout << " OK" << std::endl;

  std::cout << "All OK" << std::endl;
  return 0;
}
