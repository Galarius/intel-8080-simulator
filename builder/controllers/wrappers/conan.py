from .execute import exec
from .wrapper import Wrapper


class Conan(Wrapper):
    def __init__(self):
        super(Conan, self).__init__("conan")

    def create_local_systemc_package(
        self, host_profile, build_profile
    ):
        exec(
            [
                self.executable,
                "create",
                "conan/recipes/systemc/all/conanfile.py",
                "--version",
                "3.0.0",
                *self.__get_configuration_args(host_profile, build_profile, False),
            ],
            check=True,
        )

    def install_dependencies(
        self, host_profile, build_profile, install_folder, with_tests
    ):
        exec(
            [
                self.executable,
                "install",
                ".",
                "--update",
                f"--output-folder={install_folder}",
                "--deployer=conan/licenses",
                *self.__get_configuration_args(host_profile, build_profile, with_tests),
            ],
            check=True,
        )

    def __get_configuration_args(self, host_profile, build_profile, with_tests=False):
        args = [
            f"--profile:host={host_profile}",
            f"--profile:build={build_profile}",
            "--build=missing",
        ]
        if with_tests:
            args.extend(
                [
                    "-o",
                    "simulator-intel-8080/*:enable_testing=True",
                ]
            )
        return args
