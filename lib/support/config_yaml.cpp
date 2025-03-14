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

#include "srsran/support/config_parsers.h"
#include "srsran/support/error_handling.h"
#include <yaml-cpp/yaml.h>

using namespace srsran;

namespace {

/// \brief YAML configuration parser implementation.
class yaml_config_parser : public CLI::Config
{
public:
  std::string to_config(const CLI::App* app, bool default_also, bool, std::string) const override;

  std::vector<CLI::ConfigItem> from_config(std::istream& input) const override;

private:
  std::vector<CLI::ConfigItem> from_config_impl(const YAML::Node&               config,
                                                const std::vector<std::string>& prefix = {}) const;
};

} // namespace

std::string
yaml_config_parser::to_config(const CLI::App* app, bool default_also, bool write_description, std::string) const
{
  YAML::Node config;

  for (const CLI::Option* opt : app->get_options()) {
    // Only process the option that has a long-name (starts with a --) and is configurable.
    if (opt->get_lnames().empty() || !opt->get_configurable()) {
      continue;
    }

    const std::string& name = opt->get_lnames().front();

    // Non-flags.
    if (opt->get_type_size() != 0) {
      // If the option was found on command line.
      if (opt->count() == 1) {
        config[name] = opt->results().front();
      } else if (opt->count() > 1) {
        // Recover the items from the string.
        for (const auto& str : opt->results()) {
          YAML::Node node(YAML::Load(str));
          // Write the arrays as YAML flow instead of YAML block.
          config[name].SetStyle((node.size() == 0) ? YAML::EmitterStyle::Flow : YAML::EmitterStyle::Block);
          config[name].push_back(node);
        }
      } else if (default_also && !opt->get_default_str().empty()) {
        // If the option has a default and is requested by optional argument.
        config[name] = YAML::Load(opt->get_default_str());
      }

      continue;
    }

    // Flags.
    if (opt->count() == 1) {
      // Flag, one passed.
      config[name] = true;
      continue;
    }
    if (opt->count() > 1) {
      // Flag, multiples passed.
      config[name] = opt->count();
      continue;
    }
    if (opt->count() == 0 && default_also) {
      // Flag, not present.
      config[name] = false;
      continue;
    }
  }

  for (const CLI::App* subcom : app->get_subcommands({})) {
    if ((!default_also && !subcom->count()) || subcom->get_disabled()) {
      continue;
    }
    config[subcom->get_name()] = YAML::Load(to_config(subcom, default_also, false, ""));
  }

  return YAML::Dump(config);
}

std::vector<CLI::ConfigItem> yaml_config_parser::from_config(std::istream& input) const
{
  YAML::Node config;
  try {
    config = YAML::Load(input);
  } catch (const YAML::ParserException& ex) {
    throw CLI::FileError(std::string("Error parsing YAML configuration file: ") + ex.what());
  }

  return from_config_impl(config);
}

std::vector<CLI::ConfigItem> yaml_config_parser::from_config_impl(const YAML::Node&               config,
                                                                  const std::vector<std::string>& prefix) const
{
  std::vector<CLI::ConfigItem> results;

  // Opening ConfigItem that enables subcommand callbacks.
  // This block inserts an especial ConfigItem in CLI that enables the subcommand to execute callbacks (preparse, parse
  // and finish CLI callbacks). It is done by surrounding the subcommand with an ConfigItem with name '++' before the
  // subcommand and another ConfigItem with name '--' when the subcommand finishes.
  {
    CLI::ConfigItem& res = results.emplace_back();
    res.name             = "++";
    res.parents          = prefix;
    res.inputs           = {};
  }

  for (const auto& node : config) {
    const auto& key   = node.first;
    const auto& value = node.second;

    std::string key_name;
    try {
      key_name = key.as<std::string>();
    } catch (const YAML::Exception& ex) {
      throw CLI::FileError(std::string("Error parsing YAML configuration file: ") + ex.what());
    }

    if (value.IsScalar()) {
      CLI::ConfigItem& res = results.emplace_back();
      res.name             = key_name;
      res.parents          = prefix;
      res.inputs           = {value.Scalar()};
      continue;
    }

    if (value.IsMap()) {
      auto copy_prefix = prefix;
      copy_prefix.push_back(key_name);

      auto sub_results = from_config_impl(value, copy_prefix);
      results.insert(results.end(), sub_results.begin(), sub_results.end());
      continue;
    }

    // Sequences are stored as a vector of strings.
    if (value.IsSequence()) {
      CLI::ConfigItem& res = results.emplace_back();
      res.name             = key_name;
      res.parents          = prefix;
      for (const auto& str : value) {
        res.inputs.push_back(YAML::Dump(str));
      }
      continue;
    }

    // If the item is not one of the previous, is defined but is null, just add an empty section, so CLI parses it.
    if (value.IsDefined() && value.IsNull()) {
      auto copy_prefix = prefix;
      copy_prefix.push_back(key_name);
      {
        CLI::ConfigItem& res = results.emplace_back();
        res.name             = "++";
        res.parents          = copy_prefix;
        res.inputs           = {};
      }
      {
        CLI::ConfigItem& res = results.emplace_back();
        res.name             = "--";
        res.parents          = copy_prefix;
        res.inputs           = {};
      }
    }
  }

  // Closing ConfigItem that enables subcommand callbacks.
  {
    CLI::ConfigItem& res = results.emplace_back();
    res.name             = "--";
    res.parents          = prefix;
    res.inputs           = {};
  }

  return results;
}

std::unique_ptr<CLI::Config> srsran::create_yaml_config_parser()
{
  return std::make_unique<yaml_config_parser>();
}
