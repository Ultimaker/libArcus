from io import StringIO
from os import path

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools.env import VirtualBuildEnv
from conan.tools.files import copy, AutoPackager, update_conandata
from conan.tools.microsoft import check_min_vs, is_msvc, is_msvc_static_runtime
from conan.tools.scm import Version, Git
from conans.errors import ConanInvalidSystemRequirements

required_conan_version = ">=1.55.0"


class ArcusConan(ConanFile):
    name = "arcus"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libArcus"
    description = "Communication library between internal components for Ultimaker software"
    topics = ("conan", "binding", "cura", "protobuf", "c++")
    settings = "os", "compiler", "build_type", "arch"
    exports = "LICENSE*"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "enable_sentry": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "enable_sentry": False,
    }

    def set_version(self):
        if not self.version:
            build_meta = "" if self.develop else "+source"
            self.version = self.conan_data["version"] + build_meta

    def export(self):
        git = Git(self)
        update_conandata(self, {"version": self.version, "commit": git.get_commit()})

    @property
    def _min_cppstd(self):
        return 17

    @property
    def _compilers_minimum_version(self):
        return {
            "gcc": "9",
            "clang": "9",
            "apple-clang": "9",
            "msvc": "192",
            "visual_studio": "14",
        }

    def export_sources(self):
        copy(self, "CMakeLists.txt", self.recipe_folder, self.export_sources_folder)
        copy(self, "*", path.join(self.recipe_folder, "src"), path.join(self.export_sources_folder, "src"))
        copy(self, "*", path.join(self.recipe_folder, "include"), path.join(self.export_sources_folder, "include"))

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        if self.conf.get("user.curaengine:sentry_url", "", check_type=str) == "":
            del self.options.enable_sentry

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)
        self.cpp.package.libs = ["Arcus"]

        if self.settings.build_type == "Debug":
            self.cpp.package.defines = ["ARCUS_DEBUG"]
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp.package.system_libs = ["pthread"]
        elif self.settings.os == "Windows":
            self.cpp.package.system_libs = ["ws2_32"]

    def requirements(self):
        self.requires("protobuf/3.21.9", transitive_headers=True)

    def validate(self):
        if self.settings.compiler.cppstd:
            check_min_cppstd(self, self._min_cppstd)
        check_min_vs(self, 192)  # TODO: remove in Conan 2.0
        if not is_msvc(self):
            minimum_version = self._compilers_minimum_version.get(str(self.settings.compiler), False)
            if minimum_version and Version(self.settings.compiler.version) < minimum_version:
                raise ConanInvalidConfiguration(
                    f"{self.ref} requires C++{self._min_cppstd}, which your compiler does not support."
                )

    def build_requirements(self):
        self.test_requires("standardprojectsettings/[>=0.1.0]@ultimaker/stable")

    def generate(self):
        tc = CMakeToolchain(self)
        if is_msvc(self):
            tc.variables["USE_MSVC_RUNTIME_LIBRARY_DLL"] = not is_msvc_static_runtime(self)
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        tc.generate()

        tc = CMakeDeps(self)
        tc.generate()

        tc = VirtualBuildEnv(self)
        tc.generate(scope="build")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if self.options.get_safe("enable_sentry", False):
            # Upload debug symbols to sentry
            sentry_project = self.conf.get("user.curaengine:sentry_project", "", check_type=str)
            sentry_org = self.conf.get("user.curaengine:sentry_org", "", check_type=str)
            if sentry_project == "" or sentry_org == "":
                raise ConanInvalidConfiguration("sentry_project is not set")
            output = StringIO()
            self.run(f"sentry-cli -V", output=output)
            if "sentry-cli" not in output.getvalue():
                raise ConanInvalidSystemRequirements("sentry-cli is not installed")
            self.output.info("Uploading debug symbols to sentry")
            self.run(f"sentry-cli debug-files upload --include-sources -o {sentry_org} -p {sentry_project} .")


    def package(self):
        copy(self, pattern="LICENSE*", dst="licenses", src=self.source_folder)
        packager = AutoPackager(self)
        packager.run()
