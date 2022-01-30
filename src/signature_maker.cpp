#include <openssl/md5.h>

#include "signature_maker.hpp"

std::vector<unsigned char> SignatureMaker::ProcessBlock(const unsigned char* const block_ptr, std::size_t size) const {
  if (block_ptr == nullptr) {
    throw std::logic_error("start_ptr is nullptr");
  }

  std::vector<unsigned char> result(MD5_DIGEST_LENGTH);
  MD5(block_ptr, size, result.data());
  return result;
}
