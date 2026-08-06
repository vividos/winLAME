vcpkg_from_gitlab(
    OUT_SOURCE_PATH SOURCE_PATH
    GITLAB_URL https://gitlab.xiph.org/
    REPO xiph/speex
    REF "a1b872e6704cc5825750098ce0e0c0b4aacaef4d"
    SHA512 7661e5bae859cccbab057c91471fb839537a11fd98eabbe7831c510416e0af4839ecb70247e377ac54d5bca15de80bdad7909093b3f41d0d94140ce59bf91e34
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
