/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pcap_file_writer.h"
#include <sys/time.h>

using namespace srsran;

pcap_file_writer::pcap_file_writer() : logger(srslog::fetch_basic_logger("ALL")) {}

pcap_file_writer::~pcap_file_writer()
{
  close();
}

bool pcap_file_writer::open(uint32_t dlt_, const std::string& filename_)
{
  if (pcap_fstream.is_open()) {
    logger.error("Failed to open PCAP file \"{}\". Cause: File is already open", filename);
    return false;
  }

  dlt      = dlt_;
  filename = filename_;

  pcap_hdr_t file_header = {
      0xa1b2c3d4, /// magic number
      2,
      4,     /// version number is 2.4
      0,     /// timezone
      0,     /// sigfigs - apparently all tools do this
      65535, /// snaplen - this should be long enough
      dlt    /// Data Link Type (DLT).  Set as unused value 147 for now
  };

  pcap_fstream.open(filename.c_str(), std::ios::out | std::ios::binary);
  if (pcap_fstream.fail()) {
    logger.error("Failed to open file {} for writing: {}", filename, strerror(errno));
    return false;
  }
  logger.debug("Opened file {} for writing. DLT={}", filename, dlt);

  pcap_fstream.write((char*)&file_header, sizeof(file_header));
  if (pcap_fstream.fail()) {
    logger.error("Failed to write to PCAP: {}", strerror(errno));
    return false;
  }

  return true;
}

void pcap_file_writer::flush()
{
  if (pcap_fstream.is_open()) {
    logger.debug("Flushing PCAP (DLT={})", dlt);
    pcap_fstream.flush();
    return;
  }
  logger.info("Failed to flush closed PCAP (DLT={})", dlt);
}

void pcap_file_writer::close()
{
  flush();
  if (pcap_fstream.is_open()) {
    logger.debug("Saving PCAP (DLT={}) to \"{}\"", dlt, filename);
    pcap_fstream.close();
    logger.info("Saved PCAP (DLT={}) to \"{}\" and closed", dlt, filename);
  }
  logger.info("Failed to close already closed PCAP (DLT={})", dlt);
}

void pcap_file_writer::write_pdu_header(uint32_t length)
{
  pcaprec_hdr_t packet_header = {};

  // PCAP header
  struct timeval t = {};
  gettimeofday(&t, nullptr);
  packet_header.ts_sec   = t.tv_sec;
  packet_header.ts_usec  = t.tv_usec;
  packet_header.incl_len = length;
  packet_header.orig_len = length;

  pcap_fstream.write((char*)&packet_header, sizeof(packet_header));
  if (pcap_fstream.fail()) {
    logger.error("Failed to write to PCAP: {}", strerror(errno));
    return;
  }
}

void pcap_file_writer::write_pdu(srsran::const_span<uint8_t> pdu)
{
  if (not is_write_enabled()) {
    return;
  }

  pcap_fstream.write((char*)pdu.data(), pdu.size());
  if (pcap_fstream.fail()) {
    logger.error("Failed to write to PCAP: {}", strerror(errno));
    return;
  }
}

void pcap_file_writer::write_pdu(const byte_buffer& pdu)
{
  if (not is_write_enabled()) {
    return;
  }

  for (span<const uint8_t> seg : pdu.segments()) {
    pcap_fstream.write((char*)seg.data(), seg.size());
    if (pcap_fstream.fail()) {
      logger.error("Failed to write to PCAP: {}", strerror(errno));
      return;
    }
  }
}
