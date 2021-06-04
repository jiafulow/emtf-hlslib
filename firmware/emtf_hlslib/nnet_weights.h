#ifndef __EMTF_HLSLIB_NNET_WEIGHTS_H__
#define __EMTF_HLSLIB_NNET_WEIGHTS_H__

namespace emtf {

namespace details {

constexpr static const int num_nodes_nnet_0_layer_0 = 40;
constexpr static const int num_nodes_nnet_0_layer_1 = 24;
constexpr static const int num_nodes_nnet_0_layer_2 = 24;
constexpr static const int num_nodes_nnet_0_layer_3 = 16;
constexpr static const int num_nodes_nnet_0_layer_4 = 1;

constexpr static const int weights_nnet_0_layer_0[40] = {
    4,   10,   66,   19,   13,   11,   40,   15,   10,    4,   44,    4,  471,  418, 1023, 1023,
 1023,  241,  300,  484,  461,  376,  515,  369,  169,  144,  488,  402,  395,   52,  171,  175,
  174,  172,  171,  162,    1,   24,   16,    0
};

constexpr static const int biases_nnet_0_layer_0[40] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0
};

constexpr static const int weights_nnet_0_layer_1[40 * 24] = {
   18,  121,  -41,   55,   17,   25,    6,    9,   13,   -6,    2,   80,    1,   19,  -10,   -1,
   -2,    3,    1,    1,    0,    1,   -5,    1,    7,   87,   -9,  -11,   -8,   68,   -1,   -4,
  -62,  -31,   -3,   14,    1,   18,  -50,   -6,  -21,    9,    1,   -1,    4,  -32,   -1,  -20,
   11,    8,    2,   17,  -40,  -71,  -12,   15,   39,  -42,    2,   22,   10,   -9,   -2,  -22,
   -5,  -17,   -2,    2,    5,   14,    4,  -22,   -1,   13,   15,   -5,    1,   84,   94,    5,
    3,   28,   -1,    0,   -3,   -4,    9,    4,    4,    3,    2,   -5,   -1,  -14,    2,    2,
   -2,    1,    2,    0,    1,    0,    1,   -2,   -6,   -7,    3,    4,    3, -124,   18,    9,
    3,    8,   12,  -21,    0,  -14,    6,    1,   31,   -9,   -2,    2,    5,  -12,   -1,   -1,
   -4,   -2,   -3,   30,   -6,   -4,   -1,   -1,    2,    1,    2,   -1,   -1,    1,   -1,    0,
   -2,  -37,    0,    0,   -1,    2,  -24,   11,  -12,  -17,  -18,   -4,    0,  -70,   -1,   -4,
   -5,  -13,   -4,  -12,    1,   31,    2,    4,    0,    4,    4,    1,   -8,  -20,   -2,    1,
   -1,  -18,   -7,    4,    0,   -3,    0,    1,   -8,  -39,    3,    9,    1,   -4,  -17,  -38,
    1,  -14,  -22,   75,   -1, -102,  -58,    2,   10,   39,    7,  -71,  101,   23,   -1,  -21,
  -31,  -16,   -1,   17,   -4,  -11,   -2,    6,    3,    1,   -3,    1,   -2,    1,   -2,    1,
    2,   40,    0,   -7,   11,   43,  -15,  -10,  -16,   62,  111,  -18,    0,  -19,  126,    6,
   -4,    6,    0,   -6,   -5,   23,   12,   -5,   15,   12,    0,   14,   -5,  -12,   -1,    3,
    7,    0,    3,    0,    2,    0,    2,   -9,   -2,  -58,   -2,    3,    2,   20,   33,  152,
   10,   20,   25,    7,    1, -133,   18,   -3,    8,   37,   -8,  -28,   11,    6,   16,  -11,
    3,  -88,   15,  -14,    2,    5,   -2,   -3,   -1,    0,   -2,    2,   -1,   -2,   -4,  -14,
    4,   -1,   -5,    0,   -2,  -21,  -10,    1,  -17,  -27,  -31,  -57,   -2,  -18,  -33,    3,
  -40, -131,   -8,    7,    2,  110,    6,  -20,  -44,  -14,   22,   25,    1,    4,    1,    1,
    1,   11,    2,    0,    1,    1,    1,   -1,   -1,   15,    1,   -2,    3,   59,    1,   18,
  -16,   -2,   16,   -8,    1,  -18,  104,   -5,   61,   21,    4,   -7,    4,   63,  -23,    2,
    0,   48,   -1, -151,    7,   31,    0,   -1,    0,    2,    4,   -1,   -1,   -2,    1,   -1,
   -4,   96,    1,    3,    1,  -43,  -11,  292,    0,   -3,   -8, -228,   -2,   44,  -13,   -8,
  -17,   11,    0,   -3,    2,  -21,    7,   -2,   -2,  -30,    5,  -27,   -4,    3,   -2,    0,
    2,  -11,   -2,    0,    1,   -1,   -1,    1,    4,   38,    0,   -2,   -2,   13,  -15,   10,
   -1,  -11,  -17,   -9,    0,  -89,  -14,   -4,   -4,   32,   -4,    2,    1,  -11,   -9,    2,
   -6,    7,   14,    3,   -6,   -6,    5,    0,    1,    1,    2,    0,    0,    2,    0,   -1,
   -2,  -71,    5,    3,    2,  -27,  -47,  -10,   16,    3,   -6,  -29,    0,    1,   15,    1,
   77,   25,    3,  -12,   -2,   -7,   12,    2,   -2,  -11,   17, -185,    3,   11,   -4,   -1,
    0,   -1,   -5,    1,    1,   -3,    2,   -6,   -9,  -16,    0,    6,    4,   -3,   48,   52,
   -5,   -4,    0,  118,   -1,   18,   33,    2,  -49,    9,    2,    1,    2,   28,  -26,   -4,
   -1,  -50,   15, -150,    0,   -6,   -3,    1,   -1,    9,    1,    0,    0,   -1,    0,    0,
   -4,   16,    0,    0,    3,  -76,   -1,   -6,  -20,    6,   15,   -6,    0,  -10,   36,   -1,
   -5,  -26,    1,   28,    0,  -40,   16,   16,  -10,  -43,  -12,  -72,    4,    7,    1,    0,
    3,   -4,    7,    0,    0,   -2,    2,    2,    2,   13,    1,    0,   -1,    7,   21,   42,
   18,   -3,    2,  -23,    0,   17,  -27,    3,  -19,    4,    6,    6,   -9,  -41,    0,    0,
    2,  -85,   -1,  -19,    3,   16,   -2,    1,    0,   -9,    0,   -1,    0,   -5,    1,    0,
    2,  -16,   -1,    0,    0,  -83,    3,   26,   10,    5,    5,   -1,   -1,   25,   -6,   -2,
   -3,  -10,   -4,  -12,    1,  -17,   13,   -7,   -1,  -10,   -1,  -60,   -3,  -15,    1,    1,
   -2,    3,   -3,    2,    1,    2,   -1,   -3,    2,   -5,    0,   -2,   -3,    9,   15,   -4,
  -23,   -4,    5,   21,    1,   30,  -33,    1,   14,   17,   -1,    1,    7,   13,   38,    6,
    4,  -58,    0,   52,    1,   10,    1,    1,    1,   -7,  -12,   -1,   -2,   -2,    0,    0,
   -1,    0,   -1,    2,    0,   84,  -14,   15,   10,   -4,  -10,   -2,   -1,   20,   20,   -1,
   40,   -1,    2,   -2,   -6,   46,    6,  -16,   15, -326,    5,   96,   -4,  -23,    2,   -1,
    0,    5,   -6,    1,    0,   -2,    0,    7,   -5,   66,   -2,   -1,    2,  154,   13,   -9,
   -4,   14,   17,   65,   -1,  -47,    6,    1,  -13,   -6,    5,  -17,   -7,  -21,   -2,   -3,
   -1,  -16,    6,   36,    3,    9,    2,    4,   -1,    9,   17,   -5,   -3,    0,    7,    0,
  -12,  -29,    8,   13,   11,   -9,    7,    3,   23,   14,    3,  -74,    0,  -10,  116,   -4,
   -4,   11,  -10,  -44,   30,   13,   -5,    2,   10,    5,   12,   47,    5,    5,    2,   -3,
    0,    2,    1,   -1,    1,    0,    0,    1,    1,    5,   -3,    5,   -9,   48,    4,  -10,
   12,  -39,  -72,    7,    0,   22,  -77,    6,  -83,  -85,   -3,   16,    3,    7,    3,    3,
    4,   36,  -15,   60,   -3,    4,    1,   -1,    1,    3,   -3,    0,    0,    1,    0,   -5,
    5, -126,    2,   -3,   -3,   -4,    1,  -10,   -3,   -3,   -1,    0,    1,   -6,   -2,    4,
   15,  -11,  -10,    4,   27,   10,   -2,    0,    5,   12,   20,  -16,   -8,   -4,   11,    0,
   -9,    0,    7,   -2,   -1,    1,   14,    4,   30,   23,  -19,  -20,  -26,   36,   15,   20,
   58,   77,   54,  -85,   -1,  -71,   85,   -4,  -25,   -4,   -1,    3,    2,  -12,    6,   -3,
    0,   44,   -7,    6,    2,  -35,    0,    0,    0,    0,   -8,    0,   -1,   -1,   -2,    4,
    3,  -14,   -1,   -2,   -5,   -4,  -17,  342,  -13,  -27,  -30,   22,   -1,    6,    5,    2
};

constexpr static const int biases_nnet_0_layer_1[24] = {
  127,  -23,   10,   69,  124, -236,  -38,   43,  -89,  -21,   40,  -35,  -76,  -25,  -47,   -7,
   61,   16,  -25,  -18,  175,   19,   47,   30
};

constexpr static const int weights_nnet_0_layer_2[24 * 24] = {
   21,   17,   30,   22,  -54,   40,   33,  -56,  -68,  -12,  -14,   26,   33,   53,   70,  -39,
  -32,  108,    7,   12,   75,   38,   65,   26,   -1,    9,  -13,  -58,  -18,    7,  -17,   -9,
    2,   -4,  -87,   51,  -11,   59,   43,   26,   75,   30,    2,   10,   20,  -20,   38,   22,
  -66, -122,   84,   10,  231,   23,  -32,   97, -126, -177,  -80,  -63,  -28,  151,   21,  140,
  116, -108,    6, -214,   27,   28,    6,  -40,   20,  -58,    1,  120,   -2,   48,   58,   34,
  -57,   72,  -79,   89,   36,   64,   45,   -1,   23,   43,  -16,   49,   33,   72,  -64,   47,
    8,   -4,   27,  132,   58,    8,   29,  -84,   16,  105,  -75,  -34,  -20,  -55,  -50,   13,
   -2,   22,  -55,  -36,   -7,   -9,   72,   -7,   10,   10,  -97,  -40,   12,   -8,   -7,  -35,
   18,  -48,  -63,  -24,  -51,  -13,    1,  -52,   25,  103,   -3,   83,   12,  -32,    8,   -8,
  -19,   18,   62, -105,  -30,   -5, -133,   -5,  -16, -135,   75,  -58,   64,  -45,  -85,  -27,
   19,   -3,   -1,  -74,  -19,   33,   -9,  -88,    0,    5,  120,  -67,  -27,   73,  -12,  -30,
   -4,   19,  105,  -27,   28,  -27,   -9,  -26,  -37,  -52,   15,   12,  101,  -33,  -73,   -2,
   11,  -12,   25,   30,   15,    3,   16,    8,   -2,   52,   -3,  -12,  -18,  -18,   17,  -24,
  -38,  -32,   15,   65,   20,  -20,   -2,  -26,    6,  -25,  -12,   -3,   18,   17,    1,   28,
  -31,   -2,  -59,  -52,   -1,   27,  -14,   -7,   31,   29,   -7,   12,  -24,  -58,  -19,  -60,
  -21,    7,  -44,  -15,  -54,  -10,    1,  -31,  -15,  -23,   13,    0,   42,   40,   45,  -24,
   68,   52,  -15,  -13,   -8,  -21,   31,   27,  -12,  -30,   80,    7,  -89,    2,  130,   41,
   13,   13,  116,   27, -133,   51,   14,  109,  106,   63, -143,  -68,  -38, -159,  130,  163,
   11,  -11,  -21,   -8,   36,    5,   41,  -34,   -9,   72,  -46,  -75,  -72, -165,   34, -178,
  -54,   69,   76,    7,  -28,   43,   13,   21,  -12,   35,    6,  171, -110,  -23,  -42,   65,
   -3,    4,  -49,   36,   58,  -43,  -42,   60,  -60,  -45,   25,  -59,  -29,   32,    1, -115,
  -21,  -11,  -33,  -90,  -52,  -45,   57,   54,   38,  -30,   39,   49,  -31,   -7,  -41,    1,
   57,    1,   39,  -40,  -67, -113,  -14,  107,  -14,    7,  -84,   92,  -87,   -9,  -16,  -21,
   30,  -25,  -58,  -84,   23, -107,  -11,  -83,   -3,   -1,   22,    9,   11,  -54,   -4,  -38,
  -37,  -14,  -11,  -57,    9,  -35,   69,   15,   47,  -33,   20,   12,  -48,    9,  -17,  -92,
   65,   21, -205,  -50,  -53,  -98,   94,   59,   -9,   59, -102,   22,    2,  -39,  -13,   -9,
    6,  -34,   10,  -11,  -19,   14,   22,    4,   -7,  -29,   53,   58,  -78,   13,   60,   69,
    0,   53,   31, -204,  -29,  -23,   16, -182,  -45, -266,   32,   17,  -33,   21,  -75, -250,
 -187, -154, -195, -123,  -37,  -80,  -49,   83,   35,   -9,  -56, -103,  -29,    1,    3,    0,
   -4,  -82,   69,   98,   11,   -2,   17,  -36,    9,  -21,    3,   13,    1,  -41,   31,  -33,
   15,    9,   49,  -57,   -9,   19,  -35,  -34,   20,   32,  -23,    1,   78,  -29,  -30,   29,
  -40,  -19,  -25,   -5,   23,   17,  -22,  -22,  -36,    5,  -72,   41,    1,   -7,  -22,   17,
   -6,  -34, -114,  -80,   -4,  -67,  -20,  -27,  -18,   16,  -19,  -28,  -16,    8,   11,  -35,
    0,  -10,  -31,  -10,   32,  -26,    4,   14,   19,   28,  -11,   21,  -23,   27,   38,  -30,
   34,   61,  -16,  -31,  -13,  -34,   63,  -49,   -6,  -14,   -8,    2,   -2,  -20,   12,   13,
   11,   11,   42,  -13,  -21,   24,   20,   11,   -1,    6,  -14,  -15,  -24,   -5,   44,  -11
};

constexpr static const int biases_nnet_0_layer_2[24] = {
 -154, -227,  183,   83, -105,  -72,    2,   54,  -80,  -33,  -20, -111, -130,   80,  108,  -92,
  -16,  -15,  120,  102,  -17,   -1,   15,  -44
};

constexpr static const int weights_nnet_0_layer_3[24 * 16] = {
  -28,   -3,    4,   24,   12,  -70,    6,   -1,   53,    9,   -2,  -67,   33,   16,  -33,   28,
   40,  -37,   19,   -8,   67,   25,  -38,  -57,   76,  145,  -50,  -17, -137,  -48,   10,  -63,
   15,   30,  -65,  -81,   34,    3,  -95,    1,  -36,  -54,   -7,   42,  -12,  -32,  -28, -113,
    4, -124,  -85,  -24,  134,  -29,   38,   25,  230,  -16, -107,  -47,  -85,  -26,   48,  -61,
  -11,   19,   20,  123,   78,  -39,   42, -257,  -21,    5,   39,   54,   24,   52,    6,   28,
 -135,   29,   79,   80,   56,   11,   38,   17,   25,   25,   -2,  -74,  -39,  -19,   14,   81,
   27,  -31,   10,   22,  -17,   20,   -9,   22,  -28,  -17,   77,   -1,    5,  -24,   -3,  -51,
   73,   22,   13,  -28,  -48,  -34,   68,  -13,   34,   61,  -77,   -9,  138,  -66,  -42,   16,
  108,  -23,  -51,  -56,   28,  -64,   71,  -53,  -43,  -39,  -27,   -4,  -20,  -44, -105,  -32,
   29,   65,  -27,    8,   71,  -54,  -66,  -26,  103,  -17,  -33,  -50,   10,  -41,   84,  -52,
 -109,  -48,  -32,  -15,  -43,   -5,  -49,  -75,    6,   76,   15,   32,  -46,   -8,  -43,  -21,
   57,   19,  -56,  -28,  -15,  -27,   64,  -35,  -29,   16,   26,  -73,   -7,    8,   39,  -29,
   17,   47,   32,    4,  -67,  -29,  -45,  -27,   21,    7,  -69,   11,  -26,    1,   34,   29,
    1,   18,  -57,  -33,   24,   39,   -5,  -73,  -19,  -22,    3,  -27,   45,    0,   22,   -6,
   26,   -1,  -35,    3,  -41,  -34,  -59,    5,  -21,    7,   32,   -7,   59,   45,   13,  -28,
   29,    9,  -22,   10,   43,  -72,  -87,   -9,   93,  -43,   -9,  -68,   26,  -53,   63,  -39,
  -38,  -38,  -19,   -3,  -19,  -27,  -64, -137,  -27,    3,  106,   -4,  -37,   31,   30,   34,
 -160,   38,   53,   63,  -42,   31,  -63,    0,   44,   62,   11,  -52,  -63,   34,  150,  105,
   -9,    8,   39,    3,   43,  -18,  -43,    6,   19,   29,    8,   46,   22,   21,   -2,   -4,
   42,    0,    6,  -56,   69,   19,   36,   46,  -29,   51,   79,   64,  -20,   -6,   29,   10,
  -74,   39,   77,   82,   31,   -1,   52,  -13,   55,   98,   28,  -89,  -20,   -4,   63,  140,
  -13, -106,  -27,   34,  122,  -11,  -55,   52,  113, -109,  -69,  -95,  -64,  -28,   57,  -43,
  -28,  -26,  -69,  101,   43,   -2,    7, -170,   87,  -40,  -66,  -74,   35,  -15,   -7,  -24,
   92,  -24,  -92,  -92,  -45,   -8,  -22,  -15,  -23,  -77,  -26,  104,   37,    4,  -98, -135
};

constexpr static const int biases_nnet_0_layer_3[16] = {
    2,  124, -177,  -76,  -83,  156,  111,   53,   81,  -59,   19,  -76,   42,   56, -209,    6
};

constexpr static const int weights_nnet_0_layer_4[16 * 1] = {
  1080,   421,   687, -1502,  -665,  1033,   838,   795,   880,  1137,  1324, -1090,  1058,
 -1570,   694,  1293
};

constexpr static const int biases_nnet_0_layer_4[1] = {
 0
};

}  // namespace details

}  // namespace emtf

#endif  // __EMTF_HLSLIB_NNET_WEIGHTS_H__ not defined
