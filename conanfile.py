import os

from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.files.packager import AutoPackager
from conan.tools.layout import cmake_layout
from conans import ConanFile, tools

required_conan_version = ">=1.44.1"

class ArcusConan(ConanFile):
    name = "arcus"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libArcus"
    description = "Communication library between internal components for Ultimaker software"
    topics = ("conan", "python", "binding", "sip", "cura", "protobuf", "c++")
    settings = "os", "compiler", "build_type", "arch"
    revision_mode = "scm"
    exports = "LICENSE*"
    options = {
        "build_python": [True, False],  # TODO: Fix the Sip recipe first in the https://github.com/ultimaker/conan-ultimaker-index.git
        "shared": [True, False],
        "fPIC": [True, False],
        "examples": [True, False]
    }
    default_options = {
        "build_python": False,
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

    @property
    def _site_packages(self):
        return "site-packages"

    def requirements(self):
        # TODO: Add the Python and SIP requirements. First get it up and running for CuraEngine CI/CT
        self.requires("protobuf/3.17.1")

    def config_options(self):
        self.options["protobuf"].shared = self.options.shared

    def configure(self):
        if self.options.shared or self.settings.compiler == "Visual Studio":
            del self.options.fPIC

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 17)

    def layout(self):
        cmake_layout(self)
        self.cpp.build.libs = ["Arcus"]

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.build_context_activated = ["protobuf"]
        cmake.build_context_build_modules = ["protobuf"]
        cmake.build_context_suffix = {"protobuf": "_BUILD"}
        cmake.generate()

        tc = CMakeToolchain(self)

        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None

        tc.variables["ALLOW_IN_SOURCE_BUILD"] = True
        tc.variables["BUILD_PYTHON"] = self.options.build_python
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package(self):
        packager = AutoPackager(self)
        packager.run()

    def package_info(self):
        # To stay compatible with the FindArcus module. This should be removed when we fully switch to Conan
        self.cpp_info.set_property("cmake_file_name", "Arcus")
        self.cpp_info.set_property("cmake_target_aliases", ["Arcus"])


        self.cpp_info.defines.append("ARCUS")
        if self.settings.build_type == "Debug":
            self.cpp_info.defines.append("ARCUS_DEBUG")
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp_info.system_libs.append("pthread")
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs.append("ws2_32")
        if self.options.build_python:
            self.runenv_info.append("PYTHONPATH", os.path.join(self.package_folder, self._site_packages))