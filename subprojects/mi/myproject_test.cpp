// Top function
#include "firmware/myproject.h"

// Test bench
#include "software/testbench.h"

using namespace testbench;

// Test inputs and outputs
const int features[10][40] = {
{
     0,    36,    16,    12,    12,     0,    12,    12,    20,     0,
     0,     0,     0,    -1,     0,    -1,     0,     0,     4,     0,
     0,     0,     0,     0,     0,    -1,     0,     0,     0,     0,
     0,     6,     5,     5,     6,     0,  1264,    72,    63,     0
},
{
   175,     0,    16,    19,    42,     0,     0,    34,    47,     0,
    13,   245,     1,     0,     0,    -1,     0,     0,     0,    -1,
     0,     0,     2,     3,    -6,     0,     0,    -4,     2,   -25,
     6,     0,     5,     5,     5,     6,  1504,    10,    63,     0
},
{
  -179,     0,    14,    55,    48,     0,     0,     0,    48,     0,
    -1,  -265,     0,     0,    -1,     0,     0,     0,     0,     0,
     0,     0,    -2,    -1,     3,     0,     2,     0,     0,    20,
     4,     0,     5,     6,     5,     6,  1728,     9,    61,     0
},
{
   -16,     0,     7,     8,    12,     0,     0,     0,     8,   -25,
     8,     0,    -2,     0,    -1,     0,     0,     0,     0,     0,
     2,    -1,     1,     0,     0,     0,     5,     0,     2,     0,
     5,     0,     6,     6,     6,     0, -1328,    34,    61,     0
},
{
   309,     0,     0,   187,     0,     0,     0,   308,   516,     0,
    13,     0,     5,     0,     0,     0,     0,     0,     0,     3,
    -1,     0,     3,     0,    -8,     0,     0,     6,     0,     0,
     5,     0,     0,     6,     0,     0, -1104,    37,    54,     0
},
{
   -47,     0,     6,    16,     0,     0,     0,    17,    24,   -53,
    -2,   -60,     1,     0,     0,     0,     0,     0,     0,     1,
     1,     2,     2,     2,     0,     0,     1,     0,     0,     5,
     6,     0,     5,     4,     0,     6,   672,    16,    63,     0
},
{
     0,   135,    22,    18,    36,   116,    48,    32,    32,     0,
     0,     0,     0,     1,     0,     0,    -1,     6,     2,    -2,
     2,     0,     0,     0,     0,   -10,    -3,     2,     0,     0,
     0,     5,     6,     6,     6,     0,  1104,    62,    63,     0
},
{
  -393,     0,     0,     0,     0,     0,     0,     0,     0,  -441,
    21,  -498,     1,     0,     0,     0,     0,     0,     0,     0,
     0,     2,     0,     4,    15,     0,     0,     0,     0,    36,
     3,     0,     0,     0,     0,     5,  1152,    19,    54,     0
},
{
     0,   -75,     0,     0,     0,   -36,     0,   -48,     0,     0,
     0,     0,     0,     7,     0,     0,     0,     4,     0,     0,
     0,     0,     0,     0,     0,    13,     0,     0,     0,     0,
     0,     5,     0,     0,     0,     0,    48,    48,    57,     0
},
{
    47,     0,     8,    -8,    -8,     0,     0,    -6,   -12,     0,
    10,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     2,     0,     0,     1,     0,     0,     0,     0,     0,    -4,
     5,     0,     6,     6,     6,     6,  -768,    12,    63,     0
}
};

const int predictions[10][1] = {
{
  470
},
{
 3207
},
{
-3228
},
{
 -229
},
{
 3103
},
{
 -625
},
{
 2220
},
{
-3659
},
{
-3307
},
{
  526
}
};

// Main driver
int main(int argc, char **argv) {
  int err = 0;
  std::string clr_info = "\033[1;34m";   // blue
  std::string clr_error = "\033[1;31m";  // red
  std::string clr_reset = "\033[0m";     // no format

  std::initializer_list<int> event_list = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  // Loop over events
  for (auto ievt : event_list) {
    std::cout << clr_info << "Processing event " << ievt << clr_reset << std::endl;

    // Initialize input and output
    top_in_t in0[TOP_N_IN];
    top_out_t out[TOP_N_OUT];
    top_out_t gold[TOP_N_OUT];
    std::copy(std::begin(features[ievt]), std::end(features[ievt]), in0);
    std::fill(std::begin(out), std::end(out), 0);
    std::copy(std::begin(predictions[ievt]), std::end(predictions[ievt]), gold);

    // Call the top function !!
    myproject(in0, out);

    // Compare with the expectation
    int ievt_err = count_mismatches(std::begin(out), std::end(out), std::begin(gold));
    err += ievt_err;

    // Print error info
    if (ievt_err) {
      std::cout << clr_error << "FAILED!" << clr_reset << std::endl;
      std::cout << "Got:" << std::endl;
      print_array(out);
      std::cout << std::endl;
      std::cout << "Expected:" << std::endl;
      print_array(gold);
      std::cout << std::endl;
      std::cout << "Mismatches: " << err << std::endl;
    }
  }  // end loop over events

  // Return 0 only if the results are correct
  return err;
}
