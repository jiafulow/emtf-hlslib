// Top function
#include "firmware/myproject.h"

// Test bench
#include "software/testbench.h"

using namespace testbench;

void sanity_check() {
  static_assert(HitsType::len == emtf::num_emtf_variables + 2, "Inconsistent HitsType::len");
  static_assert(TracksType::len == emtf::num_emtf_features, "Inconsistent TracksType::len");
  static_assert(FPGAEvent::len == emtf::model_config::n_in, "Inconsistent FPGAEvent::len");
  static_assert(FPGAResult::len == emtf::model_config::n_out, "Inconsistent FPGAResult::len");
}

// Main driver
int main(int argc, char **argv) {
  // Perform sanity check
  sanity_check();

  int err = 0;
  std::string clr_info = "\033[1;34m";   // blue
  std::string clr_error = "\033[1;31m";  // red
  std::string clr_reset = "\033[0m";     // no format

  std::initializer_list<int> event_list = {0};
  //std::initializer_list<int> event_list = {
  //    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
  //    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  //    20, 21, 22, 23, 24, 25, 26, 27, 28, 29
  //};

  // Loop over events
  for (auto ievt : event_list) {
    std::cout << clr_info << "Processing event " << ievt << clr_reset << std::endl;

    // Create Event & Result from 'tb_data' text files
    Event evt;
    std::stringstream filename;
    filename << "tb_data/event_" << ievt << ".txt";
    read_tb_data(filename.str(), evt);

    Result res;
    filename.str("");
    filename << "tb_data/result_" << ievt << ".txt";
    read_tb_data(filename.str(), res);

    // Create FPGAEvent & FPGAResult
    const FPGAEvent fw_evt(evt);
    const FPGAResult fw_res(res);

    // Initialize input and output
    top_in_t in0[TOP_N_IN];
    top_out_t out[TOP_N_OUT];
    copy_array(fw_evt.data, in0);
    init_array_as_zeros(out);

    // Call the top function !!
    myproject(in0, out);

    // Compare with the expectation
    int ievt_err = count_mismatches(std::begin(out), std::end(out), std::begin(fw_res.data));
    err += ievt_err;

    // Print error info
    if (ievt_err) {
      std::cout << clr_error << "FAILED!" << clr_reset << std::endl;
      std::cout << "Got:" << std::endl;
      print_array(out);
      std::cout << std::endl;
      std::cout << "Expected:" << std::endl;
      print_array(fw_res.data);
      std::cout << std::endl;
      std::cout << "Mismatches: " << err << std::endl;
    }
  }  // end loop over events

  // Return 0 only if the results are correct
  return err;
}
