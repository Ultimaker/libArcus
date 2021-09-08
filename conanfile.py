import os
import pathlib

from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake

class ArcusConan(ConanFile):
    name = "Arcus"
    version = "4.11.0"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libArcus"
    description = "Communication library between internal components for Ultimaker software"
    topics = ("conan", "python", "binding", "sip", "cura", "protobuf", "c++")
    settings = "os", "compiler", "build_type", "arch"
    revision_mode = "scm"
    build_policy = "missing"
    exports = "LICENSE"
    options = {
        "build_python": [True, False],
        "shared": [True, False],
        "fPIC": [True, False],
        "examples": [True, False]
    }
    default_options = {
        "build_python": True,
        "shared": True,
        "fPIC": True,
        "examples": False
    }
    scm = {
        "type": "git",
        "subfolder": ".",
        "url": "auto",
        "revision": "auto"
    }

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.shared = False
            if self.settings.compiler == "gcc":
                self.options.build_python = False
        if self.settings.os == "Macos":
            self.options["protobuf"].shared = False
        else:
            self.options["protobuf"].shared = self.options.shared

    def configure(self):
        if self.options.build_python:
            self.options["SIP"].shared = self.options.shared
        if self.options.shared or self.settings.compiler == "Visual Studio":
            del self.options.fPIC

    def build_requirements(self):
        self.build_requires("cmake/[>=3.16.2]")

    def requirements(self):
        if self.options.build_python:
            self.requires("Python/3.8.10@python/testing")
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

        tc.variables["ALLOW_IN_SOURCE_BUILD"] = True
        tc.variables["BUILD_EXAMPLES"] = self.options.examples
        tc.variables["BUILD_PYTHON"] = self.options.build_python
        if self.options.build_python:
            tc.variables["Python_VERSION"] = self.deps_cpp_info["Python"].version
            tc.variables["SIP_MODULE_SITE_PATH"] = "site-packages"

        tc.variables["BUILD_STATIC"] = not self.options.shared

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
        self.cpp_info.libs = tools.collect_libs(self)
        self.libdirs = ["lib64", "lib"]
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
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp_info.system_libs.append("pthread")
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs.append("ws2_32")
