#pragma once

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <filesystem>
#include <functional>
#include <map>
#include <mutex>

#include "thread_pool.hpp"

#define BYTES_IN_MB (1024 * 1024)

using boost::interprocess::file_mapping;
using boost::interprocess::read_only;
using boost::interprocess::mapped_region;

typedef std::function<std::vector<unsigned char>(const unsigned char* const block_ptr, std::size_t size)> single_block_processor;

// This class reads a file block by block,
// passes each block along with a processing function
// to a thread pool and writes processed blocks
// to another file when they're ready
//
// As far as an input file can be huge,
// it's being mapped to virtual memory.
// As far as processed block size is
// unknown, it's being written with ostreambuf.
class PerBlockFileProcessor {
public:
  explicit PerBlockFileProcessor(
    const char* input_path,
    long int block_size_mb
  )
    : block_size_bytes_(block_size_mb * BYTES_IN_MB)
  {
    auto input_file_size_bytes_ = std::filesystem::file_size(input_path);

    if (input_file_size_bytes_ == 0)
      throw std::logic_error("Input file has zero size");

    input_file_mapping_= file_mapping(input_path, read_only);
    input_region_ = mapped_region(input_file_mapping_, read_only);
    input_region_.advise(mapped_region::advice_types::advice_sequential);

    if (input_file_size_bytes_ != input_region_.get_size())
      throw std::logic_error("The input file wasn't mapped into memory completely as requested");

    complete_blocks_ = input_region_.get_size() / block_size_bytes_;
    trailing_block_exists_ = input_region_.get_size() % block_size_bytes_;
  }

  // Calls single_block_processor for each block of the input file
  // and writes the result to output_path even if the file exists
  void ProcessFile(const char* output_path, single_block_processor process_block);

  virtual ~PerBlockFileProcessor() {};

private:
  PerBlockFileProcessor(const PerBlockFileProcessor&) =
    delete;  // non construction-copyable
  PerBlockFileProcessor& operator=(const PerBlockFileProcessor&) =
    delete;  // non copyable

  // Processed blocks shared between worker threads and output thread
  ThreadPool thread_pool_;
  std::map<size_t, std::future<std::vector<unsigned char>>> processed_blocks_; // block number -> future with the result
  mutable std::mutex mutex_; // for processed_blocks_
  mutable std::condition_variable new_processed_block_;

  // Input
  file_mapping input_file_mapping_;
  mapped_region input_region_;

  // Blocks description
  long int block_size_bytes_;
  std::size_t complete_blocks_;
  bool trailing_block_exists_;

  void WriteResult(std::ostreambuf_iterator<char>& output_iter);
};
