# emtf-hlslib

## v3.2.0 (2021-10-19)

- Update pattern bank and nnet weights.
- Replace test bench.
- Patch the 'run_hls.tcl' scripts to:
  - increase maximum array size to 2048.
  - increase clock uncertainty from 20.0% to 23.2%.
  - generate more reports.
  - perform phys_opt_design.

## v3.1.4 (2021-10-07)

- Reduce emtf_time bitwidth from 6-bit to 4-bit.
- Add 'trk_origin_rm' output in duperemoval layer.

## v3.1.3 (2021-08-29)

- Add more trait classes.
- Use round_half_to_even() instead of std::round() in preparing the tanh lookup table.
- Improve debug info in the print macros.
- Add several tests for fixed-point arithmetic in 'hls_math'.

## v3.1.2 (2021-07-26)

- Rename certain segment variables:
  - seg_fr -> seg_cscfr
  - seg_dl -> seg_gemdl
- Change namespace from emtf to emtf_hlslib.
- Fix issue related to bit concatenation.

## v3.1.1 (2021-06-13)

- Apply clang-format.
- Fix namespace.

## v3.1.0 (2021-06-05)

- Rewrite basically all the layers.

## v3.0.5 (2021-05-04)

- Replace test bench and add subproject codes.

## v3.0.4 (2021-04-18)

- Swap the order of duperemoval layer and nnet layer.

## v3.0.3 (2021-03-30)

- Initial release.
- Version number indicates:
  - MAJOR version with **significant** changes in code and results (including FW output, latency and resource usage).
  - MINOR version with changes in code that affect the results.
  - PATCH version with changes in code that do not affect the results. Bug fixes are typically included here.
