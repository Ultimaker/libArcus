from conans import ConanFile, CMake
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake


class ArcusTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def build_requirements(self):
        self.tool_requires("ninja/[>=1.10.0]")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.build_context_activated = ["ninja"]
        cmake.generate()
        tc = CMakeToolchain(self, generator = "Ninja")
        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        pass # only interested in compiling and linking