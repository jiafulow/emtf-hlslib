import functools
import json
import numpy as np


# Input filename
fname = 'nnet_weights.json'

# Output filename
outname = 'nnet_weights.h'

# C++ code fragments (header, footer, opening, closing)
nodes0, nodes1, nodes2, nodes_in, nodes_out = (24, 24, 16, 40, 1)

header = [
  '#ifndef __EMTF_HLSLIB_NNET_WEIGHTS_H__',
  '#define __EMTF_HLSLIB_NNET_WEIGHTS_H__',
  '',
  'namespace emtf {',
  '',
  'namespace details {',
  '',
  'constexpr static const int num_nodes_nnet_0_layer_0 = {};'.format(nodes_in),
  'constexpr static const int num_nodes_nnet_0_layer_1 = {};'.format(nodes0),
  'constexpr static const int num_nodes_nnet_0_layer_2 = {};'.format(nodes1),
  'constexpr static const int num_nodes_nnet_0_layer_3 = {};'.format(nodes2),
  'constexpr static const int num_nodes_nnet_0_layer_4 = {};'.format(nodes_out),
  '',
]

footer = [
  '}  // namespace details',
  '',
  '}  // namespace emtf',
  '',
  '#endif  // __EMTF_HLSLIB_NNET_WEIGHTS_H__ not defined',
  '',
]

opening = [
  'constexpr static const int weights_nnet_0_layer_0[{}] = {{'.format(nodes_in),
  'constexpr static const int biases_nnet_0_layer_0[{}] = {{'.format(nodes_in),
  'constexpr static const int weights_nnet_0_layer_1[{} * {}] = {{'.format(nodes_in, nodes0),
  'constexpr static const int biases_nnet_0_layer_1[{}] = {{'.format(nodes0),
  'constexpr static const int weights_nnet_0_layer_2[{} * {}] = {{'.format(nodes0, nodes1),
  'constexpr static const int biases_nnet_0_layer_2[{}] = {{'.format(nodes1),
  'constexpr static const int weights_nnet_0_layer_3[{} * {}] = {{'.format(nodes1, nodes2),
  'constexpr static const int biases_nnet_0_layer_3[{}] = {{'.format(nodes2),
  'constexpr static const int weights_nnet_0_layer_4[{} * {}] = {{'.format(nodes2, nodes_out),
  'constexpr static const int biases_nnet_0_layer_4[{}] = {{'.format(nodes_out),
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
  # x has been flattened.
  body = [
      format_fn(i, my_array2string(x_i.flatten()), multiline=True)
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
    assert len(obj) == len(opening)

    nnet_weights = [np.array(o).astype(np.int32) for o in obj]  # handle floats
    #print([w.flatten() for w in nnet_weights])
    write_fn(nnet_weights)
