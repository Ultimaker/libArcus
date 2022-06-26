import os

from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools import files
from conan import ConanFile
from conans import tools
from conans.errors import ConanException

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

    python_requires = "umbase/0.1.1@ultimaker/testing", "sipbuildtool/0.1@ultimaker/testing"
    python_requires_extend = "umbase.UMBaseConanfile"

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

    def requirements(self):
        for req in self._um_data(self.version)["requirements"]:
            self.requires(req)
        if self.options.build_python:
            for req in self._um_data(self.version)["requirements_pyarcus"]:
                self.requires(req)

    def config_options(self):
        if self.options.shared and self.settings.compiler == "Visual Studio":
            del self.options.fPIC

    def configure(self):
        self.options["protobuf"].shared = True
        self.options["cpython"].shared = True

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 17)

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()

        tc = CMakeToolchain(self)

        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None

        tc.variables["BUILD_PYTHON"] = self.options.build_python
        if self.options.build_python:
            sip = self.python_requires["sipbuildtool"].module.SipBuildTool(self)
            sip.configure(python_interpreter = self.deps_user_info["cpython"].python)
            sip.generate("pyArcus")

            tc.variables["Python_EXECUTABLE"] = self.deps_user_info["cpython"].python
            tc.variables["Python_USE_STATIC_LIBS"] = not self.options["cpython"].shared
            tc.variables["Python_ROOT_DIR"] = self.deps_cpp_info["cpython"].rootpath
            tc.variables["Python_FIND_FRAMEWORK"] = "NEVER"
            tc.variables["Python_FIND_REGISTRY"] = "NEVER"
            tc.variables["Python_FIND_IMPLEMENTATIONS"] = "CPython"
            tc.variables["Python_FIND_STRATEGY"] = "LOCATION"

            if self.options.shared and self.settings.os == "Windows":
                tc.variables["Python_SITELIB_LOCAL"] = self.cpp.build.bindirs[0]
            else:
                tc.variables["Python_SITELIB_LOCAL"] = self.cpp.build.libdirs[0]

        tc.generate()

    def layout(self):
        self.folders.source = "."
        try:
            build_type = str(self.settings.build_type)
        except ConanException:
            raise ConanException("'build_type' setting not defined, it is necessary for cmake_layout()")
        self.folders.build = f"cmake-build-{build_type.lower()}"
        self.folders.generators = os.path.join(self.folders.build, "conan")
        self.cpp.build.libdirs = ["."]
        self.cpp.build.bindirs = ["."]

        # TODO: Should we split Arcus up in two repo's? That would at least allow us to get rid of the components,
        # we would then only need to recompile pyArcus for each python version. Now we also compile Arcus again

        # libarcus component
        self.cpp.source.components["libarcus"].includedirs = ["arcus_include"]

        self.cpp.build.components["libarcus"].libs = ["Arcus"]
        self.cpp.build.components["libarcus"].libdirs = ["."]
        self.cpp.build.components["libarcus"].bindirs = ["."]

        self.cpp.package.components["libarcus"].includedirs = ["arcus_include"]
        self.cpp.package.components["libarcus"].libs = ["Arcus"]
        self.cpp.package.components["libarcus"].libdirs = ["lib"]
        self.cpp.package.components["libarcus"].bindirs = ["bin"]
        self.cpp.package.components["libarcus"].requires = ["protobuf::protobuf"]
        self.cpp.package.components["libarcus"].defines = ["ARCUS"]
        if self.settings.build_type == "Debug":
            self.cpp.package.components["libarcus"].defines.append("ARCUS_DEBUG")
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp.package.components["libarcus"].system_libs = ["pthread"]
        elif self.settings.os == "Windows":
            self.cpp.package.components["libarcus"].system_libs = ["ws2_32"]

        # pyarcus component
        if self.options.build_python:
            self.cpp.source.components["pyarcus"].includedirs = ["pyarcus_include"]

            self.cpp.build.components["pyarcus"].libdirs = [".", os.path.join("pyArcus", "pyArcus")]

            self.cpp.package.components["pyarcus"].includedirs = ["pyarcus_include"]
            self.cpp.package.components["pyarcus"].libdirs = ["site-packages"]
            self.cpp.package.components["pyarcus"].requires = ["libarcus", "protobuf::protobuf", "cpython::cpython"]

            if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
                self.cpp.package.components["pyarcus"].system_libs = ["pthread"]

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        packager = files.AutoPackager(self)
        packager.patterns.build.lib = ["*.so", "*.so.*", "*.a", "*.lib", "*.dylib", "*.pyd", "*.pyi"]
        packager.run()

        # Workaround for AutoPackager not playing nice with components
        files.rmdir(self, os.path.join(self.package_folder, self.cpp.package.components["libarcus"].libdirs[0], "CMakeFiles"))
        tools.remove_files_by_mask(os.path.join(self.package_folder, self.cpp.package.components["libarcus"].bindirs[0]), "*.exe")
        files.rmdir(self, os.path.join(self.package_folder, self.cpp.package.components["libarcus"].bindirs[0], "CMakeFiles"))
        tools.remove_files_by_mask(os.path.join(self.package_folder, self.cpp.package.components["libarcus"].libdirs[0]), "pyArcus.*")
        files.rmdir(self, os.path.join(self.package_folder, self.cpp.package.components["libarcus"].libdirs[0], "pyArcus"))
        if self.options.build_python:
            files.rmdir(self, os.path.join(self.package_folder, self.cpp.package.components["pyarcus"].libdirs[0], "CMakeFiles"))
            tools.remove_files_by_mask(os.path.join(self.package_folder, self.cpp.package.components["libarcus"].libdirs[0], "pyArcus"), "pyArcus.*")
            files.rmdir(self, os.path.join(self.package_folder, self.cpp.package.components["pyarcus"].libdirs[0], "pyArcus"))
            tools.remove_files_by_mask(os.path.join(self.package_folder, self.cpp.package.components["pyarcus"].libdirs[0]), "Arcus.*")
            tools.remove_files_by_mask(os.path.join(self.package_folder, self.cpp.package.components["pyarcus"].libdirs[0]), "libArcus.*")

    def package_info(self):
        if self.options.build_python:
            if self.in_local_cache:
                self.runenv_info.append_path("PYTHONPATH", self.cpp_info.components["pyarcus"].libdirs[0])
            else:
                self.runenv_info.append_path("PYTHONPATH", self.build_folder)
