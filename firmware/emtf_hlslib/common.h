#ifndef __EMTF_HLSLIB_COMMON_H__
#define __EMTF_HLSLIB_COMMON_H__

#if !defined(__SYNTHESIS__) && !defined(emtf_assert)
#include <cassert>
#define emtf_assert(expr) assert(expr)
#elif !defined(emtf_assert)
#define emtf_assert(expr) ((void)0)
#endif  // __SYNTHESIS__ and emtf_assert not defined

#if !defined(__SYNTHESIS__) && !defined(emtf_likely)
#define emtf_likely(x) __builtin_expect(!!(x), 1)
#elif !defined(emtf_likely)
#define emtf_likely(x) (x)
#endif  // __SYNTHESIS__ and emtf_likely not defined

#if !defined(__SYNTHESIS__) && !defined(emtf_unlikely)
#define emtf_unlikely(x) __builtin_expect(!!(x), 0)
#elif !defined(emtf_unlikely)
#define emtf_unlikely(x) (x)
#endif  // __SYNTHESIS__ and emtf_unlikely not defined

#ifndef emtf_maybe_unused
#define emtf_maybe_unused(param) ((void)(param))
#endif  // emtf_maybe_unused not defined

namespace emtf_hlslib {

namespace phase2 {

// from official-cmssw: DataFormats/MuonDetId/interface/CSCDetId.h
constexpr int min_endcap = 1;
constexpr int max_endcap = 2;
constexpr int min_station = 1;
constexpr int max_station = 4;
constexpr int min_ring = 1;
constexpr int max_ring = 4;
constexpr int min_chamber = 1;
constexpr int max_chamber = 36;
constexpr int min_layer = 1;
constexpr int max_layer = 6;

// from official-cmssw: DataFormats/MuonDetId/interface/CSCTriggerNumbering.h
constexpr int min_cscid = 1;
constexpr int max_cscid = 9;
constexpr int min_trigsector = 1;
constexpr int max_trigsector = 6;
constexpr int min_trigsubsector = 0;
constexpr int max_trigsubsector = 2;

// EMTF specific
constexpr int num_emtf_sectors = 12;   // 2 endcaps, 6 sectors per endcap
constexpr int num_emtf_zones = 3;      // per sector
constexpr int num_emtf_timezones = 3;  // per sector

constexpr int num_emtf_chambers = 115;  // per sector
constexpr int num_emtf_segments = 2;    // per chamber
constexpr int num_emtf_variables = 13;  // per segment

constexpr int num_emtf_tracks = 4;       // per sector
constexpr int num_emtf_patterns = 7;     // per zone
constexpr int num_emtf_features = 40;    // per track
constexpr int num_emtf_predictions = 1;  // per track
constexpr int num_emtf_sites = 12;       // per track
constexpr int num_emtf_sites_rm = 5;     // per track

constexpr int num_emtf_img_rows = 8;
constexpr int num_emtf_img_cols = 288;
constexpr int num_emtf_img_gates = 3;

constexpr int emtf_img_col_factor = 16;
constexpr int emtf_img_col_factor_log2 = 4;  // (1 << 4) = 16
constexpr int max_emtf_pattern_activation = 63;
constexpr int max_emtf_pattern_activation_log2 = 6;  // (1 << 6) - 1 = 63

}  // namespace phase2

}  // namespace emtf_hlslib

#endif  // __EMTF_HLSLIB_COMMON_H__ not defined
