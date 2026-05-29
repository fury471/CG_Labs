#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "core/FPSCamera.h" // As it includes OpenGL headers, import it after glad

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

//! \brief Namespace containing a few helpers for the LUGG computer graphics labs.
namespace bonobo
{
	//! \brief Formalise mapping between an OpenGL VAO attribute binding,
	//!        and the meaning of that attribute.
	enum class shader_bindings : unsigned int{
		vertices = 0u, //!< = 0, value of the binding point for vertices
		normals,       //!< = 1, value of the binding point for normals
		texcoords,     //!< = 2, value of the binding point for texcoords
		tangents,      //!< = 3, value of the binding point for tangents
		binormals      //!< = 4, value of the binding point for binormals
	};

	//! \brief Association of a sampler name used in GLSL to a
	//!        corresponding texture ID.
	using texture_bindings = std::unordered_map<std::string, GLuint>;

	struct material_data {
		glm::vec3 diffuse{ 0.0f };
		glm::vec3 specular{ 0.0f };
		glm::vec3 ambient{ 0.0f };
		glm::vec3 emissive{ 0.0f };
		float shininess{ 0.0f };
		float indexOfRefraction{ 1.0f };
		float opacity{ 1.0f };
	};

	//! \brief Contains the data for a mesh in OpenGL.
	struct mesh_data {
		GLuint vao{0u};                          //!< OpenGL name of the Vertex Array Object
		GLuint bo{0u};                           //!< OpenGL name of the Buffer Object
		GLuint ibo{0u};                          //!< OpenGL name of the Buffer Object for indices
		GLsizei vertices_nb{0};                  //!< number of vertices stored in bo
		GLsizei indices_nb{0};                   //!< number of indices stored in ibo
		texture_bindings bindings{};             //!< texture bindings for this mesh
		material_data material{};                //!< constant values for the material of this mesh
		GLenum drawing_mode{GL_TRIANGLES};       //!< OpenGL drawing mode, i.e. GL_TRIANGLES, GL_LINES, etc.
		std::string name{"un-named mesh"};       //!< Name of the mesh; used for debugging purposes.
	};

	enum class cull_mode_t : unsigned int {
		disabled = 0u,
		back_faces,
		front_faces
	};

	enum class polygon_mode_t : unsigned int {
		fill = 0u,
		line,
		point
	};

	//! \brief Allocate some objects needed by some helper functions.
	void init();

	//! \brief Deallocate objects allocated by the `init()` function.
	void deinit();

	//! \brief Load objects found in an object/scene file, using assimp.
	//!
	//! @param [in] filename of the object/scene file to load.
	//! @return a vector of filled in `mesh_data` structures, one per
	//!         object found in the input file
	std::vector<mesh_data> loadObjects(std::string const& filename);

	//! \brief Creates an OpenGL texture without any content nor parameters.
	//!
	//! @param [in] width width of the texture to create
	//! @param [in] height height of the texture to create
	//! @param [in] target OpenGL texture target to create, i.e.
	//!             GL_TEXTURE_2D & co.
	//! @param [in] internal_format formatting of the texture, i.e. how many
	//!             channels
	//! @param [in] format formatting of the pixel data, i.e. in which
	//!             layout are the channels stored
	//! @param [in] type data type of the pixel data
	//! @param [in] data what to put in the texture
	GLuint createTexture(uint32_t width, uint32_t height,
	                     GLenum target = GL_TEXTURE_2D,
	                     GLint internal_format = GL_RGBA,
	                     GLenum format = GL_RGBA,
	                     GLenum type = GL_UNSIGNED_BYTE,
	                     GLvoid const* data = nullptr);

	//! \brief Load an image into an OpenGL 2D-texture.
	//!
	//! @param [in] filename path to the texture to load
	//! @param [in] generate_mipmap whether or not to generate a mipmap hierarchy
	//! @return the name of the OpenGL 2D-texture
	GLuint loadTexture2D(std::string const& filename,
	                     bool generate_mipmap = true);

	//! \brief Load six images into an OpenGL cubemap-texture.
	//!
	//! @param [in] posx path to the texture on the left of the cubemap
	//! @param [in] negx path to the texture on the right of the cubemap
	//! @param [in] posy path to the texture on the top of the cubemap
	//! @param [in] negy path to the texture on the bottom of the cubemap
	//! @param [in] posz path to the texture on the back of the cubemap
	//! @param [in] negz path to the texture on the front of the cubemap
	//! @param [in] generate_mipmap whether or not to generate a mipmap hierarchy
	//! @return the name of the OpenGL cubemap-texture
	GLuint loadTextureCubeMap(std::string const& posx, std::string const& negx,
                                  std::string const& posy, std::string const& negy,
                                  std::string const& posz, std::string const& negz,
                                  bool generate_mipmap = true);

	//! \brief Overload of createTexture to allocate a cubemap texture.
	GLuint createTextureCubeMap(uint32_t width, uint32_t height,
	                            GLint internal_format = GL_RGBA,
	                            GLenum format = GL_RGBA,
	                            GLenum type = GL_UNSIGNED_BYTE,
	                            GLvoid const* data = nullptr);

	//! \brief Display a shader build error.
	void displayShaderError(std::string const& message);

	//! \brief Display a link error.
	void displayLinkError(std::string const& message);

	//! \brief Creates one fullscreen quadrilateral mesh.
	mesh_data createFullscreenQuad();
}
