vcpkg_from_gitlab(
    OUT_SOURCE_PATH SOURCE_PATH
    GITLAB_URL https://gitlab.xiph.org/
    REPO xiph/speex
    REF "a1b872e6704cc5825750098ce0e0c0b4aacaef4d"
    SHA512 53fa16e5bc90a38a8d19c5b7b6c35489e2129af04951ae557110f8987c05f1e12608f76640774e33cee7335594e01fafad49d11117da822dc9d7df36ad5755c2
)

vcpkg_configure_meson(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -Dtools=disabled
)

vcpkg_install_meson()

vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
