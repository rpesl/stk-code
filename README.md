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

## Documentation
- The SuperTuxKart documentation is located in `README-SuperTuxKart.md`
- The OpenAPI specification is located in `src/rest-api/REST-API-OpenAPI-specification.yaml`

## Directory structure
- The REST API source code is located in `src/rest-api`
- The REST API test code is located in `src/test/rest-api`
