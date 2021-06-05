#ifndef __TESTBENCH_H__
#define __TESTBENCH_H__

#include <cassert>
#include <cstdint>
#include <cmath>
#include <array>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <numeric>

// EMTF HLS
#include "../firmware/emtf_hlslib/common.h"
#include "../firmware/emtf_hlslib/types.h"

namespace {
  using namespace emtf::phase2;
}

namespace testbench {

// Hits contains N integer values.
struct HitsType {
  typedef int type;
  static const unsigned int len = 15;  // num_emtf_variables + 2
};

typedef std::array<HitsType::type, HitsType::len> Hits;

// Event contains a list of Hits objects.
struct EventType {
  typedef Hits type;
};

typedef std::vector<EventType::type> Event;

// Tracks contains N integer values.
struct TracksType {
  typedef int type;
  static const unsigned int len = 54;  // num_emtf_features + num_emtf_sites + 2
};

typedef std::array<TracksType::type, TracksType::len> Tracks;

// Result contains a list of Tracks objects.
struct ResultType {
  typedef Tracks type;
};

typedef std::vector<ResultType::type> Result;

// Event & Result for subprojects (pr, fe, rm, mi)
typedef Event PrEvent;

struct PrResultType {
  typedef std::array<int, 4> type;  // trk_qual, trk_patt, trk_col, trk_zone
};

typedef std::vector<PrResultType::type> PrResult;

typedef PrResult FeEvent;

struct FeResultType {
  typedef std::array<int, 52> type;  // num_emtf_features + num_emtf_sites
};

typedef std::vector<FeResultType::type> FeResult;

typedef FeResult RmEvent;

typedef RmEvent RmResult;

struct MiEventType {
  typedef std::array<int, 40> type;  // num_emtf_features
};

typedef std::vector<MiEventType::type> MiEvent;

struct MiResultType {
  typedef std::array<int, 1> type;  // num_emtf_predictions
};

typedef std::vector<MiResultType::type> MiResult;

// FpgaEvent contains the input, which is a list of max num of chambers,
// with max num of segments per chamber.
struct FpgaEvent {
  static const unsigned int num_chambers = num_emtf_chambers;
  static const unsigned int num_segments = num_emtf_segments;
  static const unsigned int num_variables = num_emtf_variables;
  static const unsigned int len = (num_chambers * num_segments);

  model_in_t data[len];

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
  explicit FpgaEvent(const Event& evt) {
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

      // Accept at most 2 segments
      if (!(static_cast<unsigned int>(emtf_segment) < num_segments))
        continue;

      const int iseg = index_fn(emtf_chamber, emtf_segment);

      // 13 variables
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

      assert(data[iseg].seg_valid == 1);  // segment must be valid
    }
  }  // end constructor
};

// FpgaResult contains the output that is going to be sent to the NN.
// It is a list of max num of tracks, with fixed num of variables per track.
struct FpgaResult {
  static const unsigned int num_tracks = num_emtf_tracks;
  static const unsigned int num_trk_variables = (num_emtf_features + num_emtf_sites + 2);
  static const unsigned int len = (num_tracks * num_trk_variables);

  model_out_t data[len];

  struct ArrayIndex {
    inline unsigned int operator ()(unsigned int track, unsigned int variable) const {
      assert(track < num_tracks);
      assert(variable < num_trk_variables);
      return (track * num_trk_variables) + variable;
    }
    inline int operator ()(int track, int variable) const {
      assert(static_cast<unsigned int>(track) < num_tracks);
      assert(static_cast<unsigned int>(variable) < num_trk_variables);
      return (track * num_trk_variables) + variable;
    }
  };

  // Constructor
  explicit FpgaResult(const Result& res) {
    assert(res.size() == num_tracks);

    // Initialize
    for (unsigned i = 0; i < len; i++) {
      data[i] = 0;
    }

    // Fill values
    auto index_fn = ArrayIndex();

    for (unsigned itrk = 0; itrk < num_tracks; itrk++) {
      for (unsigned ivar = 0; ivar < num_trk_variables; ivar++) {
        const unsigned int i = index_fn(itrk, ivar);
        data[i] = res[itrk][ivar];
      }
    }
  }  // end constructor
};


// _____________________________________________________________________________
// Read test bench text files.
// Note that the text files are whitespace-sensitive. Removing linebreaks or
// changing num of spaces can break this stupid function. It is also assumed
// that the arrays printed in the text files are 2-D, thus each array has
// at least 2 '[' characters and at least 2 ']' characters.
template <typename T>
int read_tb_data(const std::string filename, T& evt) {
  typename T::value_type line_buf;  // it should be a std::array
  assert(line_buf.size() != 0);

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
        ss >> line_buf.at(i) >> c;  // extract int, then get rid of ','
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
          std::cout << line_buf.at(i) << " ";
        }
        std::cout << std::endl;
      }
    }
  } else {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return 1;
  }
  infile.close();
  return 0;
}


// _____________________________________________________________________________
// Print plain array
template <typename T, std::size_t N>
void print_array(T const (&arr)[N]) {
  std::copy(std::begin(arr), std::end(arr), std::ostream_iterator<T>(std::cout, ", "));
}

// Print std::array, std::vector
template <typename T>
void print_std_array(T const& arr) {
  std::copy(std::begin(arr), std::end(arr), std::ostream_iterator<typename T::value_type>(std::cout, ", "));
}

// Get array length
template <typename T, std::size_t N>
std::size_t get_array_length(T const (&arr)[N]) {
  return N;
}

// Count mismatches
template <typename InputIt1, typename InputIt2>
int count_mismatches(InputIt1 first1, InputIt1 last1, InputIt2 first2, int tol=0) {
  int cnt = 0;
  assert(tol >= 0);  // tol must be a positive integer
  for (; first1 != last1; ++first1, ++first2) {
    // Condition: !((gold0 <= x) and (x <= gold1))
    // If tol = 0, condition is simply (gold != x)
    if (!(((*first1 - tol) <= *first2) and (*first2 <= (*first1 + tol)))) {
      ++cnt;
    }
  }
  return cnt;
}

}  // namespace testbench

#endif  // __TESTBENCH_H__ not defined
