{}:

with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "FileSignatureMaker";
  buildInputs = [ openssl ];
  nativeBuildInputs = [ cmake pkg-config valgrind ];
}
