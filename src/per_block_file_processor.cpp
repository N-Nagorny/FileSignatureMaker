#include <fstream>
#include <iterator>
#include <streambuf>

#include "per_block_file_processor.hpp"

void PerBlockFileProcessor::ProcessFile(const char* output_path, single_block_processor process_block) {
  const unsigned char* input_region_ptr =
    reinterpret_cast<const unsigned char*>(input_region_.get_address());

  std::ofstream output_file(output_path,
    std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
  );
  auto output_iter = std::ostreambuf_iterator<char>(output_file);

  // This thread writes processed blocks one by one
  // as soon as a future for the following one is ready
  std::future<void> file_writer = std::async([this, &output_iter]() {
    WriteResult(output_iter);
  });

  // This loop reads blocks one by one from a memory mapped input file
  // and makes a future for each block
  for (std::size_t i = 0; i < complete_blocks_; ++i, input_region_ptr += block_size_bytes_) {
    auto processed_block = thread_pool_.EnqueueJob(process_block, input_region_ptr, block_size_bytes_);

    {
      std::unique_lock<std::mutex> lock(mutex_);
      processed_blocks_[i] = std::move(processed_block);
    }

    new_processed_block_.notify_one();
  }

  thread_pool_.Shutdown();
  file_writer.get();

  // Processing the trailing block
  if (trailing_block_exists_) {
    std::size_t remainder_size_bytes = input_region_.get_size() - complete_blocks_ * block_size_bytes_;
    std::vector<unsigned char> result = process_block(input_region_ptr, remainder_size_bytes);
    std::copy(result.cbegin(), result.cend(), output_iter);
  }
  output_file.close();
}

void PerBlockFileProcessor::WriteResult(std::ostreambuf_iterator<char>& output_iter) {
  for (int i = 0; i < complete_blocks_; ++i) {
    std::future<std::vector<unsigned char>> future;
    {
      std::unique_lock<std::mutex> lock(mutex_);

      new_processed_block_.wait(lock, [this, i]() {
        return processed_blocks_.count(i);
      });

      future = std::move(processed_blocks_.at(i));
      processed_blocks_.erase(i);
    }

    std::vector<unsigned char> result = future.get();
    std::copy(result.cbegin(), result.cend(), output_iter);
  }
}
