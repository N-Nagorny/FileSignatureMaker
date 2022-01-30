#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#define MD5_DIGEST_LENGTH 16

void dump(unsigned char const (&a)[MD5_DIGEST_LENGTH]) {
  for (unsigned b : a) {
    std::cout << std::setw(2) << std::setfill('0') << std::hex << b;
  }
  std::cout << std::dec << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2)
    return -1;

  unsigned char output[MD5_DIGEST_LENGTH]{0};
  std::ifstream file_stream{ argv[1], std::fstream::binary };

  const size_t file_size{ std::filesystem::file_size(argv[1]) };
  std::vector<unsigned char> data(file_size);

  file_stream.read(reinterpret_cast<char*>(data.data()), file_size);

  std::size_t start_offset = 0, end_offset = start_offset + MD5_DIGEST_LENGTH;
  for (int i = 0; i < file_size / MD5_DIGEST_LENGTH; ++i) {
    std::move(data.data() + start_offset, data.data() + end_offset, output);
    dump(output);
    start_offset += MD5_DIGEST_LENGTH;
    end_offset += MD5_DIGEST_LENGTH;
  }
  return 0;
}
