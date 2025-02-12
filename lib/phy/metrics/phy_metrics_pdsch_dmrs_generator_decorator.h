/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/phy/metrics/phy_metrics_notifiers.h"
#include "srsran/phy/metrics/phy_metrics_reports.h"
#include "srsran/phy/upper/signal_processors/dmrs_pdsch_processor.h"
#include <memory>

namespace srsran {

/// DM-RS for PDSCH generator metric decorator.
class phy_metrics_dmrs_pdsch_processor_decorator : public dmrs_pdsch_processor
{
public:
  /// Creates a DM-RS for PDSCH generator from a base instance and a metric notifier.
  phy_metrics_dmrs_pdsch_processor_decorator(std::unique_ptr<dmrs_pdsch_processor> base_,
                                             pdsch_dmrs_generator_metric_notifier& notifier_) :
    base(std::move(base_)), notifier(notifier_)
  {
    srsran_assert(base, "Invalid encoder.");
  }

  // See interface for documentation.
  void map(resource_grid_writer& grid, const config_t& config) override
  {
    auto tp_before = std::chrono::high_resolution_clock::now();
    base->map(grid, config);
    auto tp_after = std::chrono::high_resolution_clock::now();

    notifier.on_new_metric({.elapsed = tp_after - tp_before});
  }

private:
  std::unique_ptr<dmrs_pdsch_processor> base;
  pdsch_dmrs_generator_metric_notifier& notifier;
};

} // namespace srsran
