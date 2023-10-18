/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "../../../lib/fapi_adaptor/mac/mac_to_fapi_translator.h"
#include "messages/helpers.h"
#include "srsran/fapi/messages.h"
#include "srsran/fapi/slot_last_message_notifier.h"
#include "srsran/fapi/slot_message_gateway.h"
#include "srsran/fapi_adaptor/precoding_matrix_table_generator.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace fapi_adaptor;

namespace {

/// Spy implementation of a slot message gateway.
class slot_message_gateway_spy : public fapi::slot_message_gateway
{
  bool                          has_dl_tti_request_method_been_called = false;
  fapi::dl_tti_request_message  dl_tti_msg;
  bool                          has_ul_tti_request_method_been_called = false;
  fapi::ul_tti_request_message  ul_tti_msg;
  bool                          has_tx_data_request_method_been_called = false;
  fapi::tx_data_request_message tx_data_msg;
  bool                          has_ul_dci_request_method_been_called = false;
  fapi::ul_dci_request_message  ul_dci_msg;

public:
  bool has_dl_tti_request_method_called() const { return has_dl_tti_request_method_been_called; }
  bool has_ul_tti_request_method_called() const { return has_ul_tti_request_method_been_called; }
  bool has_tx_data_request_method_called() const { return has_tx_data_request_method_been_called; }
  bool has_ul_dci_request_method_called() const { return has_ul_dci_request_method_been_called; }
  const fapi::dl_tti_request_message&  dl_tti_request_msg() const { return dl_tti_msg; }
  const fapi::ul_tti_request_message&  ul_tti_request_msg() const { return ul_tti_msg; }
  const fapi::tx_data_request_message& tx_data_request_msg() const { return tx_data_msg; }
  const fapi::ul_dci_request_message&  ul_dci_request_msg() const { return ul_dci_msg; }

  void dl_tti_request(const fapi::dl_tti_request_message& msg) override
  {
    has_dl_tti_request_method_been_called = true;
    dl_tti_msg                            = msg;
  }

  void ul_tti_request(const fapi::ul_tti_request_message& msg) override
  {
    has_ul_tti_request_method_been_called = true;
    ul_tti_msg                            = msg;
  }

  void ul_dci_request(const fapi::ul_dci_request_message& msg) override
  {
    has_ul_dci_request_method_been_called = true;
    ul_dci_msg                            = msg;
  }

  void tx_data_request(const fapi::tx_data_request_message& msg) override
  {
    has_tx_data_request_method_been_called = true;
    tx_data_msg                            = msg;
  }
};

class slot_last_message_notifier_spy : public fapi::slot_last_message_notifier
{
  bool       has_been_notified = false;
  slot_point notif_slot;

public:
  void on_last_message(slot_point slot) override
  {
    has_been_notified = true;
    notif_slot        = slot;
  }

  bool       has_on_last_message_method_called() const { return has_been_notified; }
  slot_point slot() const { return notif_slot; }
};

} // namespace

TEST(mac_to_fapi_translator, valid_dl_sched_results_generate_correct_dl_tti_request)
{
  slot_message_gateway_spy       gateway_spy;
  slot_last_message_notifier_spy notifier_spy;
  unsigned                       nof_prbs = 51U;
  auto                           pm_tools = generate_precoding_matrix_tables(1);
  mac_to_fapi_translator         translator(
      srslog::fetch_basic_logger("FAPI"), gateway_spy, notifier_spy, std::move(std::get<0>(pm_tools)), nof_prbs);

  ASSERT_FALSE(gateway_spy.has_dl_tti_request_method_called());

  const mac_dl_sched_result& result = srsran::unittests::build_valid_mac_dl_sched_result();
  translator.on_new_downlink_scheduler_results(result);

  ASSERT_TRUE(gateway_spy.has_dl_tti_request_method_called());
  const fapi::dl_tti_request_message& msg = gateway_spy.dl_tti_request_msg();
  ASSERT_EQ(msg.pdus.size(), 5U);
  ASSERT_EQ((msg.pdus.begin() + 1)->pdu_type, fapi::dl_pdu_type::PDCCH);
  ASSERT_EQ(msg.pdus.back().pdu_type, fapi::dl_pdu_type::PDSCH);
  ASSERT_EQ(msg.pdus.front().pdu_type, fapi::dl_pdu_type::PDCCH);
  ASSERT_EQ(msg.pdus.front().pdcch_pdu.dl_dci.size(), 3U);
  ASSERT_EQ((msg.pdus.begin() + 1)->pdcch_pdu.dl_dci.size(), 1U);
  ASSERT_EQ((msg.pdus.end() - 2)->pdu_type, fapi::dl_pdu_type::SSB);
  ASSERT_EQ((msg.pdus.end() - 3)->pdu_type, fapi::dl_pdu_type::SSB);
}

TEST(mac_to_fapi_translator, valid_ul_sched_results_generate_correct_ul_tti_request)
{
  slot_message_gateway_spy       gateway_spy;
  slot_last_message_notifier_spy notifier_spy;
  unsigned                       nof_prbs = 51U;
  auto                           pm_tools = generate_precoding_matrix_tables(1);
  mac_to_fapi_translator         translator(
      srslog::fetch_basic_logger("FAPI"), gateway_spy, notifier_spy, std::move(std::get<0>(pm_tools)), nof_prbs);

  ASSERT_FALSE(gateway_spy.has_ul_tti_request_method_called());

  const mac_ul_sched_result& result = srsran::unittests::build_valid_mac_ul_sched_result();
  translator.on_new_uplink_scheduler_results(result);

  ASSERT_TRUE(gateway_spy.has_ul_tti_request_method_called());
  const fapi::ul_tti_request_message& msg = gateway_spy.ul_tti_request_msg();
  ASSERT_EQ(msg.pdus.size(), 4U);
  ASSERT_EQ(msg.pdus.front().pdu_type, fapi::ul_pdu_type::PRACH);
  ASSERT_EQ((msg.pdus.begin() + 1)->pdu_type, fapi::ul_pdu_type::PUSCH);
  ASSERT_EQ((msg.pdus.end() - 2)->pdu_type, fapi::ul_pdu_type::PUCCH);
  ASSERT_EQ(msg.pdus.back().pdu_type, fapi::ul_pdu_type::PUCCH);
}

TEST(mac_to_fapi_translator, valid_dl_data_results_generate_correct_tx_data_request)
{
  slot_message_gateway_spy       gateway_spy;
  slot_last_message_notifier_spy notifier_spy;
  unsigned                       nof_prbs = 51U;
  auto                           pm_tools = generate_precoding_matrix_tables(1);
  mac_to_fapi_translator         translator(
      srslog::fetch_basic_logger("FAPI"), gateway_spy, notifier_spy, std::move(std::get<0>(pm_tools)), nof_prbs);

  ASSERT_FALSE(gateway_spy.has_tx_data_request_method_called());

  const mac_dl_sched_result& result = srsran::unittests::build_valid_mac_dl_sched_result();
  translator.on_new_downlink_scheduler_results(result);
  const mac_dl_data_result& data_result = srsran::unittests::build_valid_mac_data_result();
  translator.on_new_downlink_data(data_result);

  ASSERT_TRUE(gateway_spy.has_tx_data_request_method_called());
  const fapi::tx_data_request_message& msg = gateway_spy.tx_data_request_msg();
  ASSERT_EQ(msg.pdus.size(), 1);
}

TEST(mac_to_fapi_translator, valid_dl_data_results_generate_correct_ul_dci_request)
{
  slot_message_gateway_spy       gateway_spy;
  slot_last_message_notifier_spy notifier_spy;
  unsigned                       nof_prbs = 51U;
  auto                           pm_tools = generate_precoding_matrix_tables(1);
  mac_to_fapi_translator         translator(
      srslog::fetch_basic_logger("FAPI"), gateway_spy, notifier_spy, std::move(std::get<0>(pm_tools)), nof_prbs);

  ASSERT_FALSE(gateway_spy.has_ul_dci_request_method_called());

  const mac_dl_sched_result& result = srsran::unittests::build_valid_mac_dl_sched_result();
  translator.on_new_downlink_scheduler_results(result);

  ASSERT_TRUE(gateway_spy.has_ul_dci_request_method_called());
  const fapi::ul_dci_request_message& msg = gateway_spy.ul_dci_request_msg();
  ASSERT_EQ(msg.pdus.size(), 1);
}

TEST(mac_to_fapi_translator, last_message_is_notified)
{
  slot_message_gateway_spy       gateway_spy;
  slot_last_message_notifier_spy notifier_spy;
  unsigned                       nof_prbs = 51U;
  auto                           pm_tools = generate_precoding_matrix_tables(1);
  slot_point                     slot(1, 1, 1);
  mac_to_fapi_translator         translator(
      srslog::fetch_basic_logger("FAPI"), gateway_spy, notifier_spy, std::move(std::get<0>(pm_tools)), nof_prbs);

  ASSERT_FALSE(notifier_spy.has_on_last_message_method_called());
  translator.on_cell_results_completion(slot);
  ASSERT_TRUE(notifier_spy.has_on_last_message_method_called());
  ASSERT_EQ(slot, notifier_spy.slot());
}
