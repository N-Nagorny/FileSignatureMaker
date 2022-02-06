#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <tuple>

#include <openssl/md5.h>

#include "block_storage.hpp"
#include "per_block_file_processor.hpp"
#include "profile.hpp"
#include "thread_pool.hpp"

#define BYTES_IN_MB (1024 * 1024)
#define DEFAULT_BLOCK_SIZE_MB 1

void print_usage(const char* exec_name) {
  std::cout << "SYNOPSIS\n";
  std::cout << '\t' << exec_name << " source destination [blocksize]\n";
  std::cout << "OPTIONS\n";
  std::cout << '\t' << "source\n";
  std::cout << "\t\t" << "Path to an input file to make a signature for.\n";
  std::cout << '\t' << "destination\n";
  std::cout << "\t\t" << "Path to an output file to write the signature to.\n";
  std::cout << '\t' << "blocksize\n";
  std::cout << "\t\t" << "Positive integer that specifies block size"
    " in megabytes which hash is calculated for. Defaults to 1." << std::endl;
}

std::tuple<const char*, const char*, long int> get_cmd_arguments(int argc, char** argv) {
  long int block_size_mb = DEFAULT_BLOCK_SIZE_MB;

  if (argc < 3 || argc > 4) {
    throw std::invalid_argument("Invalid argument number");
  } else if (argc == 4) {
    // Read block size
    errno = 0;
    char* end_ptr;
    block_size_mb = std::strtol(argv[3], &end_ptr, 10);
    if (argv[3] == end_ptr || errno == ERANGE || *end_ptr != 0 || block_size_mb <= 0) {
      throw std::invalid_argument("Invalid block size value");
    }
  }

  return std::make_tuple(argv[1], argv[2], block_size_mb);
}

// Example single_block_processor to perform MD5 hashing
single_block_processor make_single_block_processor_md5() {
  return [](const unsigned char* const block_ptr, std::size_t size) -> std::vector<unsigned char> {
    if (block_ptr == nullptr) {
      throw std::logic_error("block_ptr is nullptr");
    }

    std::vector<unsigned char> result(MD5_DIGEST_LENGTH);
    MD5(block_ptr, size, result.data());
    return result;
  };
}

void make_signature(const std::string& input_path, const std::string& output_path, std::size_t block_size_mb, single_block_processor block_processor) {
  std::size_t block_size_bytes = block_size_mb * BYTES_IN_MB;
  std::size_t num_blocks = std::thread::hardware_concurrency();
  std::unique_ptr<IBlockStorage> block_storage_ptr = std::make_unique<BlockStorage>(num_blocks, block_size_bytes);
  PerBlockFileProcessor signature_maker(input_path, block_size_bytes, block_storage_ptr.get());
  signature_maker.ProcessFile<ThreadPool>(output_path, block_processor);
}

int main(int argc, char** argv) {
  try {
    auto [input_path, output_path, block_size_mb] = get_cmd_arguments(argc, argv);
    {
      LOG_DURATION("making a signature");
      make_signature(input_path, output_path, block_size_mb, make_single_block_processor_md5());
    }
  } catch (const std::invalid_argument& e) {
    std::cout << e.what() << std::endl << std::endl;
    print_usage(argv[0]);
    return -2;
  } catch (const std::exception& e) {
    std::cout << "An exception occured while executing: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
