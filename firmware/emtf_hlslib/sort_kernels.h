#ifndef __EMTF_HLSLIB_SORT_KERNELS_H__
#define __EMTF_HLSLIB_SORT_KERNELS_H__

#include <algorithm>  // provides std::stable_sort
#include <array>  // provides std::array

namespace emtf {

namespace details {

// Helper class for argsort
template <typename T1, typename T2>
struct argsort_pair {
  typedef T1 first_type;
  typedef T2 second_type;
  T1 first;
  T2 second;

  // Default constructor
  constexpr argsort_pair(): first(), second() {}
  // Constructor
  constexpr argsort_pair(const T1& a, const T2& b): first(a), second(b) {}
  // Copy constructor
  constexpr argsort_pair(const argsort_pair&) = default;
  // Copy assignment operator
  argsort_pair& operator=(const argsort_pair&) = default;
  // Move constructor
  constexpr argsort_pair(argsort_pair&&) = default;
  // Move assignment operator
  argsort_pair& operator=(argsort_pair&&) = default;
};

template <typename T>
void cpp_sort_four_op(
    const T& a0, const T& a1, const T& a2, const T& a3,
    T& r0, T& r1, T& r2, T& r3
) {
  // Reference implementation for sort_four_op() using std::stable_sort().
  std::array<T, 4> values = {a0, a1, a2, a3};
  auto cmp = [](const T& lhs, const T& rhs) -> bool {
    return lhs.second < rhs.second;
  };
  std::stable_sort(values.rbegin(), values.rend(), cmp);
  r0 = values[0];
  r1 = values[1];
  r2 = values[2];
  r3 = values[3];
}

template <typename T>
void sort_four_op(
    const T& a0, const T& a1, const T& a2, const T& a3,
    T& r0, T& r1, T& r2, T& r3
) {

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // The following implements the sort stage of the Batcher odd-even mergesort.
  // There are 4 wires (0..3 from top to bottom), and there are 3 stages (not counting
  // Stage 0 which is used as input).
  // At each stage, compare a pair of wires (connected by a vertical line in the diagram).
  // If the value of the top wire is less than that of the bottom wire, swap the wires.
  // Thus, the smallest element moves to the bottom. After Stage 3, all the elements are
  // sorted, descending from top to bottom.
  // Note that this is *not* a stable sorting network.
  //
  // 0         1         2         3
  // ----------o---------o-------------------
  //           |         |
  // ----------o---------|-o-------o---------
  //                     | |       |
  // ----------o---------o-|-------o---------
  //           |           |
  // ----------o-----------o-----------------

  // Stage 0: prepare 4 input wires
  const T tmp_0_0 = a0;
  const T tmp_0_1 = a1;
  const T tmp_0_2 = a2;
  const T tmp_0_3 = a3;

  // Stage 1: compare-swap if (wire[i] < wire[j]) swap(wire[j], wire[i])
  const T tmp_1_0 = (tmp_0_0.second < tmp_0_1.second) ? tmp_0_1 : tmp_0_0;
  const T tmp_1_1 = (tmp_0_0.second < tmp_0_1.second) ? tmp_0_0 : tmp_0_1;
  const T tmp_1_2 = (tmp_0_2.second < tmp_0_3.second) ? tmp_0_3 : tmp_0_2;
  const T tmp_1_3 = (tmp_0_2.second < tmp_0_3.second) ? tmp_0_2 : tmp_0_3;

  // Stage 2
  const T tmp_2_0 = (tmp_1_0.second < tmp_1_2.second) ? tmp_1_2 : tmp_1_0;
  const T tmp_2_1 = (tmp_1_1.second < tmp_1_3.second) ? tmp_1_3 : tmp_1_1;
  const T tmp_2_2 = (tmp_1_0.second < tmp_1_2.second) ? tmp_1_0 : tmp_1_2;
  const T tmp_2_3 = (tmp_1_1.second < tmp_1_3.second) ? tmp_1_1 : tmp_1_3;

  // Stage 3
  const T tmp_3_1 = (tmp_2_1.second < tmp_2_2.second) ? tmp_2_2 : tmp_2_1;
  const T tmp_3_2 = (tmp_2_1.second < tmp_2_2.second) ? tmp_2_1 : tmp_2_2;

  // Output
  r0 = tmp_2_0;
  r1 = tmp_3_1;
  r2 = tmp_3_2;
  r3 = tmp_2_3;
}

template <typename T>
void cpp_merge_eight_op(
    const T& a0, const T& a1, const T& a2, const T& a3,
    const T& b0, const T& b1, const T& b2, const T& b3,
    T& r0, T& r1, T& r2, T& r3
) {
  // Reference implementation for merge_eight_op() using std::stable_sort().
  std::array<T, 8> values = {a0, a1, a2, a3, b0, b1, b2, b3};
  auto cmp = [](const T& lhs, const T& rhs) -> bool {
    return lhs.second < rhs.second;
  };
  std::stable_sort(values.rbegin(), values.rend(), cmp);
  r0 = values[0];
  r1 = values[1];
  r2 = values[2];
  r3 = values[3];
}

template <typename T>
void merge_eight_op(
    const T& a0, const T& a1, const T& a2, const T& a3,
    const T& b0, const T& b1, const T& b2, const T& b3,
    T& r0, T& r1, T& r2, T& r3
) {

#pragma HLS PIPELINE II=zonesorting_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // The following implements the merge stage of the Batcher odd-even mergesort.
  // There are 8 wires (0..7 from top to bottom), and there are 3 stages (not counting
  // Stage 0 which is used as input). Each block of top 4 and bottom 4 wires is assumed
  // to be already sorted.
  // At each stage, compare a pair of wires (connected by a vertical line in the diagram).
  // If the value of the top wire is less than that of the bottom wire, swap the wires.
  // Thus, the smallest element moves to the bottom. After Stage 3, all the elements are
  // sorted, descending from top to bottom. The top 4 are sent out.
  // Note that this is *not* a stable sorting network.
  //
  // 0         1         2         3
  // ----------o-----------------------------
  //           |
  // ----------|-o-----------------o---------
  //           | |                 |
  // ----------|-|-o-----o---------o---------
  //           | | |     |
  // ----------|-|-|-o---|-o-------o---------
  //           | | | |   | |       |
  // ----------o-|-|-|---o-|-------o---------
  //             | | |     |
  // ------------o-|-|-----o-------o---------
  //               | |             |
  // --------------o-|-------------o---------
  //                 |
  // ----------------o-----------------------

  // Stage 0: prepare 8 input wires
  const T tmp_0_0 = a0;
  const T tmp_0_1 = a1;
  const T tmp_0_2 = a2;
  const T tmp_0_3 = a3;
  const T tmp_0_4 = b0;
  const T tmp_0_5 = b1;
  const T tmp_0_6 = b2;
  const T tmp_0_7 = b3;

  // Stage 1: compare-swap if (wire[i] < wire[j]) swap(wire[j], wire[i])
  const T tmp_1_0 = (tmp_0_0.second < tmp_0_4.second) ? tmp_0_4 : tmp_0_0;
  const T tmp_1_1 = (tmp_0_1.second < tmp_0_5.second) ? tmp_0_5 : tmp_0_1;
  const T tmp_1_2 = (tmp_0_2.second < tmp_0_6.second) ? tmp_0_6 : tmp_0_2;
  const T tmp_1_3 = (tmp_0_3.second < tmp_0_7.second) ? tmp_0_7 : tmp_0_3;
  const T tmp_1_4 = (tmp_0_0.second < tmp_0_4.second) ? tmp_0_0 : tmp_0_4;
  const T tmp_1_5 = (tmp_0_1.second < tmp_0_5.second) ? tmp_0_1 : tmp_0_5;
  //const T tmp_1_6 = (tmp_0_2.second < tmp_0_6.second) ? tmp_0_2 : tmp_0_6;  // unused
  //const T tmp_1_7 = (tmp_0_3.second < tmp_0_7.second) ? tmp_0_3 : tmp_0_7;  // unused

  // Stage 2
  const T tmp_2_2 = (tmp_1_2.second < tmp_1_4.second) ? tmp_1_4 : tmp_1_2;
  const T tmp_2_3 = (tmp_1_3.second < tmp_1_5.second) ? tmp_1_5 : tmp_1_3;
  const T tmp_2_4 = (tmp_1_2.second < tmp_1_4.second) ? tmp_1_2 : tmp_1_4;
  //const T tmp_2_5 = (tmp_1_3.second < tmp_1_5.second) ? tmp_1_3 : tmp_1_5;  // unused

  // Stage 3
  const T tmp_3_1 = (tmp_1_1.second < tmp_2_2.second) ? tmp_2_2 : tmp_1_1;
  const T tmp_3_2 = (tmp_1_1.second < tmp_2_2.second) ? tmp_1_1 : tmp_2_2;
  const T tmp_3_3 = (tmp_2_3.second < tmp_2_4.second) ? tmp_2_4 : tmp_2_3;
  //const T tmp_3_4 = (tmp_2_3.second < tmp_2_4.second) ? tmp_2_3 : tmp_2_4;  // unused
  //const T tmp_3_5 = (tmp_2_5.second < tmp_1_6.second) ? tmp_1_6 : tmp_2_5;  // unused
  //const T tmp_3_6 = (tmp_2_5.second < tmp_1_6.second) ? tmp_2_5 : tmp_1_6;  // unused

  // Output
  r0 = tmp_1_0;
  r1 = tmp_3_1;
  r2 = tmp_3_2;
  r3 = tmp_3_3;
  //r4 = tmp_3_4;  // unused
  //r5 = tmp_3_5;  // unused
  //r6 = tmp_3_6;  // unused
  //r7 = tmp_1_7;  // unused
}

template <typename T>
void max_of_seven_op(
    const T& a0, const T& a1, const T& a2, const T& a3,
    const T& a4, const T& a5, const T& a6, T& out
) {

#pragma HLS PIPELINE II=pooling_config::target_ii

#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INLINE

  // The following implements the reduce max of 7 inputs. It is almost identical to
  // a 3-level binary tree with 8 inputs, except that there is no need to compare
  // the first two nodes. After stage 3, the first node is the max.
  //
  // 0         1         2         3
  // --------------------o---------o---------
  //                     |         |
  // --------------------|---------|---------
  //                     |         |
  // ----------o---------o---------|---------
  //           |                   |
  // ----------o-------------------|---------
  //                               |
  // ----------o---------o---------o---------
  //           |         |
  // ----------o---------|-------------------
  //                     |
  // ----------o---------o-------------------
  //           |
  // ----------o-----------------------------

  // Stage 0: prepare 7 input wires
  const T tmp_0_0 = a0;
  //const T tmp_0_1 = a0;  // unused
  const T tmp_0_2 = a1;
  const T tmp_0_3 = a2;
  const T tmp_0_4 = a3;
  const T tmp_0_5 = a4;
  const T tmp_0_6 = a5;
  const T tmp_0_7 = a6;

  // Stage 1: compare-swap if (wire[i] < wire[j]) swap(wire[j], wire[i])
  const T tmp_1_2 = (tmp_0_2.second < tmp_0_3.second) ? tmp_0_3 : tmp_0_2;
  //const T tmp_1_3 = (tmp_0_2.second < tmp_0_3.second) ? tmp_0_2 : tmp_0_3;  // unused
  const T tmp_1_4 = (tmp_0_4.second < tmp_0_5.second) ? tmp_0_5 : tmp_0_4;
  //const T tmp_1_5 = (tmp_0_4.second < tmp_0_5.second) ? tmp_0_4 : tmp_0_5;  // unused
  const T tmp_1_6 = (tmp_0_6.second < tmp_0_7.second) ? tmp_0_7 : tmp_0_6;
  //const T tmp_1_7 = (tmp_0_6.second < tmp_0_7.second) ? tmp_0_6 : tmp_0_7;  // unused

  // Stage 2
  const T tmp_2_0 = (tmp_0_0.second < tmp_1_2.second) ? tmp_1_2 : tmp_0_0;
  //const T tmp_2_2 = (tmp_0_0.second < tmp_1_2.second) ? tmp_0_0 : tmp_1_2;  // unused
  const T tmp_2_4 = (tmp_1_4.second < tmp_1_6.second) ? tmp_1_6 : tmp_1_4;
  //const T tmp_2_6 = (tmp_1_4.second < tmp_1_6.second) ? tmp_1_4 : tmp_1_6;  // unused

  // Stage 3
  const T tmp_3_0 = (tmp_2_0.second < tmp_2_4.second) ? tmp_2_4 : tmp_2_0;
  //const T tmp_3_4 = (tmp_2_0.second < tmp_2_4.second) ? tmp_2_0 : tmp_2_4;  // unused

  // Output
  out = tmp_3_0;
}

}  // namespace details

}  // namespace emtf

#endif  // __EMTF_HLSLIB_SORT_KERNELS_H__ not defined
