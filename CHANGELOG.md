# emtf-hlslib

## v3.1.2 (2021-07-24)

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
