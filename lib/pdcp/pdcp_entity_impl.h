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

#include "pdcp_entity_rx.h"
#include "pdcp_entity_tx.h"
#include "srsgnb/pdcp/pdcp_entity.h"
#include "srsgnb/ran/bearer_logger.h"
#include <cstdio>

namespace srsgnb {

class pdcp_entity_impl : public pdcp_entity
{
public:
  pdcp_tx_upper_data_interface* get_tx_upper_data_interface() final { return tx.get(); };
  pdcp_tx_lower_interface*      get_tx_lower_interface() final { return tx.get(); };
  pdcp_rx_lower_interface*      get_rx_lower_interface() final { return rx.get(); };

private:
  bearer_logger logger;

  std::unique_ptr<pdcp_entity_tx> tx = {};
  std::unique_ptr<pdcp_entity_rx> rx = {};
};
} // namespace srsgnb
