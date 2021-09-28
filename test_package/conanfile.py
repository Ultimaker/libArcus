from conans import ConanFile, CMake
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake


class ArcusTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()
        tc = CMakeToolchain(self)
        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None

        # FIXME: Otherwise it throws: error LNK2001: unresolved external symbol "__declspec(dllimport)
        tc.variables["BUILD_STATIC"] = False if self.settings.os != "Windows" else True
        tc.variables["BUILD_SHARED_LIBS"] = True if self.settings.os != "Windows" else False

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        pass # only interested in compiling and linking