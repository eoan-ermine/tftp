# TFTP Common

A simple header-only Trivial File Transfer Protocol (*TFTP*) packets parsing and serialization library.

[RFC 1350](https://datatracker.ietf.org/doc/html/rfc1350) (*TFTP Protocol Revision 2*) compilant, [RFC 2347](https://datatracker.ietf.org/doc/html/rfc2347) (*TFTP Option Extension*) support. [RFC 2348](https://datatracker.ietf.org/doc/html/rfc2348) (*TFTP Blocksize Option*), [RFC 2349](https://datatracker.ietf.org/doc/html/rfc2349) (*TFTP Timeout Interval and Transfer Size Options*) compatible.

![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue) ![](https://github.com/eoan-ermine/tftp/actions/workflows/build_and_test.yml/badge.svg) ![](https://github.com/eoan-ermine/tftp/actions/workflows/documentation.yml/badge.svg) ![](https://github.com/eoan-ermine/tftp/actions/workflows/style.yml/badge.svg) [![](https://img.shields.io/badge/docs-blue)](https://eoanermine.com/tftp/)

## Supported platforms

|               |                          |               |               |
|---------------|--------------------------|---------------|---------------|
| **Platform name** | Windows | Linux | MacOS |
| **Minimum required version** | 98 | --- | --- |

## Dependencies

| Dependency name                     | Minimum required version | Ubuntu 22.04                         |
|-------------------------------------|--------------------------|--------------------------------------|
| C++                                 | C++17                    | sudo apt-get install build-essential |
| Doxygen (optional)                  | ---                      | sudo apt-get install doxygen         |
| ClangFormat (development, optional) | ---                      | sudo apt-get install clang-format    |

## Quick start

1. Download and install CMake. Version 3.12.0 is the minimum required.
2. Open a shell. Your development tools must be reachable from this shell through the PATH environment variable.
3. Create a build directory, go to this directory:
```shell
mkdir build
cd build
```
4. Execute this command in the shell replacing `path/to/tftp/source/root` with the path to the root of your tftp source tree:
```shell
cmake path/to/tftp/source/root
```
5. After CMake has finished running, proceed to use IDE project files, or start the build from the build directory:
```shell
cmake --build .
```
The --build option tells cmake to invoke the underlying build tool (make, ninja, xcodebuild, msbuild, etc.)

The underlying build tool can be invoked directly, of course, but the --build option is portable.

6. After tftp has finished building, install it from the build directory:
```shell
cmake --build . --target install
```

The --target option with install parameter in addition to the --build option tells cmake to build the install target.

## Options and variables

Variables customize how the build will be generated. Options are boolean variables, with possible values ON/OFF. Options and variables are defined on the CMake command line like this:

```shell
cmake -DVARIABLE=value path/to/tftp/source
```

### CMake variables

* `BUILD_TESTS: BOOL`

Adds test build targets as a dependencies of the default build target. Defaults to OFF.

* `BUILD_EXAMPLES: BOOL`

Adds examples build targets as a dependencies of the default build target. Defaults to OFF.

## CMake targets

* The `format` target (i.e `ninja format`) will run clang-format on all project files
* The `check-format` target (i.e `ninja check-format`) will verify that project's code follows formatting conventions
* The `docs` target (i.e `ninja docs`) will generate documentation using doxygen
