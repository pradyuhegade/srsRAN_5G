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

#include "srsran/f1u/cu_up/f1u_session_manager.h"

namespace srsran::srs_cu_up {

class f1u_session_manager_impl : public f1u_session_manager
{
public:
  ~f1u_session_manager_impl() override = default;
  explicit f1u_session_manager_impl(const f1u_session_maps& f1u_sessions_);

  gtpu_tnl_pdu_session& get_next_f1u_gateway() override;

private:
  const f1u_session_maps& f1u_sessions;
  uint32_t                next_gw = 0;
};

} // namespace srsran::srs_cu_up
