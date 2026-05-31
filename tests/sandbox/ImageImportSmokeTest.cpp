#include "sandbox/scene/ImageImport.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{

void require(bool condition, char const* message)
{
	if (!condition)
		throw std::runtime_error{ message };
}

std::filesystem::path write_file(std::string const& filename, std::string const& contents)
{
	std::filesystem::path const path = std::filesystem::temp_directory_path() / filename;
	{
		std::ofstream file{ path };
		file << contents;
	}
	return path;
}

} // namespace

int main()
{
	try {
	std::filesystem::path const path = write_file("sfm_image_smoke.ppm", "P3\n2 1\n255\n255 0 0\n0 255 0\n");
	sfm::scene::ImageImportResult const result = sfm::scene::import_image(path);
	std::filesystem::remove(path);
	require(result.succeeded, "image import should load supported PPM P3");
	require(result.image.width == 2, "unexpected image width");
	require(result.image.height == 1, "unexpected image height");
	require(result.image.pixel_count() == 2u, "unexpected image pixel count");
	require(sfm::scene::sample_image_nearest(result.image, -10, 0).r == 1.0f, "nearest sample should clamp x coordinates");

	std::filesystem::path const bad_header = write_file("sfm_image_bad_header.ppm", "P6\n1 1\n255\n0 0 0\n");
	sfm::scene::ImageImportResult const bad_header_result = sfm::scene::import_image(bad_header);
	std::filesystem::remove(bad_header);
	require(!bad_header_result.succeeded, "image import should reject non-P3 PPM");

	std::filesystem::path const missing_samples = write_file("sfm_image_missing.ppm", "P3\n2 1\n255\n255 0 0\n");
	sfm::scene::ImageImportResult const missing_samples_result = sfm::scene::import_image(missing_samples);
	std::filesystem::remove(missing_samples);
	require(!missing_samples_result.succeeded, "image import should reject incomplete PPM samples");

	std::filesystem::path const unsupported = write_file("sfm_image_bad.png", "");
	sfm::scene::ImageImportResult const unsupported_result = sfm::scene::import_image(unsupported);
	std::filesystem::remove(unsupported);
	require(!unsupported_result.succeeded, "image import should reject unsupported extensions");
	return 0;
	} catch (std::exception const& exception) {
		std::cerr << exception.what() << '\n';
		return 1;
	}
}
