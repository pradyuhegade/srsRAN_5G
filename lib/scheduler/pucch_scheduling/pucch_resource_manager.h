/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "../ue_scheduling/ue_configuration.h"
#include "pucch_allocator.h"

namespace srsran {

/// Containers used to return the output of the PUCCH resource manager.
struct pucch_harq_resource_alloc_record {
  /// Pointer to PUCCH resource configuration to be used.
  const pucch_resource* pucch_res;
  /// PUCCH resource indicator corresponding to the resource that will be used by the UE.
  unsigned pucch_res_indicator;
};

/// \brief Class that manages the cell allocation of PUCCH resources across UEs.
/// The correct functioning of pucch_resource_manager is based on the following assumptions:
/// (i)   Each UE has max 8 PUCCH F1 and max 8 PUCCH F2 dedicated to HARQ-ACK reporting.
/// (ii)  The cell PUCCH list has max \ref MAX_SR_PUCCH_RESOURCES PUCCH F1 dedicated to SR reporting; each UE is
/// assigned only 1 of these PUCCH F1 resources for SR.
/// (iii) The cell PUCCH list has max 1 PUCCH F2 dedicated to CSI reporting; each UE use the same CSI resource.
/// (iv)  All UEs use the same cell resources.
/// (v)   Indexing of the PUCCH F1 and PUCCH F2 resources for HARQ-ACK reporting must be contiguous within the F1 group
/// and with F2 group. However, the last PUCCH F1 group resource's and the first PUCCH F2 group resource's indices need
/// not be contiguous. E.g., PUCCH F1 indices (for HARQ-ACK reporting) = {0, ..., 7}, and PUCCH F2 indices (for
/// HARQ-ACK reporting) = {10, ..., 17}.
class pucch_resource_manager
{
public:
  pucch_resource_manager();

  // Reset all resources to "unused".
  void slot_indication(slot_point slot_tx);

  /// Returns true if the common PUCCH resource indexed by r_pucch is available at the given slot.
  bool is_common_resource_available(slot_point sl, size_t r_pucch);

  /// Set the common PUCCH resource indexed by r_pucch at the given slot as currently "not available".
  void reserve_common_resource(slot_point sl, size_t r_pucch);

  /// \brief Returns the PUCCH resource to be used for HARQ-ACK (format 1).
  /// \remark This index refers to the \c pucch-ResourceId of the \c PUCCH-Resource, as per TS 38.331.
  /// \return If any PUCCH resource available, it returns (i) the pointer to the configuration and (ii) the PUCCH
  /// resource indicator corresponding to the PUCCH resource that will be used by the UE. If there are no PUCCH
  /// resources available, the pointer passed will be \c nullptr, whereas the PUCCH resource indicator is to be ignored.
  pucch_harq_resource_alloc_record
  reserve_next_f1_harq_res_available(slot_point slot_harq, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Returns the PUCCH format 2 resource to be used (SR / HARQ-ACK / CSI).
  /// \remark This index refers to the \c pucch-ResourceId of the \c PUCCH-Resource, as per TS 38.331.
  /// \return If any PUCCH resource available, it returns (i) the pointer to the configuration and (ii) the PUCCH
  /// resource indicator corresponding to the PUCCH resource that will be used by the UE. If there are no PUCCH
  /// resources available, the pointer passed will be \c nullptr, whereas the PUCCH resource indicator is to be ignored.
  pucch_harq_resource_alloc_record
  reserve_next_f2_harq_res_available(slot_point slot_harq, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Returns the PUCCH format 2 resource to be used (SR / HARQ-ACK / CSI).
  /// \remark This index refers to the \c pucch-ResourceId of the \c PUCCH-Resource, as per TS 38.331.
  /// \return If any PUCCH resource available, it returns the pointer to the configuration. If there are no PUCCH
  /// resources available, the pointer passed will be \c nullptr..
  const pucch_resource* reserve_specific_format2_res(slot_point          slot_harq,
                                                     rnti_t              crnti,
                                                     unsigned            res_indicator,
                                                     const pucch_config& pucch_cfg);

  /// \brief Returns the PUCCH format 2 resource to be used (SR / CSI).
  /// \remark This index refers to the \c pucch-ResourceId of the \c PUCCH-Resource, as per TS 38.331.
  /// \return If any PUCCH resource available, it returns (i) the pointer to the configuration and (ii) the PUCCH
  /// resource indicator corresponding to the PUCCH resource that will be used by the UE. If there are no PUCCH
  /// resources available, the pointer passed will be \c nullptr, whereas the PUCCH resource indicator is to be ignored.
  const pucch_resource*
  reserve_csi_resource(slot_point slot_harq, rnti_t crnti, const ue_cell_configuration& ue_cell_cfg);

  /// \brief Returns the pointer to the configuration of the PUCCH resource to be used for SR.
  /// \remark There is only one resource used for SR.
  /// \return the pointer to the configuration of the PUCCH resource to be used for SR, if available; else, it returns
  /// \c nullptr.
  const pucch_resource* reserve_sr_res_available(slot_point slot_sr, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Release PUCCH (format 1) resource from being allocated to a given UE.
  /// \param[in] slot_harq slot for which the PUCCH resource was scheduled.
  /// \param[in] crnti UE from which the resource needs to be released.
  /// \param[in] pucch_cfg UE's PUCCH config.
  /// \return True if the resource for the UE was found in the allocation records for the given slot.
  bool release_harq_f1_resource(slot_point slot_harq, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Release PUCCH (format 2) resource from being allocated to a given UE.
  /// \param[in] slot_harq slot for which the PUCCH resource was scheduled.
  /// \param[in] crnti UE from which the resource needs to be released.
  /// \param[in] pucch_cfg UE's PUCCH config.
  /// \return True if the resource for the UE was found in the allocation records for the given slot.
  bool release_harq_f2_resource(slot_point slot_harq, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Release PUCCH (format 1) resource used for SR from being allocated to a given UE.
  /// \param[in] slot_harq slot for which the PUCCH resource was scheduled.
  /// \param[in] crnti UE from which the resource needs to be released.
  /// \param[in] pucch_cfg UE's PUCCH config.
  /// \return True if the resource for the UE was found in the allocation records for the given slot.
  bool release_sr_resource(slot_point slot_sr, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Release PUCCH (format 2) resource used for CSI from being allocated to a given UE.
  /// \param[in] slot_harq slot for which the PUCCH resource was scheduled.
  /// \param[in] crnti UE from which the resource needs to be released.
  /// \param[in] pucch_cfg UE's PUCCH config.
  /// \return True if the resource for the UE was found in the allocation records for the given slot.
  bool release_csi_resource(slot_point slot_sr, rnti_t crnti, const ue_cell_configuration& ue_cell_cfg);

  /// \brief Returns the PUCCH resource indicator (format 1) of the resource used for a given RNTI at a given slot.
  /// \return PUCCH resource indicator of the resource used allocated to the UE; if UE is not found, returns -1.
  int fetch_f1_pucch_res_indic(slot_point slot_tx, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Returns the PUCCH resource indicator (format 2) of the resource used for a given RNTI at a given slot.
  /// \return PUCCH resource indicator of the resource used allocated to the UE; if UE is not found, returns -1.
  int fetch_f2_pucch_res_indic(slot_point slot_tx, rnti_t crnti, const pucch_config& pucch_cfg);

  /// \brief Returns the configuration of the PUCCH resource used for CSI (format 2) for a given RNTI at a given slot.
  /// \return Pointer to the resource configuration used allocated to the UE; if UE is not found, returns \c nullptr.
  const pucch_resource*
  fetch_csi_pucch_res_config(slot_point slot_tx, rnti_t crnti, const ue_cell_configuration& ue_cell_cfg);

private:
  /// Size of the ring buffer of pucch_resource_manager. This size sets a limit on how far in advance a PUCCH can be
  /// allocated.
  static const size_t RES_MANAGER_RING_BUFFER_SIZE =
      get_allocator_ring_size_gt_min(SCHEDULER_MAX_K0 + SCHEDULER_MAX_K1);

  static const unsigned PUCCH_HARQ_F1_RES_SET_ID = 0;
  static const unsigned PUCCH_HARQ_F2_RES_SET_ID = 1;

  // [Implementation-defined] Number of PUCCH resources (of single format) that can be handled by the resource manager.
  // NOTE: this number allows us to have a 1-to-1 match between PUCCH resource indicator and index of the PUCCH resource
  // in its corresponding PUCCH resource set.
  static const size_t MAX_HARQ_PUCCH_RESOURCES{128};
  static const size_t MAX_PUCCH_RESOURCES{128};
  // As per Section 9.2.1, TS 38.213, this is given by the number of possible values of r_PUCCH, which is 16.
  static const size_t MAX_COMMON_PUCCH_RESOURCES{16};
  static const size_t MAX_SR_PUCCH_RESOURCES{4};

  struct resource_tracker {
    rnti_t       rnti;
    pucch_format format;
  };

  using pucch_res_record_array  = std::array<resource_tracker, MAX_PUCCH_RESOURCES>;
  using common_res_record_array = std::array<bool, MAX_COMMON_PUCCH_RESOURCES>;

  // Record for the RNTI and PUCCH resource indicator used for a given resource at a given slot.
  struct rnti_pucch_res_id_slot_record {
    common_res_record_array used_common_resources;
    pucch_res_record_array  ues_using_pucch_res;
  };

  // Returns the resource manager allocation record for a given slot.
  rnti_pucch_res_id_slot_record& get_slot_resource_counter(slot_point sl);

  pucch_harq_resource_alloc_record reserve_next_harq_res_available(slot_point          slot_harq,
                                                                   rnti_t              crnti,
                                                                   const pucch_config& pucch_cfg,
                                                                   pucch_format        format);

  bool release_harq_resource(slot_point slot_harq, rnti_t crnti, const pucch_config& pucch_cfg, pucch_format format);

  int fetch_pucch_res_indic(slot_point slot_tx, rnti_t crnti, const pucch_config& pucch_cfg, pucch_format format);

  // Ring buffer of rnti_pucch_res_id_slot_record for PUCCH resources.
  std::array<rnti_pucch_res_id_slot_record, RES_MANAGER_RING_BUFFER_SIZE> resource_slots;

  // Keeps track of the last slot_point used by the resource manager.
  slot_point last_sl_ind;
};

} // namespace srsran
