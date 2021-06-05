import functools
import json
import numpy as np


# Input filename
fname = 'pattern_bank.json'

# Output filename
outname = 'pattern_bank.h'

# C++ code fragments (header, footer, opening, closing)
num_emtf_zones = 3
num_emtf_patterns = 7
num_img_rows = 8
reference = 55
max_pad = 36

header = [
  '#ifndef __EMTF_HLSLIB_PATTERN_BANK_H__',
  '#define __EMTF_HLSLIB_PATTERN_BANK_H__',
  '',
  'namespace emtf {',
  '',
  'namespace phase2 {',
  '',
  'namespace detail {',
  '',
  'constexpr static const int pattern_col_reference = {};'.format(reference),
  'constexpr static const int pattern_col_max_pad = {};'.format(max_pad),
  '',
]

footer = [
  '}  // namespace detail',
  '',
  '}  // namespace phase2',
  '',
  '}  // namespace emtf',
  '',
  '#endif  // __EMTF_HLSLIB_PATTERN_BANK_H__ not defined',
  '',
]

opening = [
  'constexpr static const int pattern_col_start_zone_0_patt_0[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_0_patt_1[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_0_patt_2[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_0_patt_3[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_0_patt_4[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_0_patt_5[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_0_patt_6[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_1_patt_0[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_1_patt_1[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_1_patt_2[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_1_patt_3[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_1_patt_4[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_1_patt_5[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_1_patt_6[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_2_patt_0[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_2_patt_1[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_2_patt_2[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_2_patt_3[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_2_patt_4[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_2_patt_5[{}] = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_start_zone_2_patt_6[{}] = {{'.format(num_img_rows),
  '\nconstexpr static const int pattern_col_mid_zone_0_patt_0[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_0_patt_1[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_0_patt_2[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_0_patt_3[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_0_patt_4[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_0_patt_5[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_0_patt_6[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_1_patt_0[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_1_patt_1[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_1_patt_2[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_1_patt_3[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_1_patt_4[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_1_patt_5[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_1_patt_6[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_2_patt_0[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_2_patt_1[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_2_patt_2[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_2_patt_3[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_2_patt_4[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_2_patt_5[{}]   = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_mid_zone_2_patt_6[{}]   = {{'.format(num_img_rows),
  '\nconstexpr static const int pattern_col_stop_zone_0_patt_0[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_0_patt_1[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_0_patt_2[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_0_patt_3[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_0_patt_4[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_0_patt_5[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_0_patt_6[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_1_patt_0[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_1_patt_1[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_1_patt_2[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_1_patt_3[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_1_patt_4[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_1_patt_5[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_1_patt_6[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_2_patt_0[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_2_patt_1[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_2_patt_2[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_2_patt_3[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_2_patt_4[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_2_patt_5[{}]  = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_stop_zone_2_patt_6[{}]  = {{'.format(num_img_rows),
  '\nconstexpr static const int pattern_col_pad_zone_0[{}]          = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_pad_zone_1[{}]          = {{'.format(num_img_rows),
  'constexpr static const int pattern_col_pad_zone_2[{}]          = {{'.format(num_img_rows),
  '\nconstexpr static const int pattern_activation_zone_0[1u << {}] = {{'.format(num_img_rows),
  '\nconstexpr static const int pattern_activation_zone_1[1u << {}] = {{'.format(num_img_rows),
  '\nconstexpr static const int pattern_activation_zone_2[1u << {}] = {{'.format(num_img_rows),
]

closing = [
    '};' for _ in opening
]


def write_fn(x):
  my_array2string = functools.partial(
      np.array2string, separator=', ', max_line_width=96, threshold=2000)

  # Remove square brackets, attach opening and closing.
  def format_fn(i, txt, multiline=False):
    if multiline:
      txt = txt.replace('[', '\n ').replace(']', '\n')
      txt = '{}{}{}\n'.format(opening[i], txt, closing[i])
    else:
      txt = txt.replace('[', '').replace(']', '')
      txt = '{}{}{}'.format(opening[i], txt, closing[i])
    return txt

  # Join header, body, and footer.
  body = [
      format_fn(i, my_array2string(x_i), multiline=(len(x_i) > 8))
      for i, x_i in enumerate(x)
  ]
  fulltext = header + body + footer
  fulltext = '\n'.join(fulltext)

  # Write to file
  with open(outname, 'w') as f:
    f.write(fulltext)


# Main
if __name__ == '__main__':
  with open(fname) as f:
    obj = json.load(f)
    #print(json.dumps(obj))

    patterns = np.array(obj['patterns'], dtype=np.int32)
    patt_brightness = np.array(obj['patt_brightness'], dtype=np.int32)

    # Some rearrangement.
    patt_start = [patterns[i, j, :, 0] for i in range(num_emtf_zones) for j in range(num_emtf_patterns)]
    patt_mid = [patterns[i, j, :, 1] for i in range(num_emtf_zones) for j in range(num_emtf_patterns)]
    patt_stop = [patterns[i, j, :, 2] for i in range(num_emtf_zones) for j in range(num_emtf_patterns)]
    patt_pad = [np.abs(patterns[i, :, :, 0] - reference).max(axis=0) for i in range(num_emtf_zones)]
    patt_brightness = [patt_brightness[i] for i in range(num_emtf_zones)]
    patt_all = patt_start + patt_mid + patt_stop + patt_pad + patt_brightness
    #print(patt_all)
    assert len(patt_all) == len(opening)
    write_fn(patt_all)
