set(CPACK_PACKAGE_VENDOR "Ultimaker")
set(CPACK_PACKAGE_CONTACT "Arjen Hiemstra <a.hiemstra@ultimaker.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libArcus Communication library")
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_VERSION_MAJOR 15)
set(CPACK_PACKAGE_VERSION_MINOR 05)
set(CPACK_PACKAGE_VERSION_PATCH 90)

set(RPM_REQUIRES
    "python3 >= 3.4.0"
    "libgcc >= 4.9.0"
    "libstdc++ >= 4.9.0"
    "glibc >= 2.19"
    "zlib >= 1.2.0"
    "protobuf >= 3.0.0"
)
string(REPLACE ";" "," RPM_REQUIRES "${RPM_REQUIRES}")
set(CPACK_RPM_PACKAGE_REQUIRES ${RPM_REQUIRES})

set(DEB_DEPENDS
    "python3 (>= 3.4.0)"
    "libgcc1 (>= 4.9.0)"
    "libstdc++6 (>= 4.9.0)"
    "libc6 (>= 2.19)"
    "zlib1g (>= 1.2.0)"
    "protobuf (>= 3.0.0)"
)
string(REPLACE ";" ", " DEB_DEPENDS "${DEB_DEPENDS}")
set(CPACK_DEBIAN_PACKAGE_DEPENDS ${DEB_DEPENDS})

include(CPack)
