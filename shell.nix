{}:

with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "signature";
  buildInputs = [ boost openssl ];
  nativeBuildInputs = [ gnumake pkg-config valgrind ];
}
