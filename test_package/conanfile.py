from conan import ConanFile
from conan.tools.build import can_run
from conan.tools.cmake import cmake_layout, CMake


class ArcusTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain", "VirtualRunEnv"
    test_type = "explicit"

    def build_requirements(self):
        self.test_requires("standardprojectsettings/[>=0.2.0]@ultimaker/cura_11622")  # FIXME: use stable after merge
        self.tool_requires("protobuf/3.21.12")

    def requirements(self):
        self.requires(self.tested_reference_str)
        self.requires("protobuf/3.21.12")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if can_run(self):
            ext = ".exe" if self.settings.os == "Windows" else ""
            prefix_path = "" if self.settings.os == "Windows" else "./"
            self.run(f"{prefix_path}test{ext}", env = "conanrun")
