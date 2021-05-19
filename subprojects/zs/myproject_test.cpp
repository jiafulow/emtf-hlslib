// Top function
#include "firmware/myproject.h"

// Test bench
#include "software/testbench.h"

using namespace testbench;

void sanity_check() {
}

// Main driver
int main(int argc, char **argv) {
  // Perform sanity check
  sanity_check();

  int err = 0;
  std::string clr_info = "\033[1;34m";   // blue
  std::string clr_error = "\033[1;31m";  // red
  std::string clr_reset = "\033[0m";     // no format

  // List of event numbers
  std::initializer_list<int> event_list = {0};
  //std::vector<int> event_list(100);
  //std::iota(event_list.begin(), event_list.end(), 0);

  // Loop over events
  for (auto ievt : event_list) {
    std::cout << clr_info << "Processing event " << ievt << clr_reset << std::endl;

    // Create evt_flat & res_flat (hardcoded)
    std::vector<PrEvent::value_type::value_type> evt_flat;
    std::vector<PrResult::value_type::value_type> res_flat;
    evt_flat = {
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 402, 402, 402,
     402, 410, 410, 410, 410, 410, 410, 410, 410, 410, 422, 422, 422, 422, 422, 422,
     422, 422, 422, 422, 294, 294, 294, 294, 294, 294, 154, 166, 166, 166,  26,  26,
      26,  38,  38,  38,  26, 102, 102, 102, 102, 235, 239, 255, 255, 255, 254, 382,
     375, 375, 375, 375, 375, 375, 375, 375, 375, 367, 363, 363, 363, 363, 363, 346,
     346, 346,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
    };
    res_flat = {71423, 63142, 66086, 68198};

    // Initialize input & output
    top_in_t in0[TOP_N_IN];
    top_out_t out[TOP_N_OUT];
    std::copy(std::begin(evt_flat), std::end(evt_flat), std::begin(in0));
    std::fill(std::begin(out), std::end(out), 0);  // init as zeros

    // Call the top function !!
    myproject(in0, out);

    // Compare with the expectation
    int ievt_err = count_mismatches(std::begin(res_flat), std::end(res_flat), std::begin(out));
    err += ievt_err;

    // Print error info
    if (ievt_err) {
      std::cout << clr_error << "FAILED!" << clr_reset << std::endl;
      std::cout << "Got:" << std::endl;
      print_array(out);
      std::cout << std::endl;
      std::cout << "Expected:" << std::endl;
      print_std_array(res_flat);
      std::cout << std::endl;
      std::cout << "Mismatches: " << err << std::endl;
    }
  }  // end loop over events

  // Return 0 only if the results are correct
  return err;
}
