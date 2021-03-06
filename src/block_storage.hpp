#pragma once

#include <istream>
#include <map>
#include <vector>
#include <queue>

#include "interface_block_storage.hpp"

// BlockStorage allocates
// num_blocks cells which
// may hold a block or nothing
class BlockStorage : public IBlockStorage {
public:
  explicit BlockStorage(
    std::size_t num_blocks,
    std::size_t block_size_bytes
  )
    : cells_(num_blocks)
    , block_size_bytes_(block_size_bytes)
  {
    for (std::size_t i = 0; i < num_blocks; ++i) {
      cells_[i].resize(block_size_bytes);
      free_cells_.push(i);
    }
  }

  virtual ~BlockStorage() = default;

  const std::vector<unsigned char>& GetBlock(std::size_t index) const override;
  std::size_t GetFreeCells() const override;
  bool ReadBlock(std::istream& input_stream, std::size_t index) override;
  void FreeBlock(std::size_t index) override;

private:
  BlockStorage(const BlockStorage&) =
    delete;  // non construction-copyable
  BlockStorage& operator=(const BlockStorage&) =
    delete;  // non copyable

  std::size_t block_size_bytes_;
  std::vector<std::vector<unsigned char>> cells_; // cell number -> block
  std::map<std::size_t, std::size_t> indices_; // external index -> cell number
  std::queue<std::size_t> free_cells_;
};
