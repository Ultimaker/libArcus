import os

from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conans import ConanFile, tools

required_conan_version = ">=1.46.2"

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
        "build_python": [True, False],
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {
        "build_python": True,
        "shared": True,
        "fPIC": True,
    }
    scm = {
        "type": "git",
        "subfolder": ".",
        "url": "auto",
        "revision": "auto"
    }

    @property
    def _site_packages(self):
        return os.path.join("lib", "site-packages")

    def build_requirements(self):
        self.tool_requires("protobuf/3.17.1")
        self.tool_requires("ninja/[>=1.10.0]")

    def requirements(self):
        self.requires("protobuf/3.17.1")

    def system_requirements(self):
        pass  # Add Python here ???


    def config_options(self):
        self.options["protobuf"].shared = self.options.shared

    def configure(self):
        if self.options.shared or self.settings.compiler == "Visual Studio":
            del self.options.fPIC

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 20)

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.build_context_activated = ["protobuf", "ninja"]
        cmake.build_context_build_modules = ["protobuf"]
        cmake.build_context_suffix = {"protobuf": "_BUILD"}
        cmake.generate()

        tc = CMakeToolchain(self, generator = "Ninja")

        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None

        tc.variables["ALLOW_IN_SOURCE_BUILD"] = True
        tc.variables["BUILD_PYTHON"] = self.options.build_python
        tc.variables["Python_VERSION"] = "3.10.4"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("libArcus.*", dst="lib", src=self.build_folder, excludes=("python/*", "CMakeFiles/*"))
        self.copy("*.h", dst="include/Arcus", src=f"{self.source_folder}/src", excludes=("./PlatformSocket_p.h", "./Socket_p.h", "./WireMessage_p.h"))
        self.copy("*.h", dst="include/Arcus", src=f"{self.build_folder}/src")
        self.copy("pyArcus.pyi", dst=self._site_packages, src=f"{self.build_folder}/pyArcus/pyArcus")
        self.copy("pyArcus.so", dst=self._site_packages, src=f"{self.build_folder}/")


    def package_info(self):
        self.cpp_info.components["libarcus"].libs = ["Arcus"]
        self.cpp_info.components["libarcus"].requires = ["protobuf::protobuf"]
        self.cpp_info.components["libarcus"].defines.append("ARCUS")
        if self.settings.build_type == "Debug":
            self.cpp_info.components["libarcus"].defines.append("ARCUS_DEBUG")
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp_info.components["libarcus"].system_libs.append("pthread")
        elif self.settings.os == "Windows":
            self.cpp_info.components["libarcus"].system_libs.append("ws2_32")
        if self.options.build_python:
            self.cpp_info.components["pyarcus"].requires = ["libarcus", "protobuf::protobuf"]
            self.cpp_info.components["pyarcus"].system_libs.append("Python3.10")
            self.runenv_info.append("PYTHONPATH", os.path.join(self.package_folder, self._site_packages))