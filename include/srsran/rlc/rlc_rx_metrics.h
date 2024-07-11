/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/rlc/rlc_config.h"
#include "srsran/support/engineering_notation.h"
#include "srsran/support/format_utils.h"
#include "fmt/format.h"

namespace srsran {

struct rlc_tm_rx_metrics {};

struct rlc_um_rx_metrics {
  uint32_t num_sdu_segments;      ///< Number of SDU segments RX'ed
  uint32_t num_sdu_segment_bytes; ///< Number of SDU segments Bytes
};

struct rlc_am_rx_metrics {
  uint32_t num_sdu_segments;      ///< Number of SDU segments RX'ed
  uint32_t num_sdu_segment_bytes; ///< Number of SDU segments bytes
  uint32_t num_ctrl_pdus;         ///< Number of control PDUs
  uint32_t num_ctrl_pdu_bytes;    ///< Number of control PDUs bytes
};

struct rlc_rx_metrics {
  // SDU metrics
  uint32_t num_sdus;      ///< Number of SDUs
  size_t   num_sdu_bytes; ///< Number of SDU bytes

  // PDU metrics
  uint32_t num_pdus;           ///< Number of PDUs
  size_t   num_pdu_bytes;      ///< Number of PDU bytes
  uint32_t num_lost_pdus;      ///< Number of dropped PDUs (reassembly timeout expiry or out of rx window)
  uint32_t num_malformed_pdus; ///< Number of malformed PDUs
  uint32_t sdu_latency_us;     ///< total SDU latency (in us)>

  /// RLC mode of the entity
  rlc_mode mode;

  /// Mode-specific metrics
  ///
  /// The associated union member is indicated by \c mode.
  /// Contents of the other fields are undefined.
  union {
    rlc_tm_rx_metrics tm;
    rlc_um_rx_metrics um;
    rlc_am_rx_metrics am;
  } mode_specific;
};

class rlc_rx_metrics_interface
{
public:
  rlc_rx_metrics_interface()                                            = default;
  virtual ~rlc_rx_metrics_interface()                                   = default;
  rlc_rx_metrics_interface(const rlc_rx_metrics_interface&)             = delete;
  rlc_rx_metrics_interface& operator=(const rlc_rx_metrics_interface&)  = delete;
  rlc_rx_metrics_interface(const rlc_rx_metrics_interface&&)            = delete;
  rlc_rx_metrics_interface& operator=(const rlc_rx_metrics_interface&&) = delete;

  virtual rlc_rx_metrics get_metrics()           = 0;
  virtual rlc_rx_metrics get_and_reset_metrics() = 0;
  virtual void           reset_metrics()         = 0;
};

inline std::string format_rlc_rx_metrics(timer_duration metrics_period, const rlc_rx_metrics& m)
{
  fmt::memory_buffer buffer;
  fmt::format_to(buffer,
                 "num_sdus={} sdu_rate={}bps num_pdus={} pdu_rate={}bps",
                 scaled_fmt_integer(m.num_sdus, false),
                 float_to_eng_string(static_cast<float>(m.num_sdu_bytes) * 8 * 1000 / metrics_period.count(), 1, false),
                 scaled_fmt_integer(m.num_pdus, false),
                 (double)m.num_pdu_bytes * 8 / (double)metrics_period.count());

  // No TM specific metrics for RX
  if ((m.mode == rlc_mode::um_bidir || m.mode == rlc_mode::um_unidir_ul)) {
    // Format UM specific metrics for RX
    fmt::format_to(buffer,
                   " num_sdu_segments={} sdu_segmments_rate={}bps",
                   scaled_fmt_integer(m.mode_specific.um.num_sdu_segments, false),
                   float_to_eng_string(static_cast<float>(m.mode_specific.um.num_sdu_segment_bytes) * 8 * 1000 /
                                           metrics_period.count(),
                                       1,
                                       false));
  } else if (m.mode == rlc_mode::am) {
    fmt::format_to(
        buffer,
        " num_sdu_segments={} sdu_segmments_rate={}bps",
        " ctrl_pdus={} ctrl_rate={}bps",
        scaled_fmt_integer(m.mode_specific.am.num_sdu_segments, false),
        float_to_eng_string(
            static_cast<float>(m.mode_specific.am.num_sdu_segment_bytes) * 8 * 1000 / metrics_period.count(), 1, false),
        scaled_fmt_integer(m.mode_specific.am.num_ctrl_pdus, false),
        float_to_eng_string(
            static_cast<float>(m.mode_specific.am.num_ctrl_pdu_bytes) * 8 * 1000 / metrics_period.count(), 1, false));
  }
  return to_c_str(buffer);
}

} // namespace srsran

namespace fmt {

// RLC RX metrics formatter
// TODO print mode-specific metrics
template <>
struct formatter<srsran::rlc_rx_metrics> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::rlc_rx_metrics m, FormatContext& ctx) -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(),
                     "num_pdus={} num_pdu_bytes={} num_lost_pdus={} num_malformed_pdus={} num_sdus={} num_sdu_bytes={}",
                     m.num_pdus,
                     m.num_pdu_bytes,
                     m.num_lost_pdus,
                     m.num_malformed_pdus,
                     m.num_sdus,
                     m.num_sdu_bytes);
  }
};
} // namespace fmt
