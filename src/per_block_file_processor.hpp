#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <mutex>

#include "block_storage.hpp"
#include "thread_pool.hpp"

#define BYTES_IN_MB (1024 * 1024)

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
    const std::string& input_path,
    long int block_size_mb
  )
    : input_path_(input_path)
    , block_size_bytes_(block_size_mb * BYTES_IN_MB)
    , thread_pool_(std::thread::hardware_concurrency(), std::thread::hardware_concurrency())
    , block_storage_(std::thread::hardware_concurrency(), block_size_mb * BYTES_IN_MB)
  {
    input_file_size_bytes_ = std::filesystem::file_size(input_path);

    if (input_file_size_bytes_ == 0)
      throw std::logic_error("Input file has zero size");

    complete_blocks_ = input_file_size_bytes_ / block_size_bytes_;
    trailing_block_exists_ = input_file_size_bytes_ % block_size_bytes_;
  }

  // Calls single_block_processor for each block of the input file
  // and writes the result to output_path even if the file exists
  void ProcessFile(const std::string& output_path, single_block_processor process_block);

  virtual ~PerBlockFileProcessor() {};

private:
  PerBlockFileProcessor(const PerBlockFileProcessor&) =
    delete;  // non construction-copyable
  PerBlockFileProcessor& operator=(const PerBlockFileProcessor&) =
    delete;  // non copyable

  ThreadPool thread_pool_;
  BlockStorage block_storage_;
  mutable std::mutex block_storage_mutex_;
  // Processed blocks are shared between worker threads and output thread
  std::map<size_t, std::future<std::vector<unsigned char>>> processed_blocks_; // block number -> future with the result
  mutable std::mutex processed_blocks_mutex_;
  mutable std::condition_variable new_processed_block_;
  mutable std::condition_variable new_written_block_;

  // Input file description
  std::string input_path_;
  std::size_t input_file_size_bytes_;

  // Blocks description
  long int block_size_bytes_;
  std::size_t complete_blocks_;
  bool trailing_block_exists_;

  void WriteResult(std::ofstream& output_stream);
};
