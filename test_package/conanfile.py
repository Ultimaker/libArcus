from io import StringIO

from conans import ConanFile, CMake
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.env import VirtualRunEnv
from conans.errors import ConanException


class ArcusTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "VirtualRunEnv"

    def build_requirements(self):
        self.tool_requires("ninja/[>=1.10.0]")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.build_context_activated = ["ninja"]
        cmake.generate()

        venv = VirtualRunEnv(self)
        venv.generate()

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
        cpp_info = self.deps_cpp_info["arcus"]
        if "pyarcus" in cpp_info.components:
            test_buf = StringIO()
            self.run("python -c 'import pyArcus; print(pyArcus.__file__)'", env="conanrun", output=test_buf)
            if self.deps_cpp_info["arcus"].lib_paths[0] not in test_buf.getvalue():
                raise ConanException("pyArcus wasn't build correctly!")
