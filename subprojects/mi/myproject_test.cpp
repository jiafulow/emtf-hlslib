// Top function
#include "firmware/myproject.h"

// Test bench
#include "software/testbench.h"

using namespace emtf::phase2;
using namespace testbench;

void sanity_check() {
  MiEvent evt(1);
  assert(evt.front().size() == TOP_N_IN);
  MiResult res(1);
  assert(res.front().size() == TOP_N_OUT);
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
  //std::initializer_list<int> event_list = {0};
  std::vector<int> event_list(100);
  std::iota(event_list.begin(), event_list.end(), 0);

  // Loop over events
  for (auto ievt : event_list) {
    std::cout << clr_info << "Processing event " << ievt << clr_reset << std::endl;

    // Create MiEvent & MiResult from 'tb_data' text files
    MiEvent evt;
    std::stringstream filename;
    filename << "tb_data/mi_event_" << ievt << ".txt";
    read_tb_data(filename.str(), evt);

    MiResult res;
    filename.str("");
    filename << "tb_data/mi_result_" << ievt << ".txt";
    read_tb_data(filename.str(), res);

    // Create evt_flat & res_flat
    assert((evt.size() == 1) and (res.size() == 1));
    auto evt_flat = evt.front();
    auto res_flat = res.front();

    // Initialize input & output
    top_in_t in0[TOP_N_IN];
    top_out_t out[TOP_N_OUT];
    std::copy(std::begin(evt_flat), std::end(evt_flat), std::begin(in0));
    std::fill(std::begin(out), std::end(out), 0);  // init as zeros

    // Call the top function !!
    myproject(in0, out);

    // Compare with the expectation
    int ievt_err = count_mismatches(std::begin(res_flat), std::end(res_flat), std::begin(out), 1);
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
