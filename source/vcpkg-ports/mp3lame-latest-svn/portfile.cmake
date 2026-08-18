#
# Port file for lame-latest-svn
#

# set this to the SVN revision to get
set(REV r6784)

# When this line doesn't work (anymore), visit this page to regenerate the tarball:
# https://sourceforge.net/p/lame/svn/HEAD/tarball?path=
vcpkg_download_distfile(
    ARCHIVE
    URLS "https://sourceforge.net/code-snapshots/svn/l/la/lame/svn/lame-svn-${REV}-trunk.zip"
    FILENAME lame-svn-${REV}-trunk.zip
    SHA512 60eb319c0a43a4ea9fa94ed7aac56affea9a9b0b980f52cb97a1ab493d7f5002a47e54c9304d86be934f43aca82c5de84f69b93664fbb62592d572b4176034df
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE ${ARCHIVE}
)

# change RuntimeLibrary for static library
if(VCPKG_CRT_LINKAGE STREQUAL "dynamic")
    vcpkg_replace_string(
        "${SOURCE_PATH}/lame/vc_solution/vs_libmp3lame.vcxproj"
        "<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>"
        "<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>")
    vcpkg_replace_string(
        "${SOURCE_PATH}/lame/vc_solution/vs_libmp3lame.vcxproj"
        "<RuntimeLibrary>MultiThreaded</RuntimeLibrary>"
        "<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>")
endif()

# build static library
vcpkg_install_msbuild(
    SOURCE_PATH ${SOURCE_PATH}/lame
    PROJECT_SUBPATH vc_solution/vs_libmp3lame.vcxproj
    INCLUDES_SUBPATH include
    ALLOW_ROOT_INCLUDES
    LICENSE_SUBPATH COPYING
    OPTIONS
        "/p:HaveMpg123=false"
        "/p:HaveLibsndfile=false"
)

file(REMOVE "${CURRENT_PACKAGES_DIR}/include/lame.def")
file(REMOVE "${CURRENT_PACKAGES_DIR}/include/libmp3lame.sym")

if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")

    # change RuntimeLibrary for dynamic library
    if(VCPKG_CRT_LINKAGE STREQUAL "dynamic")
        vcpkg_replace_string(
            "${SOURCE_PATH}/lame/vc_solution/vs_libmp3lame_dll.vcxproj"
            "<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>"
            "<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>")
        vcpkg_replace_string(
            "${SOURCE_PATH}/lame/vc_solution/vs_libmp3lame_dll.vcxproj"
            "<RuntimeLibrary>MultiThreaded</RuntimeLibrary>"
            "<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>")
    endif()

    # also build dynamic library
    vcpkg_install_msbuild(
        SOURCE_PATH ${SOURCE_PATH}/lame
        PROJECT_SUBPATH vc_solution/vs_libmp3lame_dll.vcxproj
        LICENSE_SUBPATH COPYING
        OPTIONS
            "/p:HaveMpg123=false"
            "/p:HaveLibsndfile=false"
    )

    # and remove the static library again
    file(REMOVE "${CURRENT_PACKAGES_DIR}/lib/libmp3lame-static.lib")
    file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/lib/libmp3lame-static.lib")
endif()
