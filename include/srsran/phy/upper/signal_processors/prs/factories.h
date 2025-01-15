/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/phy/generic_functions/precoding/precoding_factories.h"
#include "srsran/phy/upper/sequence_generators/sequence_generator_factories.h"
#include "srsran/phy/upper/signal_processors/prs/prs_generator.h"
#include "srsran/phy/upper/signal_processors/prs/prs_generator_validator.h"
#include <memory>

namespace srsran {

/// PRS generator factory.
class prs_generator_factory
{
public:
  /// Default destructor.
  virtual ~prs_generator_factory() = default;

  /// Creates a PRS generator.
  virtual std::unique_ptr<prs_generator> create() = 0;

  /// Creates a PRS generator configuration validator.
  virtual std::unique_ptr<prs_generator_validator> create_validator() = 0;
};

/// \brief Creates a generic PRS generator factory.
/// \param[in] prg_factory      Pseudo-random sequence generator factory.
/// \param[in] precoder_factory Channel precoder factory.
/// \return A valid PRS generator factory if successful.
std::shared_ptr<prs_generator_factory>
create_prs_generator_generic_factory(std::shared_ptr<pseudo_random_generator_factory> prg_factory,
                                     std::shared_ptr<channel_precoder_factory>        precoder_factory);

} // namespace srsran
