#include <fstream>
#include <iterator>
#include <streambuf>

#include "per_block_file_processor.hpp"

void PerBlockFileProcessor::ProcessFile(const std::string& output_path, single_block_processor process_block) {
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

  thread_pool_.Startup();

  // This loop reads blocks one by one from an input file stream
  // and makes a future for each block
  for (std::size_t i = 0; i < complete_blocks_; ++i) {
    const unsigned char* block_ptr = nullptr;
    {
      std::unique_lock<std::mutex> lock(block_storage_mutex_);

      new_written_block_.wait(lock, [this, i]() {
        return block_storage_.GetFreeCells() > 0;
      });

      if (!block_storage_.ReadBlock(input_file, i)) {
        throw std::logic_error("block_storage overflow");
      }

      block_ptr = block_storage_.GetBlock(i).data();
    }
    auto processed_block = thread_pool_.EnqueueJob(process_block, block_ptr, block_size_bytes_);

    {
      std::unique_lock<std::mutex> lock(processed_blocks_mutex_);
      processed_blocks_[i] = std::move(processed_block);
    }

    new_processed_block_.notify_one();
  }

  thread_pool_.Shutdown();
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

void PerBlockFileProcessor::WriteResult(std::ofstream& output_stream) {
  for (int i = 0; i < complete_blocks_; ++i) {
    std::future<std::vector<unsigned char>> future;
    {
      std::unique_lock<std::mutex> lock(processed_blocks_mutex_);

      new_processed_block_.wait(lock, [this, i]() {
        return processed_blocks_.count(i);
      });

      future = std::move(processed_blocks_.at(i));
      processed_blocks_.erase(i);
    }

    std::vector<unsigned char> result = future.get();

    {
      std::unique_lock<std::mutex> lock(block_storage_mutex_);
      block_storage_.FreeBlock(i);
    }
    new_written_block_.notify_one();

    output_stream.write(reinterpret_cast<const char*>(result.data()), result.size());
  }
}
