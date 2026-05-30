#include "sandbox/scene/ImageImport.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
	std::filesystem::path const path = std::filesystem::temp_directory_path() / "sfm_image_smoke.ppm";
	{
		std::ofstream file{ path };
		file << "P3\n";
		file << "2 1\n";
		file << "255\n";
		file << "255 0 0\n";
		file << "0 255 0\n";
	}

	sfm::scene::ImageImportResult const result = sfm::scene::import_image(path);
	std::filesystem::remove(path);
	if (!result.succeeded) {
		std::cerr << "image import failed\n";
		return 1;
	}
	if (result.image.width != 2 || result.image.height != 1 || result.image.pixel_count() != 2u) {
		std::cerr << "unexpected image dimensions\n";
		return 1;
	}
	return 0;
}
