#include "gltf.h"
#include "gltf_print.h"

int main()
{
	std::filesystem::path gltfFilePath = PROJECT_DIR "/helmet/DamagedHelmet.gltf";
	auto gltf = Aegix::GLTF::load(gltfFilePath);
	if (!gltf.has_value())
	{
		std::cerr << "Failed to load GLTF file: " << gltfFilePath << "\n";
		return 1;
	}

	std::cout << "GLTF file: " << gltfFilePath << "\n\n";
	std::cout << gltf.value() << "\n";

	std::cin.get();
	return 0;
}