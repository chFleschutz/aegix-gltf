
# Aegix GLTF

<div align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/e/e1/GlTF_logo.svg" width="500"/>
</div>
<br>

Aegix GLTF is a compact library for loading and parsing GLTF 2.0 files in C++. It focuses on direct data mapping, translating all elements of a GLTF file into C++ structs that mirror the GLTF specification. The library uses the [nlohmann/json](https://github.com/nlohmann/json) library for JSON parsing.

For more details about the GLTF format and its capabilities, refer to the [GLTF 2.0 Specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html).

### Supported GLTF Features

- [x] Scene
- [x] Node
- [x] Mesh
- [x] Accessor
- [x] BufferView
- [x] Buffer
- [x] Material
- [x] Texture
- [x] Image
- [x] Sampler
- [ ] Camera
- [ ] Skin
- [ ] Animation

## Getting Started

1. **Clone the repository**

    ```bash
    git clone https://github.com/chFleschutz/aegix-gltf.git
    ```

2. **Generate Project Files**

   Use CMake to generate the project files, or open the folder directly in your CMake-supported IDE (e.g., Visual Studio).
  
3. **Build and run the test project**


### Using the libaray

The project builds as a static library. To use it in another project, follow these steps or refer to the test folder for an example:

1. Include the directory in your CMakeLists.txt file
    ```cmake
    add_subdirectory(path/to/aegix-gltf)
    ```

2. Link the library
    ```cmake
    target_link_libraries(your_target PRIVATE Aegix::GLTF)
    ```


## Usage

1. **Include `gltf.h`**

2. **Load a GLTF file**

    Call `Aegix::GLTF::load` to load a GLTF file. The function returns a `std::optional` which only contains a value if loading the file succeeds.

5. **(Optional) Include `gltf_print.h`**

    Include `gltf_print.h` to define operator overloads for printing the GLTF structs.

### Example

```cpp
#include "gltf.h"
#include "gltf_print.h" // Optional, for printing 

std::filesystem::path gltfFilePath = "path/to/file.gltf";
std::optional<Aegix::GLTF::GLTF> gltf = Aegix::GLTF::load(gltfFilePath);
if (gltf.has_value())
{
    std::cout << gltf.value() << "\n";
}
```


