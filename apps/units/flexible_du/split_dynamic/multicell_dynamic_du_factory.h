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

#include "apps/units/flexible_du/du_unit.h"

namespace srsran {
struct dynamic_du_unit_config;

du_unit create_multicell_dynamic_du(const dynamic_du_unit_config& dyn_du_cfg, const du_unit_dependencies& dependencies);

} // namespace srsran
