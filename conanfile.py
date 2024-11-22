import os
from shutil import which

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools.env import VirtualBuildEnv
from conan.tools.files import copy, AutoPackager, update_conandata
from conan.tools.microsoft import check_min_vs, is_msvc, is_msvc_static_runtime
from conan.tools.scm import Version, Git


required_conan_version = ">=2.7.0"


class ArcusConan(ConanFile):
    name = "arcus"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libArcus"
    description = "Communication library between internal components for Ultimaker software"
    topics = ("conan", "binding", "cura", "protobuf", "c++")
    settings = "os", "compiler", "build_type", "arch"
    exports = "LICENSE*"
    package_type = "library"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "enable_sentry": [True, False],
        "sentry_project": ["ANY"],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "enable_sentry": False,
        "sentry_project": name,
    }

    def set_version(self):
        if not self.version:
            self.version = self.conan_data["version"]

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
        copy(self, "*", os.path.join(self.recipe_folder, "src"), os.path.join(self.export_sources_folder, "src"))
        copy(self, "*", os.path.join(self.recipe_folder, "include"), os.path.join(self.export_sources_folder, "include"))

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

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
        self.requires("protobuf/3.21.12", transitive_headers=True)

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

        if self.options.enable_sentry:
            for sentry_setting in ["organization", "token"]:
                if self.conf.get(f"user.sentry:{sentry_setting}", "", check_type=str) == "":
                    raise ConanInvalidConfiguration(f"Unable to enable Sentry because no {sentry_setting} was configured")

    def build_requirements(self):
        self.test_requires("standardprojectsettings/[>=0.2.0]@ultimaker/cura_11622")  # FIXME: use stable after merge
        self.tool_requires("protobuf/3.21.12")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["ENABLE_SENTRY"] = self.options.enable_sentry
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

        if self.options.enable_sentry:
            sentry_project = self.options.sentry_project
            sentry_organization = self.conf.get("user.sentry:organization", "", check_type=str)
            sentry_token = self.conf.get("user.sentry:token", "", check_type=str)

            if which("sentry-cli") is None:
                raise ConanException("sentry-cli is not installed, unable to upload debug symbols")

            if self.settings.os == "Linux":
                self.output.info("Stripping debug symbols from binary")
                ext = ".so" if self.options.shared else ".a"
                self.run(f"objcopy --only-keep-debug --compress-debug-sections=zlib libArcus{ext} libArcus.debug")
                self.run(f"objcopy --strip-debug --strip-unneeded libArcus{ext}")
                self.run(f"objcopy --add-gnu-debuglink=libArcus.debug libArcus{ext}")

            build_source_dir = self.build_path.parent.parent.as_posix()
            self.output.info("Uploading debug symbols to sentry")
            self.run(f"sentry-cli --auth-token {sentry_token} debug-files upload --include-sources -o {sentry_organization} -p {sentry_project} {build_source_dir}")

    def package(self):
        copy(self, pattern="LICENSE*", dst="licenses", src=self.source_folder)
        packager = AutoPackager(self)
        packager.run()
