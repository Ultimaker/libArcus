import os
import pathlib

from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.layout import LayoutPackager, clion_layout

class ArcusConan(ConanFile):
    name = "Arcus"
    version = "4.10.0"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libArcus"
    description = "Communication library between internal components for Ultimaker software"
    topics = ("conan", "python", "binding", "sip", "cura", "protobuf", "c++")
    settings = "os", "compiler", "build_type", "arch"
    exports = "LICENSE"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "examples": [True, False],
        "python_version": "ANY"
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "examples": False,
        "python_version": "3.8"
    }
    scm = {
        "type": "git",
        "subfolder": ".",
        "url": "auto",
        "revision": "auto"
    }

    _ext = None

    @property
    def ext(self):
        if self._ext:
            return self._ext
        self._ext = "lib" if self.settings.os == "Windows" else "a"
        if self.options.shared:
            if self.settings.os == "Windows":
                self._ext = "dll"
            elif self.settings.os == "Macos":
                self._ext = "dylib"
            else:
                self._ext = "so"
        return self._ext

    # TODO: Move lib naming logic to python_requires project
    _lib_name = None

    @property
    def lib_name(self):
        if self._lib_name:
            return self._lib_name
        pre = "" if self.settings.os == "Windows" else "lib"
        ext = "" if self.settings.os == "Windows" else f".{self.ext}"
        self._lib_name = f"{pre}{self.name}{ext}"
        return self._lib_name

    def config_options(self):
        if self.settings.os == "Windows" and self.settings.compiler == "gcc":
            self.options.python = False
        self.options["protobuf"].shared = False if self.settings.os == "Macos" else self.options.shared

    def configure(self):
        self.options["SIP"].python_version = self.options.python_version
        self.options["SIP"].shared = self.options.shared
        if self.options.shared or self.settings.compiler == "Visual Studio":
            del self.options.fPIC

    def build_requirements(self):
        self.build_requires("cmake/[>=3.16.2]")

    def requirements(self):
        self.requires("SIP/[>=4.19.24]@riverbankcomputing/testing")
        self.requires("protobuf/3.17.1")

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 17)

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()

        tc = CMakeToolchain(self)

        # FIXME: This shouldn't be necessary (maybe a bug in Conan????)
        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None
            tc.blocks["shared"].values["shared_libs"] = False # FIXME: Otherwise it throws: error LNK2001: unresolved external symbol "__declspec(dllimport)

        tc.variables["ALLOW_IN_SOURCE_BUILD"] = True
        tc.variables["BUILD_PYTHON"] = True
        tc.variables["BUILD_EXAMPLES"] = self.options.examples
        tc.variables["Python_VERSION"] = self.options.python_version
        tc.variables["SIP_MODULE_SITE_PATH"] = "site-packages"

        # FIXME: Otherwise it throws: error LNK2001: unresolved external symbol "__declspec(dllimport)
        tc.variables["BUILD_STATIC"] = not self.options.shared if self.settings.os != "Windows" else True

        tc.generate()

    _cmake = None

    def configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libs = [self.lib_name]
        self.libdirs = ["lib64"]
        if self.in_local_cache:
            self.runenv_info.prepend_path("PYTHONPATH", os.path.join(self.package_folder, "site-packages"))
        else:
            self.runenv_info.prepend_path("PYTHONPATH", os.path.join(pathlib.Path(__file__).parent.absolute(),
                                                                     f"cmake-build-{self.options.build_type}".lower()))
        try:
            tools.rmdir(os.path.join(self.package_folder, "site-packages", str(self.settings.build_type)))
        except:
            pass
        self.cpp_info.defines.append("ARCUS")
        if self.settings.build_type == "Debug":
            self.cpp_info.defines.append("ARCUS_DEBUG")
        self.cpp_info.names["cmake_find_package"] = self.name
        self.cpp_info.names["cmake_find_package_multi"] = self.name
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp_info.system_libs.append("pthread")
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs.append("ws2_32")
