#include <stdexcept>
#include <string>

#include "block_storage.hpp"

const std::vector<unsigned char>& BlockStorage::GetBlock(std::size_t index) const {
  std::size_t cell_number = indices_.at(index);
  return cells_.at(cell_number);
}

std::size_t BlockStorage::GetFreeCells() const {
  return free_cells_.size();
}

bool BlockStorage::ReadBlock(std::istream& input_stream, std::size_t index) {
  if (!free_cells_.empty()) {
    std::size_t cell_number = free_cells_.front();
    free_cells_.pop();

    input_stream.read(reinterpret_cast<char*>(cells_[cell_number].data()), block_size_bytes_);
    indices_[index] = cell_number;
    return true;
  }
  return false;
}

void BlockStorage::FreeBlock(std::size_t index) {
  if (indices_.count(index) == 0) {
    throw std::logic_error("Block " + std::to_string(index) + " does not exist");
  }

  std::size_t cell_number = indices_.at(index);
  free_cells_.push(cell_number);
  indices_.erase(index);
}
