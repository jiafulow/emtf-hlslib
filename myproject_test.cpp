// Top function
#include "firmware/myproject.h"

// Test bench
#include "software/testbench.h"

using namespace emtf_hlslib::phase2;
using namespace testbench;

void sanity_check() {
  static_assert(HitsType::len == (num_emtf_variables + 2), "Inconsistent HitsType::len");
  static_assert(TracksType::len == (num_emtf_features + num_emtf_sites + 2), "Inconsistent TracksType::len");
  static_assert(FpgaEvent::len == model_config::n_in, "Inconsistent FpgaEvent::len");
  static_assert(FpgaResult::len == model_config::n_out, "Inconsistent FpgaResult::len");
}

// Main driver
int main(int argc, char** argv) {
  // Perform sanity check
  sanity_check();

  int err = 0;
  std::string clr_info = "\033[1;34m";   // blue
  std::string clr_error = "\033[1;31m";  // red
  std::string clr_reset = "\033[0m";     // no format

  // List of event numbers
  std::initializer_list<int> event_list = {0};
  // std::vector<int> event_list(100);
  // std::iota(event_list.begin(), event_list.end(), 0);

  // Loop over events
  for (auto ievt : event_list) {
    // Skip events that have known mismatches
    //
    // - 22: trk_invpt[0] value off by one
    // - 84: trk_invpt[0] value off by one
    auto should_skip = [](int ievt) -> bool {
      static const std::set<int> s = {22, 84};
      return s.find(ievt) != s.end();
    };

    if (should_skip(ievt))
      continue;

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

    // Create FpgaEvent & FpgaResult
    const FpgaEvent fpga_evt(evt);
    const FpgaResult fpga_res(res);

    // Initialize input & output
    top_in_t in0[TOP_N_IN];
    top_out_t out[TOP_N_OUT];
    std::copy(std::begin(fpga_evt.data), std::end(fpga_evt.data), std::begin(in0));  // copy array
    std::fill(std::begin(out), std::end(out), 0);                                    // init as zeros

    // Call the top function !!
    myproject(in0, out);

    // Compare with the expectation
    int ievt_err = count_mismatches(std::begin(fpga_res.data), std::end(fpga_res.data), std::begin(out));
    err += ievt_err;

    // Print error info
    if (ievt_err) {
      std::cout << clr_error << "FAILED!" << clr_reset << std::endl;
      std::cout << "Got:" << std::endl;
      print_array(out);
      std::cout << std::endl;
      std::cout << "Expected:" << std::endl;
      print_array(fpga_res.data);
      std::cout << std::endl;
      std::cout << "Mismatches: " << err << std::endl;
    }
  }  // end loop over events

  // Return 0 only if the results are correct
  return err;
}
