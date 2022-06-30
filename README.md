# Oggtube
### Cross-platform C++ Ogg transmuxer

[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/gnu-enjoyer/Oggtube)

Supports both MSVC and G++.

Default CMakeList assumes vcpkg or pkg-config, buildspec is for OneDev but should work with most common CI as well.

### Recent changes

#### v0.0.4
- Replaced slow STL regex with compile-time regex thanks to [CTRE](https://github.com/hanickadot/compile-time-regular-expressions)

#### v0.0.3
- Migrated dev env to Gitpod 

#### v0.0.2
- Extensive refactor and overhaul to improve usability and build system

### External dependencies
- libavcodec-dev
- libavformat-dev
- libssl-dev
