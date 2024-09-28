from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.files import load, copy
from conan.tools.cmake import CMake, cmake_layout

import os

class SimulatorIntel8080Recipe(ConanFile):
    name = "simulator-intel-8080"
    description = "Intel 8080 Simulator"
    package_type = "application"
    license = "MIT"
    homepage = "https://github.com/Galarius/simulator-intel-8080"
    topics = ("intel8080", "simulator")
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualBuildEnv"
    options = {"enable_testing": [True, False]}
    default_options = {"enable_testing": False}
    requires = (
        "cli11/[^2.3.2]",
        "spdlog/[^1.11.0]",
        "systemc/3.0.0"
    )
    exports_sources = (
        "include/**",
        "src/**",
        "tests/**",
        "CMakeLists.txt",
        "version",
        "LICENSE",
    )

    def set_version(self):
        version = load(self, os.path.join(self.recipe_folder, "version"))
        self.version = version.strip()

    def build_requirements(self):
        if self.options.enable_testing:
            self.test_requires("gtest/[^1.13.0]")

    def validate(self):
        check_min_cppstd(self, 17)

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure(
            {
                "ENABLE_TESTING": self.options.enable_testing,
            }
        )
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        build_folder = (
            os.path.join(self.build_folder, str(self.settings.build_type))
            if self.settings.os == "Windows"
            else self.build_folder
        )
        bin_ext = ".exe" if self.settings.os == "Windows" else ""
        copy(
            self,
            f"{self.name}{bin_ext}",
            build_folder,
            os.path.join(self.package_folder, "bin"),
        )
        copy(
            self,
            "LICENSE",
            self.source_folder,
            os.path.join(self.package_folder, "licenses"),
        )

