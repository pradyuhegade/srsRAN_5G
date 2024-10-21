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

#include "srsran/mac/cell_configuration.h"
#include "srsran/mac/mac_lc_config.h"

namespace srsran {

namespace srs_du {
struct du_cell_config;
}

/// Derives MAC Cell Configuration from DU Cell Configuration.
mac_cell_creation_request make_mac_cell_config(du_cell_index_t                                 cell_index,
                                               const srs_du::du_cell_config&                   du_cfg,
                                               std::vector<byte_buffer>                        bcch_dl_sch_payloads,
                                               const sched_cell_configuration_request_message& sched_cell_cfg);

/// \brief Generates default SRB MAC Logical Channel configuration for SRBs other than SRB0.
mac_lc_config make_default_srb_mac_lc_config(lcid_t lcid);

/// \brief Generates default DRB MAC Logical Channel configuration for DRBs.
mac_lc_config make_default_drb_mac_lc_config();

/// Returns the \c prioritized_bit_rate enum value greater than or equal to given bitrate in bit per second.
prioritized_bit_rate get_pbr_ge_to_given_bit_rate(uint64_t bitrate_bps);

/// \brief Generates DRB MAC Logical Channel configuration for GBR DRBs.
mac_lc_config make_gbr_drb_mac_lc_config(const gbr_qos_flow_information& gbr_qos_flow_info);

/// \brief Generates DRB MAC Logical Channel configuration for non-GBR DRBs.
mac_lc_config make_non_gbr_drb_mac_lc_config();

} // namespace srsran
