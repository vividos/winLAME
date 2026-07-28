vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO knik0/faac
    REF "faac-${VERSION}"
    SHA512 6de4f408ab2fd154761e41b2cedf4ebc2795ca502daa9abc1c143d93436d6ddb9c71de97112bb72243ce49abcb7fd4f9d23aadaa5dba9fc155ec27dc9b084b1f
)

vcpkg_configure_meson(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -Dfrontend=false
)

vcpkg_install_meson()

vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
