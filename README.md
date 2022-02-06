# FileSignatureMaker

# Introduction

When one's trying to check integrity of many large files with MD5 or almost any other hash algorithm, they face the fact that hashing is a linear process where its state depends on all previous input so it cannot be parallelised. But it's possible to split each file into small blocks, concatenate hashes for them and get so called file signature for using it as an identity criterion.

## Build procedure

The project uses [CMake](https://cmake.org/) as a build tool and proposes two ways of acquiring dependencies:
* [Conan](https://conan.io/)
* [nix](https://nixos.org/) (only for Linux)

The dependencies are `OpenSSL` and `Boost.Interprocess`.

Here is an example of build procedure for Windows:
```sh
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

For Linux:
```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE:STRING="Release"
cmake --build .
```

### Linux

The best way to reproduce my environment fast is to install [nix](https://nixos.org/) and execute `nix-shell` in the root directory. In this case it's recommended to set `USE_CONAN` CMake configuration option to `OFF`.

## Testing procedure

Testing was conducted on Linux. Windows has been used for general working check.

I've been used [vmtouch](https://hoytech.com/vmtouch/) to check whether an input file is in file system cache.

I left `tests/` directory though it is not a part of the program. It contains dirty-hand ad-hoc tools which I used to check that the program calculates MD5 correctly.
