# VkWrapper
## About
Lightweight C++ wrapper over Vulkan library
## Prerequisites
* Compiler: gcc 10.0+ / clang 10.0+ / any other compiler supporting core c++20 features
* CMake: 3.10+
* Vulkan SDK: https://vulkan.lunarg.com/
## Download & Build
* Linux-like environments:
```
git clone https://github.com/FixPlus/VkWrapper.git
cd VkWrapper
mkdir build && cd build
cmake ..
make install -j <num workers>
```
## Usage
### Import
Currently, there are no way to import this library as a package.\
So, to use this library add following directories to your include location list:
```
${VkWrapper_INSTALLDIR}/include
```
And add this directory to your library search path list:
```
${VkWrapper_INSTALLDIR}/lib
```
Also make sure you add next directory to PATH (or LD_LIBARRY_PATH):
```
${VkWrapper_INSTALLDIR}/bin
```
Finnaly, make sure to link with VkWrapper:
```
-lVkWrapper
```
### Use in code
Include needed headers and you are done
```c++
#include <Instance.hpp>

int main(){
    auto vulkanInstance = vkw::Instance();
}
```