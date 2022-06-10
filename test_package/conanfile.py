import shutil

from io import StringIO
from pathlib import Path

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.env import VirtualRunEnv
from conans import tools
from conans.errors import ConanException


class ArcusTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "VirtualRunEnv"

    def build_requirements(self):
        self.tool_requires("ninja/[>=1.10.0]")
        self.tool_requires("cmake/[>=3.23.0]")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()

        venv = VirtualRunEnv(self)
        venv.generate()

        tc = CMakeToolchain(self, generator = "Ninja")
        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None
        tc.generate()

    def build(self):
        if not tools.cross_building(self, skip_x64_x86 = True):
            shutil.copy(Path(self.source_folder).joinpath("test.py"), Path(self.build_folder).joinpath("test.py"))

        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        if self.settings.os == "Windows" and not tools.cross_building(self, skip_x64_x86 = True):
            self.copy("*.dll", dst=".", src="@bindirs")
            self.copy("*.pyd", dst=".", src="@libdirs")

    def test(self):
        if not tools.cross_building(self):
            ext = ".exe" if self.settings.os == "Windows" else ""
            self.run(f"test{ext}", env = "conanrun")

        cpp_info = self.deps_cpp_info["arcus"]
        if "pyarcus" in cpp_info.components:
            test_buf = StringIO()
            self.run(f"python test.py", env = "conanrun", output = test_buf)
            if "True" not in test_buf.getvalue():
                raise ConanException("pyArcus wasn't build correctly!")
