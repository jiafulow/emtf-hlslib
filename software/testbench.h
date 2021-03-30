#ifndef __TESTBENCH_H__
#define __TESTBENCH_H__

#include <cassert>
#include <cstdint>
#include <array>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

namespace testbench {

// Hits contains 15 integer values.
struct HitsType {
  typedef int type;
  static const unsigned int len = 15;
};
typedef std::array<HitsType::type, HitsType::len> Hits;

// Event contains a list of Hits objects.
struct EventType {
  typedef Hits type;
};
typedef std::vector<EventType::type> Event;

// Tracks contains 40 integer values.
struct TracksType {
  typedef int type;
  static const unsigned int len = 40;
};
typedef std::array<TracksType::type, TracksType::len> Tracks;

// Result contains a list of Tracks objects.
struct ResultType {
  typedef Tracks type;
};
typedef std::vector<ResultType::type> Result;

// FPGAEvent contains the input, which is a list of max num of chambers,
// with max num of segments per chamber.
struct FPGAEvent {
  static const unsigned int num_chambers = emtf::num_emtf_chambers;
  static const unsigned int num_segments = emtf::num_emtf_segments;
  static const unsigned int num_variables = emtf::num_emtf_variables;
  static const unsigned int len = num_chambers * num_segments;

  emtf::model_in_t data[len];

  struct ArrayIndex {
    inline unsigned int operator ()(unsigned int emtf_chamber, unsigned int emtf_segment) const {
      assert(emtf_chamber < num_chambers);
      assert(emtf_segment < num_segments);
      return (emtf_chamber * num_segments) + emtf_segment;
    }
    inline int operator ()(int emtf_chamber, int emtf_segment) const {
      assert(static_cast<unsigned int>(emtf_chamber) < num_chambers);
      assert(static_cast<unsigned int>(emtf_segment) < num_segments);
      return (emtf_chamber * num_segments) + emtf_segment;
    }
  };

  // Constructor
  explicit FPGAEvent(const Event& evt) {
    assert(num_variables == 13);

    // Initialize
    for (unsigned iseg = 0; iseg < len; iseg++) {
      data[iseg] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // 13 zeros
    }

    // Fill values
    // This is basically converting sparsified data into unsparsified
    auto index_fn = ArrayIndex();

    for (unsigned ihit = 0; ihit < evt.size(); ihit++) {
      const int emtf_chamber = evt[ihit][0];
      const int emtf_segment = evt[ihit][1];
      const int iseg = index_fn(emtf_chamber, emtf_segment);

      data[iseg].emtf_phi    = evt[ihit][2];
      data[iseg].emtf_bend   = evt[ihit][3];
      data[iseg].emtf_theta1 = evt[ihit][4];
      data[iseg].emtf_theta2 = evt[ihit][5];
      data[iseg].emtf_qual1  = evt[ihit][6];
      data[iseg].emtf_qual2  = evt[ihit][7];
      data[iseg].emtf_time   = evt[ihit][8];
      data[iseg].seg_zones   = evt[ihit][9];
      data[iseg].seg_tzones  = evt[ihit][10];
      data[iseg].seg_fr      = evt[ihit][11];
      data[iseg].seg_dl      = evt[ihit][12];
      data[iseg].seg_bx      = evt[ihit][13];
      data[iseg].seg_valid   = evt[ihit][14];
    }
  }  // end constructor
};

// FPGAResult contains the output that is going to be sent to the NN.
// It is a list of max num of tracks, with fixed num of features per track.
struct FPGAResult {
  static const unsigned int num_tracks = emtf::num_emtf_tracks;
  static const unsigned int num_features = emtf::num_emtf_features;
  static const unsigned int len = num_tracks * num_features;

  emtf::model_out_t data[len];

  struct ArrayIndex {
    inline unsigned int operator ()(unsigned int track, unsigned int feature) const {
      assert(track < num_tracks);
      assert(feature < num_features);
      return (track * num_features) + feature;
    }
    inline int operator ()(int track, int feature) const {
      assert(static_cast<unsigned int>(track) < num_tracks);
      assert(static_cast<unsigned int>(feature) < num_features);
      return (track * num_features) + feature;
    }
  };

  // Constructor
  explicit FPGAResult(const Result& res) {
    assert(res.size() == num_tracks);

    // Initialize
    for (unsigned i = 0; i < len; i++) {
      data[i] = 0;
    }

    // Fill values
    auto index_fn = ArrayIndex();

    for (unsigned itrk = 0; itrk < num_tracks; itrk++) {
      for (unsigned ifeat = 0; ifeat < num_features; ifeat++) {
        const unsigned i = index_fn(itrk, ifeat);
        data[i] = res[itrk][ifeat];
      }
    }
  }  // end constructor
};


// _____________________________________________________________________________
// Read test bench text files.
// Note that the text files are whitespace-sensitive. Removing linebreaks or
// changing num of spaces can break this stupid function.
template <typename T>
int read_tb_data(const std::string filename, T& evt) {
  typename T::value_type line_buf;  // it should be a std::array

  std::string line;  // line in file
  char c;            // delimiter in line
  bool first_line = true;
  bool debug = false;

  std::ifstream infile(filename);

  if (infile.is_open()) {
    while (std::getline(infile, line)) {  // split by line break
      // There is a line break after every 16 ints. Join these as a single line
      if (line_buf.size() >= 16) {
        for (unsigned i = 16; i < line_buf.size(); i += 16) {
          std::string tmp_line;
          std::getline(infile, tmp_line);
          line += tmp_line;
        }
      }

      // Parse the line
      std::istringstream ss(line);
      if (first_line) {
        ss >> c;  // get rid of opening '['
        first_line = false;
      }
      ss >> c;  // get rid of '['
      for (unsigned i = 0; i < line_buf.size(); i++) {
        ss >> line_buf[i] >> c;  // extract int, then get rid of ','
      }
      ss >> c;  // get rid of ']'
      ss >> c;  // get rid of ',' or ending ']'

      // Append
      evt.emplace_back(line_buf);

      // Debug
      if (debug) {
        std::cout << "Line: " << line << std::endl;
        std::cout << "Parsed line: ";
        for (unsigned i = 0; i < line_buf.size(); i++) {
          std::cout << line_buf[i] << " ";
        }
        std::cout << std::endl;
      }
    }
  } else {
    std::cerr << "Failed to open file: " << filename << std::endl;
  }
  infile.close();
  return 0;
}


// _____________________________________________________________________________
// Initialize array to zeros
template <typename T, size_t N>
void init_array_as_zeros(T (&arr)[N]) {
  std::fill(arr, arr + N, 0);
}

// Initialize array to ones
template <typename T, size_t N>
void init_array_as_ones(T (&arr)[N]) {
  std::fill(arr, arr + N, 1);
}

// Initialize array to ones
template <typename T, size_t N>
void copy_array(T const (&arr1)[N], T (&arr2)[N]) {
  std::copy(arr1, arr1 + N, arr2);
}

// Print plain array
template <typename T, size_t N>
void print_array(T const (&arr)[N]) {
  std::copy(arr, arr + N, std::ostream_iterator<T>(std::cout, ", "));
}

// Print std::array, std::vector
template <typename T>
void print_std_array(T const& arr) {
  std::copy(std::begin(arr), std::end(arr), std::ostream_iterator<typename T::value_type>(std::cout, ", "));
}

// Get array length
template <typename T, size_t N>
size_t get_array_length(T const (&arr)[N]) {
  return N;
}

// Count mismatches
template <typename InputIt1, typename InputIt2>
int count_mismatches(InputIt1 first1, InputIt1 last1, InputIt2 first2) {
  int cnt = 0;
  for (; first1 != last1; ++first1, ++first2) {
    if (*first1 != *first2) {
      ++cnt;
    }
  }
  return cnt;
}

}  // namespace testbench

#endif  // __TESTBENCH_H__ not defined
