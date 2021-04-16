import functools
import json
import numpy as np


# Input filenames
x_test_sparse_fname = 'x_test_sparse.json'
extracted_zonemerging_0_fname = 'extracted_zonemerging_0.json'
extracted_trkbuilding_0_fname = 'extracted_trkbuilding_0.json'
extracted_duperemoval_0_fname = 'extracted_duperemoval_0.json'
extracted_trainfilter_0_fname = 'extracted_trainfilter_0.json'
extracted_fullyconnect_0_fname = 'extracted_fullyconnect_0.json'

# Output filenames
evt_fname = 'event_%i.txt'
res_fname = 'result_%i.txt'
pr_evt_fname = 'pr_event_%i.txt'
pr_res_fname = 'pr_result_%i.txt'
fe_evt_fname = 'fe_event_%i.txt'
fe_res_fname = 'fe_result_%i.txt'
rm_evt_fname = 'rm_event_%i.txt'
rm_res_fname = 'rm_result_%i.txt'
mi_evt_fname = 'mi_event_%i.txt'
mi_res_fname = 'mi_result_%i.txt'

# Masking
mask_value = 999999


def ismasked(x):
  return (x == mask_value)


def replace_mask(x, replacement=0):
  np.copyto(x, replacement, where=ismasked(x))
  return x


def write_testbench(x, y, evt_fname, res_fname):
  assert len(x) == len(y)
  my_array2string = functools.partial(
      np.array2string, separator=', ', max_line_width=100, formatter={'int': (lambda x: '% 4d' % x)})

  # Write to file
  for i, evt in enumerate(x):
    with open(evt_fname % i, 'w') as f:
      f.write(my_array2string(evt))

  for i, res in enumerate(y):
    with open(res_fname % i, 'w') as f:
      f.write(my_array2string(res))


# Main
if __name__ == '__main__':
  with open(x_test_sparse_fname) as f:
    x_test_sparse = json.load(f)
    x_test_sparse = [np.array(x, dtype=np.int32) for x in x_test_sparse]
    print(len(x_test_sparse), x_test_sparse[0].shape)  # N, (None, 15)

  with open(extracted_zonemerging_0_fname) as f:
    extracted_zonemerging_0 = json.load(f)
    extracted_zonemerging_0 = [np.array(x, dtype=np.int32) for x in extracted_zonemerging_0]
    extracted_zonemerging_0 = np.stack(extracted_zonemerging_0, axis=-1)  # add axis and concatenate arrays
    print(extracted_zonemerging_0.shape)  # (N, 4, 4)

  with open(extracted_trkbuilding_0_fname) as f:
    extracted_trkbuilding_0 = json.load(f)
    extracted_trkbuilding_0 = [np.array(x, dtype=np.int32) for x in extracted_trkbuilding_0]
    extracted_trkbuilding_0 = np.dstack(extracted_trkbuilding_0)  # concatenate arrays
    extracted_trkbuilding_0 = replace_mask(extracted_trkbuilding_0)
    print(extracted_trkbuilding_0.shape)  # (N, 4, 52)

  with open(extracted_duperemoval_0_fname) as f:
    extracted_duperemoval_0 = json.load(f)
    extracted_duperemoval_0 = [np.array(x, dtype=np.int32) for x in extracted_duperemoval_0]
    extracted_duperemoval_0 = np.dstack(extracted_duperemoval_0)  # concatenate arrays
    extracted_duperemoval_0 = replace_mask(extracted_duperemoval_0)
    print(extracted_duperemoval_0.shape)  # (N, 4, 52)

  with open(extracted_trainfilter_0_fname) as f:
    extracted_trainfilter_0 = json.load(f)
    extracted_trainfilter_0 = [np.array(x).astype(np.int32) for x in extracted_trainfilter_0]  # handle bools
    assert len(extracted_trainfilter_0) == 1
    extracted_trainfilter_0 = extracted_trainfilter_0[0]  # squeeze
    print(extracted_trainfilter_0.shape)  # (N, 4, 1)

  with open(extracted_fullyconnect_0_fname) as f:
    extracted_fullyconnect_0 = json.load(f)
    extracted_fullyconnect_0 = [np.array(x).astype(np.int32) for x in extracted_fullyconnect_0]  # handle floats
    assert len(extracted_fullyconnect_0) == 1
    extracted_fullyconnect_0 = extracted_fullyconnect_0[0]  # squeeze
    print(extracted_fullyconnect_0.shape)  # (N, 4, 1)

  # Concatenate arrays
  extracted_trainfilter_0_dummy = np.any(
      extracted_duperemoval_0[:, :, :40] != 0, axis=-1, keepdims=True)  # (N, 4, 1)
  extracted_complete = np.dstack(
      (extracted_duperemoval_0, extracted_trainfilter_0_dummy, extracted_fullyconnect_0))
  print(extracted_complete.shape)  # (N, 4, 54)

  # Complete model
  write_testbench(x_test_sparse,
                  extracted_complete[:, :, :],
                  evt_fname=evt_fname,
                  res_fname=res_fname)

  # Pattern recognition
  write_testbench(x_test_sparse,
                  extracted_zonemerging_0[:, :, :],
                  evt_fname=pr_evt_fname,
                  res_fname=pr_res_fname)

  # Feature extraction
  write_testbench(extracted_zonemerging_0[:, :1, :],  # first track only
                  extracted_trkbuilding_0[:, :1, :],
                  evt_fname=fe_evt_fname,
                  res_fname=fe_res_fname)

  # Duplicate removal
  write_testbench(extracted_trkbuilding_0[:, :, :],
                  extracted_duperemoval_0[:, :, :],
                  evt_fname=rm_evt_fname,
                  res_fname=rm_res_fname)

  # Model inference
  write_testbench(extracted_duperemoval_0[:, :1, :40],  # first track only, 40 features
                  extracted_fullyconnect_0[:, :1, :],
                  evt_fname=mi_evt_fname,
                  res_fname=mi_res_fname)

  # DONE
  print('\nThe text files are produced in the current directory. To move them to the right directories, '
        'please refer to the following example commands.\n'
        '\n'
        '  mv event_* result_* ../tb_data/\n'
        '  mv pr_event_* pr_result_* ../subprojects/pr/tb_data/\n'
        '  mv fe_event_* fe_result_* ../subprojects/fe/tb_data/\n'
        '  mv rm_event_* rm_result_* ../subprojects/rm/tb_data/\n'
        '  mv mi_event_* mi_result_* ../subprojects/mi/tb_data/')
