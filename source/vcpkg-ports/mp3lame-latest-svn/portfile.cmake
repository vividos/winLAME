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

if("mpg123" IN_LIST FEATURES)

    # vs_libmpg123_config.props is expecting the binary distribution of
    # mpg123; modify the file to use vcpkg include and lib files
    list(APPEND FEATURE_OPTIONS "/p:HaveMpg123=true")
    list(APPEND FEATURE_OPTIONS "/p:Mpg123Path=${CURRENT_INSTALLED_DIR}/include")

    vcpkg_replace_string(
        "${SOURCE_PATH}/lame/vc_solution/vs_libmpg123_config.props"
        "<PreLinkEvent>"
        "<PreLinkEvent Condition=\"false\">")

    vcpkg_replace_string(
        "${SOURCE_PATH}/lame/vc_solution/vs_libmpg123_config.props"
        ";%(AdditionalLibraryDirectories)"
        ";${CURRENT_INSTALLED_DIR}/lib;%(AdditionalLibraryDirectories)")

    vcpkg_replace_string(
        "${SOURCE_PATH}/lame/vc_solution/vs_libmpg123_config.props"
        "libmpg123-0.lib"
        "mpg123.lib")

    vcpkg_replace_string(
        "${SOURCE_PATH}/lame/vc_solution/vs_libmpg123_config.props"
        "Condition=\"'$(HaveMpg123)' == 'true' And '$(ConfigurationType)' != 'StaticLibrary'\""
        "Condition=\"false\"")

else()
    list(APPEND FEATURE_OPTIONS "/p:HaveMpg123=false")
endif()

# build static library
vcpkg_install_msbuild(
    SOURCE_PATH ${SOURCE_PATH}/lame
    PROJECT_SUBPATH vc_solution/vs_libmp3lame.vcxproj
    INCLUDES_SUBPATH include
    ALLOW_ROOT_INCLUDES
    LICENSE_SUBPATH COPYING
    OPTIONS
        ${FEATURE_OPTIONS}
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
            ${FEATURE_OPTIONS}
            "/p:HaveLibsndfile=false"
    )

    # and remove the static library again
    file(REMOVE "${CURRENT_PACKAGES_DIR}/lib/libmp3lame-static.lib")
    file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/lib/libmp3lame-static.lib")
endif()
