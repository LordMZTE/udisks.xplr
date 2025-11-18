{ clangStdenv
, cmake
, pkg-config
, glib
, luajit
, udisks2
, ...
}:
clangStdenv.mkDerivation {
  name = "udisks.xplr";
  src = ./.;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];
  buildInputs = [
    glib
    luajit
    udisks2.dev
  ];

  postInstall = ''
    echo "return [[$out]]" > $out/udisks/nix_path.lua
  '';
}
