# speex port

This is a port of the [Speex](https://www.speex.org/) library. The original
vcpkg port uses `vcpkg_make_install()` which uses `make` and related Unix
tools and is painfully slow on Windows machines. This overlay port instead
uses `meson` for the build instead. Only windows triplets are supported.
