#pragma once

#include <istream>

class IBlockStorage {
public:
  virtual ~IBlockStorage() = default;

  virtual const std::vector<unsigned char>& GetBlock(std::size_t index) const = 0;
  virtual std::size_t GetFreeCells() const = 0;
  virtual bool ReadBlock(std::istream& input_stream, std::size_t index) = 0;
  virtual void FreeBlock(std::size_t index) = 0;
};
