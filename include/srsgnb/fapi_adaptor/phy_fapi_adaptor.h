/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#ifndef SRSGNB_FAPI_ADAPTOR_PHY_FAPI_ADAPTOR_H
#define SRSGNB_FAPI_ADAPTOR_PHY_FAPI_ADAPTOR_H

#include "srsgnb/fapi/config_message_notifier.h"
#include "srsgnb/fapi/slot_message_notifier.h"

namespace srsgnb {

class upper_phy_timing_notifier;

namespace fapi_adaptor {

/// \brief Interface to the PHY side of the FAPI adaptor object.
///
/// This interface will give access to the interfaces needed to interconnect the adaptor with the PHY, listen and manage
/// FAPI events.
///
/// \note:This object has the ownership of all the components of the adaptor.
class phy_fapi_adaptor
{
public:
  virtual ~phy_fapi_adaptor() = default;

  /// Returns the adaptor's upper PHY timing notifier.
  virtual upper_phy_timing_notifier& get_upper_phy_timing_notifier() = 0;

  /// \brief Configures the adaptor's FAPI config message notifier to the given one.
  ///
  /// \param[in] fapi_config_notifier FAPI config message notifier to set in the adaptor.
  virtual void set_config_message_notifier(config_message_notifier& fapi_config_notifier) = 0;

  /// \brief Configures the adaptor's FAPI slot message notifier to the given one.
  ///
  /// \param fapi_slot_notifier FAPI slot message notifier to set in the adaptor.
  virtual void set_slot_message_notifier(slot_message_notifier& fapi_slot_notifier) = 0;
};

} // namespace fapi_adaptor
} // namespace srsgnb

#endif // SRSGNB_FAPI_ADAPTOR_PHY_FAPI_ADAPTOR_H
