# Development Notes

## Prerequisites

### Common

* Git
* Cmake
* Python3
* Build Tools
  * macOS: Command Line Tools
  * Linux: gcc, g++  
  * Windows: 
    * ["Visual Studio Build Tools 2022"](https://visualstudio.microsoft.com/downloads/)
    * In "Visual Studio Installer" check also "Desktop Development with C++"

## Dependencies

### macOS/Linux

```shell
python3 -m venv .pyenv
source .pyenv/bin/activate
pip3 install -r requirements.txt
export CONAN_HOME=$(pwd)/.conan-home
conan profile detect
```

### Windows

```bat
python.exe -m venv .pyenv
.pyenv/Scripts/activate.bat
pip.exe install -r requirements.txt
set CONAN_HOME=$(pwd)/.conan-home
conan.exe profile detect
```

## Building the project

| | |
|---|---|
| `./build.py conan-install` | Install dependencies and gathers licenses|
| `./build.py configure` | Configure the project |
| `./build.py` | Build the project |
| or | |
| `./build.py all` | Perform all of the above commands |

*Run `./build.py [cmd] --help` to learn more about configurable arguments.*

