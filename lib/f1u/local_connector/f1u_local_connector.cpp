/*
 *
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/f1u/local_connector/f1u_local_connector.h"
#include "srsran/f1u/cu_up/f1u_bearer_factory.h"
#include "srsran/f1u/du/f1u_bearer_factory.h"
#include "srsran/ran/lcid.h"

using namespace srsran;

std::unique_ptr<srs_cu_up::f1u_bearer>
f1u_local_connector::create_cu_bearer(uint32_t                             ue_index,
                                      uint32_t                             ul_teid,
                                      srs_cu_up::f1u_rx_delivery_notifier& rx_delivery_notifier,
                                      srs_cu_up::f1u_rx_sdu_notifier&      rx_sdu_notifier,
                                      timer_factory                        timers)
{
  logger_cu.info("Creating CU F1-U bearer. UL-TEID={}", ul_teid);
  std::unique_lock<std::mutex> lock(map_mutex);
  srsran_assert(
      cu_map.find(ul_teid) == cu_map.end(), "Cannot create CU F1-U bearer with already existing UL-TEID={}", ul_teid);
  std::unique_ptr<f1u_dl_local_adapter>  cu_tx      = std::make_unique<f1u_dl_local_adapter>();
  std::unique_ptr<srs_cu_up::f1u_bearer> f1u_bearer = srs_cu_up::create_f1u_bearer(
      ue_index, drb_id_t{}, *cu_tx, rx_delivery_notifier, rx_sdu_notifier, timers, *this, ul_teid);
  f1u_cu_bearer cu_bearer(std::move(cu_tx), f1u_bearer.get());
  cu_map.insert({ul_teid, std::move(cu_bearer)});
  return f1u_bearer;
}

void f1u_local_connector::attach_dl_teid(uint32_t ul_teid, uint32_t dl_teid)
{
  std::unique_lock<std::mutex> lock(map_mutex);
  if (cu_map.find(ul_teid) == cu_map.end()) {
    logger_cu.warning("Could not find UL-TEID at CU to connect. UL-TEID={}, DL-TEID={}", ul_teid, dl_teid);
    return;
  }
  logger_cu.debug("Connecting CU F1-U bearer. UL-TEID={}, DL-TEID={}", ul_teid, dl_teid);

  if (du_map.find(dl_teid) == du_map.end()) {
    logger_cu.warning("Could not find DL-TEID at DU to connect. UL-TEID={}, DL-TEID={}", ul_teid, dl_teid);
    return;
  }
  logger_cu.debug("Connecting DU F1-U bearer. UL-TEID={}, DL-TEID={}", ul_teid, dl_teid);

  auto& du_tun = du_map.at(dl_teid);
  auto& cu_tun = cu_map.at(ul_teid);
  du_tun.du_tx->attach_cu_handler(cu_tun.f1u_bearer->get_rx_pdu_handler());
  cu_tun.dl_teid = dl_teid;
}

void f1u_local_connector::disconnect_cu_bearer(uint32_t ul_teid)
{
  std::unique_lock<std::mutex> lock(map_mutex);
  // Find bearer from ul_teid
  auto bearer_it = cu_map.find(ul_teid);
  if (bearer_it == cu_map.end()) {
    logger_cu.warning("Could not find UL-TEID={} at CU to remove.", ul_teid);
    return;
  }

  // Disconnect UL path of DU first if we have a dl_teid for lookup
  if (bearer_it->second.dl_teid.has_value()) {
    auto du_bearer_it = du_map.find(bearer_it->second.dl_teid.value());
    if (du_bearer_it != du_map.end()) {
      logger_cu.debug("Disconnecting DU F1-U bearer with DL-TEID={} from CU handler. UL-TEID={}",
                      bearer_it->second.dl_teid,
                      ul_teid);
      du_bearer_it->second.du_tx->detach_cu_handler();
    } else {
      // Bearer could already been removed from DU.
      logger_cu.info("Could not find DL-TEID={} at DU to disconnect DU F1-U bearer from CU handler. UL-TEID={}",
                     bearer_it->second.dl_teid,
                     ul_teid);
    }
  } else {
    logger_cu.warning("No DL-TEID provided to disconnect DU F1-U bearer from CU handler. UL-TEID={}", ul_teid);
  }

  // Remove DL path
  logger_cu.debug("Removing CU F1-U bearer with UL-TEID={}.", ul_teid);
  cu_map.erase(bearer_it);
}

srs_du::f1u_bearer* f1u_local_connector::create_du_bearer(uint32_t                     ue_index,
                                                          drb_id_t                     drb_id,
                                                          srs_du::f1u_config           config,
                                                          uint32_t                     dl_teid,
                                                          uint32_t                     ul_teid,
                                                          srs_du::f1u_rx_sdu_notifier& du_rx,
                                                          timer_factory                timers)
{
  std::unique_lock<std::mutex> lock(map_mutex);
  if (cu_map.find(ul_teid) == cu_map.end()) {
    logger_du.warning(
        "Could not find CU F1-U bearer, when creating DU F1-U bearer. DL-TEID={}, UL-TEID={}", dl_teid, ul_teid);
    return nullptr;
  }

  logger_du.debug("Creating DU F1-U bearer. DL-TEID={}, UL-TEID={}", dl_teid, ul_teid);
  std::unique_ptr<f1u_ul_local_adapter> du_tx = std::make_unique<f1u_ul_local_adapter>();

  srs_du::f1u_bearer_creation_message f1u_msg = {};
  f1u_msg.ue_index                            = ue_index;
  f1u_msg.drb_id                              = drb_id;
  f1u_msg.config                              = config;
  f1u_msg.rx_sdu_notifier                     = &du_rx;
  f1u_msg.tx_pdu_notifier                     = du_tx.get();
  f1u_msg.timers                              = timers;

  std::unique_ptr<srs_du::f1u_bearer> f1u_bearer = srs_du::create_f1u_bearer(f1u_msg);
  srs_du::f1u_bearer*                 ptr        = f1u_bearer.get();
  auto&                               cu_tun     = cu_map.at(ul_teid);
  cu_tun.cu_tx->attach_du_handler(f1u_bearer->get_rx_pdu_handler());

  du_tx->attach_cu_handler(cu_tun.f1u_bearer->get_rx_pdu_handler());

  f1u_du_bearer du_bearer(std::move(du_tx), std::move(f1u_bearer));
  du_map.insert({dl_teid, std::move(du_bearer)});
  return ptr;
}

void f1u_local_connector::remove_du_bearer(uint32_t dl_teid)
{
  std::unique_lock<std::mutex> lock(map_mutex);
  auto                         bearer_it = du_map.find(dl_teid);
  if (bearer_it == du_map.end()) {
    logger_du.warning("Could not find DL-TEID at DU to remove. DL-TEID={}", dl_teid);
    return;
  }
  logger_du.debug("Removing DU F1-U bearer. DL-TEID={}", dl_teid);
  du_map.erase(bearer_it);
}
