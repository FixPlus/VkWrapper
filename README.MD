# VkWrapper
## About

Lightweight C++ wrapper over Vulkan library. This is WIP.

## Prerequisites

* Compiler: gcc 10.0+ / clang 10.0+ / any other compiler supporting core c++20 features
* CMake: 3.18+
* Boost: 1.62+
* Python 3+
* Vulkan SDK 1.3.224.1+: https://vulkan.lunarg.com/

## Download & Build

### bash:

```
git clone https://github.com/FixPlus/VkWrapper.git
cd VkWrapper
mkdir build && cd build
cmake ..
make install -j <num workers>
```

### IDEs and other platforms:

Open it as a cmake project and follow instructions from your IDE.

## Usage

### Import in cmake projects

Use find package

```
find_package(VkWrapper <version>)
```

Link your project to it:

```
target_link_libraries(<target name> PUBLIC VkWrapper::VkWrapper)
```

### Use in code

Include needed headers and you are done

```c++
#include <vkw/Library.hpp>

int main(){
    auto vulkanLibary = vkw::Library();
}
```