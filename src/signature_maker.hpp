#pragma once

#include "per_block_file_processor.hpp"

class SignatureMaker : public PerBlockFileProcessor {
public:
  explicit SignatureMaker(
    const char* input_path,
    long int block_size_mb
  ) : PerBlockFileProcessor(input_path, block_size_mb)
  {}

  ~SignatureMaker() {};

private:
  std::vector<unsigned char> ProcessBlock(const unsigned char* const block_ptr, std::size_t size) const override;
};
