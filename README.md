# FileSignatureMaker

# Introduction

When one's trying to check integrity of many large files with MD5 or almost any other hash algorithm, they face the fact that hashing is a linear process where its state depends on all previous input so it cannot be parallelised. But it's possible to split each file into small blocks, concatenate hashes for them and get so called file signature for using it as an identity criterion.

## Build procedure

There are `Makefile` for building the program for Linux and `Signature.vcxproj` for Windows.

The program uses `OpenSSL` and `Boost.Interprocess` as dependencies.

### Linux

I've been used `gcc 10.3.0`, `OpenSSL 1.1.1l` and `boost 1.69.0` for building it.

The best way to reproduce my environment fast is to install [nix](https://nixos.org/) and execute `nix-shell` in the root directory.

### Windows

I've been used `msbuild` from `Visual Studio 2019`, `OpenSSL 1.1.1l` and `boost` for building it.

Please make sure that you updated values for `WindowsTargetPlatformVersion`, `IncludePath` and `AdditionalLibraryDirectories` for your environment.

## Testing procedure

Testing was conducted on Linux. Windows has been used for general working check.

I've been used [vmtouch](https://hoytech.com/vmtouch/) to check whether an input file is in file system cache.

I left `tests/` directory though it is not a part of the program. It contains dirty-hand ad-hoc tools which I used to check that the program calculates MD5 correctly.
