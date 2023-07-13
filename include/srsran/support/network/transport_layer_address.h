/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "fmt/format.h"
#include <netdb.h>
#include <string>
#include <sys/socket.h>

namespace srsran {

/// \brief Representation of an Transport Layer Address.
class transport_layer_address
{
public:
  transport_layer_address() = default;
  transport_layer_address(const std::string& ip_str) { from_string(ip_str); }

  /// \brief Converts a string with an IPv4 address with format X.X.X.X or with an IPv6 address with
  /// format X:X:X:X:X:X:X:X to a transport_layer_address.
  transport_layer_address& from_string(const std::string& ip_str);

  /// Converts the transport_layer_address to an IPv4 or IPv6 string.
  std::string to_string() const { return fmt::format("{}", *this); }

  /// Converts a string of bits (each character is base 2) to a transport_layer_address.
  transport_layer_address& from_bitstring(std::string bit_str);

  /// Converts the transport layer address to a string of bits (each character is base 2).
  std::string to_bitstring() const;

  /// Extracts the posix representation of the transport layer address.
  const struct sockaddr_storage& native() const { return addr; }
  struct sockaddr_storage&       native() { return addr; }

  /// Compares two transport_layer_addresses.
  bool operator==(const transport_layer_address& other) const;
  bool operator!=(const transport_layer_address& other) const { return not(*this == other); }
  bool operator==(const std::string& ip_str) const { return *this == transport_layer_address(ip_str); }
  bool operator!=(const std::string& ip_str) const { return not(*this == ip_str); }

  bool operator<(const transport_layer_address& other) const;
  bool operator<=(const transport_layer_address& other) const { return *this < other or *this == other; }
  bool operator>=(const transport_layer_address& other) const { return not(*this < other); }
  bool operator>(const transport_layer_address& other) const { return not(*this <= other); }

private:
  struct sockaddr_storage addr;
};

} // namespace srsran

template <>
struct std::hash<srsran::transport_layer_address> {
  std::size_t operator()(const srsran::transport_layer_address& s) const noexcept
  {
    return std::hash<std::string>{}(s.to_string());
  }
};

namespace fmt {

template <>
struct formatter<srsran::transport_layer_address> : public formatter<std::string> {
  template <typename FormatContext>
  auto format(const srsran::transport_layer_address& s, FormatContext& ctx)
      -> decltype(std::declval<FormatContext>().out())
  {
    char ip_addr[NI_MAXHOST];
    getnameinfo((sockaddr*)&s.native(), sizeof(s), ip_addr, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);

    return format_to(ctx.out(), "{}", ip_addr);
  }
};

} // namespace fmt
