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

#include "srsran/phy/upper/channel_processors/pusch/pusch_codeword_buffer.h"
#include "srsran/srsvec/bit.h"
#include "srsran/srsvec/copy.h"

namespace srsran {

class pusch_codeword_buffer_spy : public pusch_codeword_buffer
{
public:
  pusch_codeword_buffer_spy() = default;

  pusch_codeword_buffer_spy(unsigned buffer_size) : data(buffer_size), scrambling_seq(buffer_size) {}

  span<log_likelihood_ratio> get_next_block_view(unsigned block_size) override
  {
    srsran_assert(!completed, "Wrong state.");
    block_size = std::min(block_size, static_cast<unsigned>(data.size()) - count);
    return span<log_likelihood_ratio>(data).subspan(count, block_size);
  }

  void on_new_block(span<const log_likelihood_ratio> new_data, const bit_buffer& new_sequence) override
  {
    srsran_assert(!completed, "Wrong state.");
    srsran_assert(new_sequence.size() == new_data.size(), "Sizes must be equal.");
    unsigned block_size = new_data.size();

    // Append soft bits.
    span<log_likelihood_ratio> dst_data = span<log_likelihood_ratio>(data).subspan(count, block_size);
    srsvec::copy(dst_data, new_data);

    // Append scrambling sequence.
    srsvec::copy_offset(scrambling_seq, count, new_sequence, 0, new_sequence.size());

    count += block_size;
  }

  void on_end_codeword() override
  {
    srsran_assert(!completed, "Wrong state.");
    completed = true;
  }

  span<const log_likelihood_ratio> get_data() const
  {
    srsran_assert(completed, "Wrong state.");
    return span<const log_likelihood_ratio>(data).first(count);
  }

  const bit_buffer& get_scrambling_seq() const
  {
    srsran_assert(completed, "Wrong state.");
    return scrambling_seq;
  }

private:
  bool                              completed = false;
  unsigned                          count     = 0;
  std::vector<log_likelihood_ratio> data;
  dynamic_bit_buffer                scrambling_seq;
};

} // namespace srsran
