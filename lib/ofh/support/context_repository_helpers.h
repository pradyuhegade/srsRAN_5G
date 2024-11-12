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

#include "srsran/ran/slot_point.h"

namespace srsran {
namespace ofh {

/// System frame number maximum value in the Open Fronthaul repositories.
static constexpr unsigned SFN_MAX_VALUE = 1U << 8;

/// \brief Calculates the size of the repository using the given parameters.
///
/// This function will find the minimum divisible value of the maximum number of slots that Open Fronthaul supports.
inline unsigned calculate_repository_size(subcarrier_spacing scs, unsigned minimum_repo_size)
{
  for (unsigned nof_slots = SFN_MAX_VALUE * get_nof_slots_per_subframe(scs) * NOF_SUBFRAMES_PER_FRAME;
       nof_slots % minimum_repo_size != 0;
       ++minimum_repo_size) {
  }

  return minimum_repo_size;
}

/// Calculates the repository local index for the given slot and repository size.
inline unsigned calculate_repository_index(slot_point slot, unsigned repo_size)
{
  slot_point entry_slot(slot.numerology(), slot.sfn() % SFN_MAX_VALUE, slot.slot_index());
  return entry_slot.system_slot() % repo_size;
}

} // namespace ofh
} // namespace srsran
