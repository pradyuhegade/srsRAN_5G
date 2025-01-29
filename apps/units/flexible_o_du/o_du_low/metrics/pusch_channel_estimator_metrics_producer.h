/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#pragma once

#include "srsran/phy/metrics/phy_metrics_notifiers.h"
#include "srsran/phy/metrics/phy_metrics_reports.h"
#include "srsran/ran/subcarrier_spacing.h"
#include "srsran/support/math/math_utils.h"
#include <atomic>

namespace srsran {

/// PUSCH processor metric producer.
class pusch_channel_estimator_metric_producer_impl : private pusch_channel_estimator_metric_notifier
{
public:
  /// Gets the average channel estimation time in microseconds.
  double get_average_latency() const { return static_cast<double>(sum_elapsed_ns) / static_cast<double>(count) * 1e-3; }

  /// Gets the average PRB processing rate in millions of PRB-per-second.
  double get_processing_rate() const
  {
    return static_cast<double>(sum_nof_prb) / static_cast<double>(sum_elapsed_ns) * 1e3;
  }

  /// Gets the total execution time of the channel estimation.
  std::chrono::nanoseconds get_total_time() const { return std::chrono::nanoseconds(sum_elapsed_ns); }

  /// Gets the PUSCH channel estimator metrics.
  pusch_channel_estimator_metric_notifier& get_notifier() { return *this; }

private:
  // See interface for documentation.
  void new_metric(const pusch_channel_estimator_metrics& metrics) override
  {
    sum_nof_prb += metrics.nof_prb;
    sum_elapsed_ns += metrics.elapsed.count();
    ++count;
  }

  std::atomic<uint64_t> count          = {};
  std::atomic<uint64_t> sum_nof_prb    = {};
  std::atomic<uint64_t> sum_elapsed_ns = {};
};

} // namespace srsran
