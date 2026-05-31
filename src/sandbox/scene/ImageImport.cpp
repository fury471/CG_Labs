#include "ImageImport.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <locale>
#include <sstream>
#include <utility>

namespace sfm::scene
{
namespace
{

void add_message(ImageImportResult& result, std::string message)
{
	result.messages.emplace_back(std::move(message));
}

std::string lowercase_ascii(std::string text)
{
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return text;
}

bool read_token(std::istream& stream, std::string& token)
{
	token.clear();
	while (stream >> token) {
		if (!token.empty() && token[0] == '#') {
			std::string ignored;
			std::getline(stream, ignored);
			continue;
		}
		return true;
	}
	return false;
}

bool parse_int_token(std::string const& token, int& value)
{
	std::istringstream stream{ token };
	stream.imbue(std::locale::classic());
	stream >> std::noskipws >> value;
	return !stream.fail() && stream.eof();
}

} // namespace

ImageImportResult import_ppm_p3_image(std::filesystem::path const& path)
{
	ImageImportResult result{};
	std::ifstream file{ path };
	if (!file) {
		add_message(result, "Image import failed: could not open '" + path.string() + "'");
		return result;
	}

	std::string token;
	if (!read_token(file, token) || token != "P3") {
		add_message(result, "Image import failed: expected ASCII PPM P3 header");
		return result;
	}

	int width = 0;
	int height = 0;
	int max_value = 0;
	if (!read_token(file, token) || !parse_int_token(token, width) ||
	    !read_token(file, token) || !parse_int_token(token, height) ||
	    !read_token(file, token) || !parse_int_token(token, max_value)) {
		add_message(result, "Image import failed: malformed PPM dimensions or max value");
		return result;
	}
	if (width <= 0 || height <= 0 || max_value <= 0 || max_value > 65535) {
		add_message(result, "Image import failed: unsupported PPM dimensions or max value");
		return result;
	}

	std::vector<glm::vec3> pixels;
	pixels.reserve(static_cast<std::size_t>(width * height));
	for (int i = 0; i < width * height; ++i) {
		int r = 0;
		int g = 0;
		int b = 0;
		if (!read_token(file, token) || !parse_int_token(token, r) ||
		    !read_token(file, token) || !parse_int_token(token, g) ||
		    !read_token(file, token) || !parse_int_token(token, b)) {
			add_message(result, "Image import failed: not enough RGB samples");
			return result;
		}
		glm::vec3 colour{ static_cast<float>(r), static_cast<float>(g), static_cast<float>(b) };
		colour /= static_cast<float>(max_value);
		pixels.push_back(glm::clamp(colour, glm::vec3{ 0.0f }, glm::vec3{ 1.0f }));
	}

	result.succeeded = true;
	result.image.width = width;
	result.image.height = height;
	result.image.pixels = std::move(pixels);
	result.image.source_file = path.string();
	result.image.source_format = "M17 ASCII PPM P3 sample image";
	add_message(result, "Loaded image '" + path.string() + "'");
	add_message(result, "Format: " + result.image.source_format);
	add_message(result, "Image size: " + std::to_string(width) + " x " + std::to_string(height));
	return result;
}

ImageImportResult import_image(std::filesystem::path const& path)
{
	std::string const extension = lowercase_ascii(path.extension().string());
	if (extension == ".ppm")
		return import_ppm_p3_image(path);

	ImageImportResult result{};
	add_message(result, "Image import failed: unsupported file extension '" + extension + "'");
	return result;
}

glm::vec3 sample_image_nearest(ImageResource const& image, int x, int y) noexcept
{
	if (image.empty())
		return glm::vec3{ 1.0f, 0.0f, 1.0f };

	x = std::clamp(x, 0, image.width - 1);
	y = std::clamp(y, 0, image.height - 1);
	return image.pixels[static_cast<std::size_t>(y * image.width + x)];
}

} // namespace sfm::scene
