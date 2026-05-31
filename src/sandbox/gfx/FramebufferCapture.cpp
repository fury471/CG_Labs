#include "FramebufferCapture.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <fstream>
#include <system_error>

namespace sfm::gfx
{

FramebufferCaptureResult capture_back_buffer_to_ppm(std::filesystem::path const& output_path,
                                                    int framebuffer_width,
                                                    int framebuffer_height)
{
	FramebufferCaptureResult result{};
	if (framebuffer_width <= 0 || framebuffer_height <= 0) {
		result.messages.emplace_back("Baseline capture failed: framebuffer size is invalid");
		return result;
	}

	if (output_path.has_parent_path()) {
		std::error_code error;
		std::filesystem::create_directories(output_path.parent_path(), error);
		if (error) {
			result.messages.emplace_back("Baseline capture failed: could not create output directory '" + output_path.parent_path().string() + "'");
			return result;
		}
	}

	std::vector<unsigned char> pixels(static_cast<std::size_t>(framebuffer_width) * static_cast<std::size_t>(framebuffer_height) * 3u);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, framebuffer_width, framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	GLenum const read_error = glGetError();
	if (read_error != GL_NO_ERROR) {
		result.messages.emplace_back("Baseline capture failed: glReadPixels reported OpenGL error " + std::to_string(read_error));
		return result;
	}

	std::ofstream image_file{ output_path, std::ios::binary };
	if (!image_file) {
		result.messages.emplace_back("Baseline capture failed: could not open image output '" + output_path.string() + "'");
		return result;
	}
	image_file << "P6\n" << framebuffer_width << ' ' << framebuffer_height << "\n255\n";
	for (int y = framebuffer_height - 1; y >= 0; --y) {
		unsigned char const* const row_begin = pixels.data() + static_cast<std::ptrdiff_t>(y) * framebuffer_width * 3;
		image_file.write(reinterpret_cast<char const*>(row_begin), static_cast<std::streamsize>(framebuffer_width * 3));
	}
	if (!image_file) {
		result.messages.emplace_back("Baseline capture failed: could not write image output '" + output_path.string() + "'");
		return result;
	}

	result.succeeded = true;
	result.messages.emplace_back("Baseline capture wrote image '" + output_path.string() + "'");
	return result;
}

} // namespace sfm::gfx
