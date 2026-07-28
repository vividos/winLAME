#
# Port file for lame-latest-svn
#

# set this to the SVN revision to get
set(REV r6706)

# When this line doesn't work (anymore), visit this page to regenerate the tarball:
# https://sourceforge.net/p/lame/svn/HEAD/tarball?path=
vcpkg_download_distfile(
    ARCHIVE
    URLS "https://sourceforge.net/code-snapshots/svn/l/la/lame/svn/lame-svn-${REV}-trunk.zip"
    FILENAME lame-svn-${REV}-trunk.zip
    SHA512 d5630bc85937581189e233e008fd1836f2be76e2f13048e4f768c0b91b0ab946586154fca6917a68bc0a9a1db6710850ad70233d1b6c1523c472acd66e030b1c
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
