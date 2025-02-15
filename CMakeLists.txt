cmake_minimum_required(VERSION 3.30)

project(Sinvi   VERSION 0.9
                LANGUAGES CXX
                DESCRIPTION "Reverse enginering tool for fixed size protocols"
            )

include("build.cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(lib)
add_subdirectory(capturer)
add_subdirectory(drawer)
add_subdirectory(cockpit)
add_subdirectory(fdm)

add_custom_target(config_file
    SOURCES config.ini)

add_custom_target(github_actions
    SOURCES
    .github/workflows/cmake-multi-platform.yml
    .github/workflows/codeql.yml
)

install(TARGETS
    sinviCapturer
    sinviDrawer
    sinviCockpit
    fgfsInvisFDM
    Sinvi
    BUNDLE DESTINATION ${CMAKE_CURRENT_LIST_DIR}/
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/config.ini
    DESTINATION .
)

if (WIN32)
#    install(FILES ${CMAKE_CXX_COMPILER}/bin/*.dll

#        DESTINATION .
#    )
endif()

set(CPACK_RESOURCE_FILE_LICENSE  "${CMAKE_CURRENT_LIST_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "vincent.lecoq@airbus.com")
set(CPACK_PACKAGE_VENDOR "AHJ")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "TGZ;RPM;DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpcap-dev")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CPACK_GENERATOR "PACKAGEMAKER;DRAGANDROP;BUNDLE;OSXX11")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
