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

#include "srsran/gateways/udp_network_gateway_factory.h"
#include "udp_network_gateway_impl.h"

using namespace srsran;

std::unique_ptr<udp_network_gateway> srsran::create_udp_network_gateway(udp_network_gateway_creation_message msg)
{
  return std::make_unique<udp_network_gateway_impl>(
      msg.config, msg.data_notifier, msg.io_tx_executor, msg.io_rx_executor);
}
