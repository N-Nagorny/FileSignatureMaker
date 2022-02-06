#include "per_block_file_processor.hpp"

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
      block_storage_ptr_->FreeBlock(i);
    }
    new_written_block_.notify_one();

    output_stream.write(reinterpret_cast<const char*>(result.data()), result.size());
  }
}
