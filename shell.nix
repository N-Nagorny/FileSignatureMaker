{}:

with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "FileSignatureMaker";
  buildInputs = [ boost openssl ];
  nativeBuildInputs = [ cmake pkg-config valgrind ];
}
