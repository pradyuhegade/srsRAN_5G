/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/ran/csi_report/csi_report_configuration.h"
#include "srsran/ran/csi_report/csi_report_data.h"
#include "srsran/ran/csi_report/csi_report_packed.h"
#include "srsran/support/units.h"

namespace srsran {

/// Collects the RI, LI, wideband CQI, and CSI fields bit-width.
struct ri_li_cqi_cri_sizes {
  unsigned ri;
  unsigned li;
  unsigned wideband_cqi_first_tb;
  unsigned wideband_cqi_second_tb;
  unsigned subband_diff_cqi_first_tb;
  unsigned subband_diff_cqi_second_tb;
  unsigned cri;
};

/// Gets the RI, LI, wideband CQI, and CRI fields bit-width.
ri_li_cqi_cri_sizes get_ri_li_cqi_cri_sizes(pmi_codebook_type        pmi_codebook,
                                            ri_restriction_type      ri_restriction,
                                            csi_report_data::ri_type ri,
                                            unsigned                 nof_csi_rs_resources);

/// Gets the PMI field bit-width.
unsigned csi_report_get_size_pmi(pmi_codebook_type codebook, csi_report_data::ri_type ri);

/// Unpacks wideband CQI.
csi_report_data::wideband_cqi_type csi_report_unpack_wideband_cqi(csi_report_packed packed);

/// Unpacks PMI.
csi_report_pmi
csi_report_unpack_pmi(const csi_report_packed& packed, pmi_codebook_type codebook, csi_report_data::ri_type ri);

/// Unpacks RI as per TS38.212 Section 6.3.1.1.2. and TS38.214 Section 5.2.2.2.1.
csi_report_data::ri_type csi_report_unpack_ri(const csi_report_packed&   ri_packed,
                                              const ri_restriction_type& ri_restriction);

} // namespace srsran
