#pragma once

#include <filesystem>
#include <fstream>
#include <future>
#include <functional>
#include <map>
#include <mutex>

#include "interface_block_storage.hpp"

typedef std::function<std::vector<unsigned char>(const unsigned char* const block_ptr, std::size_t size)> single_block_processor;

// This class reads a file block by block,
// passes each block along with a processing function
// to a thread pool and writes processed blocks
// to another file when they're ready
class PerBlockFileProcessor {
public:
  explicit PerBlockFileProcessor(
    const std::string& input_path,
    std::size_t block_size_bytes,
    IBlockStorage* block_storage_ptr
  )
    : input_path_(input_path)
    , block_size_bytes_(block_size_bytes)
    , block_storage_ptr_(block_storage_ptr)
  {
    input_file_size_bytes_ = std::filesystem::file_size(input_path);

    if (input_file_size_bytes_ == 0)
      throw std::logic_error("Input file has zero size");

    complete_blocks_ = input_file_size_bytes_ / block_size_bytes_;
    trailing_block_exists_ = input_file_size_bytes_ % block_size_bytes_;
  }

  virtual ~PerBlockFileProcessor() = default;

  // Calls single_block_processor for each block of the input file
  // and writes the result to output_path even if the file exists
  template <typename JobExecutor>
  void ProcessFile(const std::string& output_path, single_block_processor process_block) {
    std::ifstream input_file(input_path_,
      std::ios_base::in | std::ios_base::binary
    );
    input_file.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);

    std::ofstream output_file(output_path,
      std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
    );
    output_file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    // This thread writes processed blocks one by one
    // as soon as a future for the following one is ready
    std::future<void> file_writer = std::async([this, &output_file]() {
      WriteResult(output_file);
    });

    JobExecutor executor;

    // This loop reads blocks one by one from an input file stream
    // and makes a future for each block
    for (std::size_t i = 0; i < complete_blocks_; ++i) {
      const unsigned char* block_ptr = nullptr;
      {
        std::unique_lock<std::mutex> lock(block_storage_mutex_);

        new_written_block_.wait(lock, [this, i]() {
          return block_storage_ptr_->GetFreeCells() > 0;
        });

        if (!block_storage_ptr_->ReadBlock(input_file, i)) {
          throw std::logic_error("block_storage overflow");
        }

        block_ptr = block_storage_ptr_->GetBlock(i).data();
      }
      auto processed_block = executor.EnqueueJob(process_block, block_ptr, block_size_bytes_);

      {
        std::unique_lock<std::mutex> lock(processed_blocks_mutex_);
        processed_blocks_[i] = std::move(processed_block);
      }

      new_processed_block_.notify_one();
    }

    executor.Shutdown();
    file_writer.get();

    // Processing the trailing block
    if (trailing_block_exists_) {
      std::size_t remainder_size_bytes = input_file_size_bytes_ - complete_blocks_ * block_size_bytes_;
      std::vector<unsigned char> trailing_block(remainder_size_bytes);
      input_file.read(reinterpret_cast<char*>(trailing_block.data()), remainder_size_bytes);

      std::vector<unsigned char> result = process_block(trailing_block.data(), trailing_block.size());
      output_file.write(reinterpret_cast<const char*>(result.data()), result.size());
    }

    input_file.close();
    output_file.close();
  }

private:
  PerBlockFileProcessor(const PerBlockFileProcessor&) =
    delete;  // non construction-copyable
  PerBlockFileProcessor& operator=(const PerBlockFileProcessor&) =
    delete;  // non copyable

  IBlockStorage* block_storage_ptr_;
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
  std::size_t block_size_bytes_;
  std::size_t complete_blocks_;
  bool trailing_block_exists_;

  void WriteResult(std::ofstream& output_stream);
};
