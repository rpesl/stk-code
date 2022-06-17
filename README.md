# REST API for SuperTuxKart

## Additional dependencies
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [RapidJSON](https://rapidjson.org/)
- [GoogleTest](https://github.com/google/googletest)

## Build instructions
Download and extract the [SuperTuxKart assets](https://github.com/supertuxkart/stk-assets-mobile/releases) and the additional dependencies.
Follow the SuperTuxKart documentation for building SuperTuxKart from source.
Use cmake to initialize and build the project.
The target for SuperTuxKart is `supertuxkart`.
The unit test target is `supertuxkart-test`.

### Building on Windows
To build REST API for SuperTuxKart on Windows, follow these instructions:

1. Clone the project `git clone https://github.com/rpesl/stk-code stk-code`
2. Download and extract the [SuperTuxKart assets](https://github.com/supertuxkart/stk-assets-mobile/releases). Place the `stk-assets` folder next to the `stk-code` folder.
    .
    ├── stk-code
    └── stk-assets
3. Download the Windows dependencies package from [SuperTuxKart - Dependencies latest](https://github.com/supertuxkart/dependencies/releases). For example `dependencies-win-x86_64.zip`. Unzip the file and place the folder inside `stk-code`.
    .
    ├── stk-code
    │   └── dependencies-win-x86_64
    └── stk-assets
4. Download and install [Visual Studio ](https://www.visualstudio.com/downloads/). Select "Desktop development with C++" in the installer.
5. Download CMake from [CMake - download page](https://cmake.org/download/) and install it.
6. Open CMake and
- Set "Where is the source code" to the path of `stk-code`.
- Set "Where to build the binaries" to `.../stk-code/build`.
- Press "Configure"; CMake will ask you if it is OK to create the aforementioned directory, press "Yes". CMake will then ask you about your version of Visual Studio. Confirm your selection; CMake will begin creating the required files for the build in the directory.
- After CMake is finished, press the "Generate" button.
7. Navigate to your build directory and open the `SuperTuxKart.sln` file; Visual Studio will now load the solution.
8. In the "Solution Explorer" on the right, right click on the `supertuxkart` project and select "Set as StartUp project".
9. Open the "Build" menu and select "Build Solution".
10. After building, the game is located at `.../stk-code/build/bin/Debug`, or `.../Release` respectively.


## Documentation
- The SuperTuxKart documentation is located in `README-SuperTuxKart.md`
- The OpenAPI specification is located in `src/rest-api/REST-API-OpenAPI-specification.yaml`

## Directory structure
- The REST API source code is located in `src/rest-api`
- The REST API test code is located in `src/test/rest-api`
