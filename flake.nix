{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable-small";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, utils, ... }: utils.lib.eachDefaultSystem (system:
    let
      pkgs = (import nixpkgs { inherit system; });
    in
    {
      devShells.default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
        buildInputs = with pkgs; [
          clang-tools # for clang-format
          glib
          cmake
          pkg-config
          luajit
          udisks2.dev
        ];
      };
      packages.default = (pkgs.callPackage ./. { });
    });
}

