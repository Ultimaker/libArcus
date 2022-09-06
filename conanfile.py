from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools import files
from conan import ConanFile
from conans import tools

required_conan_version = ">=1.48.0"


class ArcusConan(ConanFile):
    name = "arcus"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libArcus"
    description = "Communication library between internal components for Ultimaker software"
    topics = ("conan", "binding", "cura", "protobuf", "c++")
    settings = "os", "compiler", "build_type", "arch"
    revision_mode = "scm"
    exports = "LICENSE*"

    python_requires = "umbase/0.1.5@ultimaker/testing"
    python_requires_extend = "umbase.UMBaseConanfile"

    options = {
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {
        "shared": True,
        "fPIC": True,
    }
    scm = {
        "type": "git",
        "subfolder": ".",
        "url": "auto",
        "revision": "auto"
    }

    def requirements(self):
        self.requires("umbase/0.1.6@ultimaker/stable")  # required for the CMake build modules
        for req in self._um_data()["requirements"]:
            self.requires(req)

    def config_options(self):
        if self.options.shared and self.settings.compiler == "Visual Studio":
            del self.options.fPIC

    def configure(self):
        self.options["protobuf"].shared = True

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 17)

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def layout(self):
        cmake_layout(self)
        self.cpp.package.libs = ["Arcus"]

        if self.settings.build_type == "Debug":
            self.cpp.package.defines = ["ARCUS_DEBUG"]
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp.package.system_libs = ["pthread"]
        elif self.settings.os == "Windows":
            self.cpp.package.system_libs = ["ws2_32"]

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        packager = files.AutoPackager(self)
        packager.run()
