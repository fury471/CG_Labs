#include "assignment5.hpp"

#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/ShaderProgramManager.hpp"
#include "core/node.hpp"

#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>
#include <random>


edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
	// make the window size changeable
	glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);

	bonobo::init();
}

edaf80::Assignment5::~Assignment5()
{
	bonobo::deinit();
}


// Project a vector to the ground (XZ) and normalize it.
// Returns (0,0,0) if the horizontal length is 0.
static inline glm::vec3 xz_unit(glm::vec3 v) {
	v.y = 0.0f;
	float l2 = glm::length2(v);
	return (l2 > 0.0f) ? v / std::sqrt(l2) : glm::vec3(0.0f);
}

// sphere agent for obstacles
struct SphereAgent {
	Node node; // renderable
	glm::vec3 pos{ 0.0f };
	glm::vec3 vel{ 0.0f };
	float r{ 0.1f }; // radius
	bool alive{ true };

	// --- coherent wander state ---
	glm::vec3 target_dir{ 0.0f, 0.0f, 1.0f }; // unit vector
	float target_speed{ 2.0f }; // m/s
	float retarget_timer{ 0.0f }; // seconds left for current path

	// --- self-rotation state ---
	glm::vec3 spin_axis{ 0.0f, 1.0f, 0.0f }; // Y axis (good for XZ-plane gameplay)
	float     spin_speed{ 0.0f };            // rad/s (randomized at spawn)
	float     spin_angle{ 0.0f };            // current angle (accumulated)
};

auto rand_unit = []() {
	// random dir on unit sphere
	float u = (float)rand() / (float)RAND_MAX; // [0,1]
	float v = (float)rand() / (float)RAND_MAX; // [0,1]
	float theta = 2.0f * glm::pi<float>() * u;
	float z = 2.0f * v - 1.0f; // [-1,1]
	float k = std::sqrt(glm::max(0.0f, 1.0f - z * z));
	return glm::normalize(glm::vec3(k * std::cos(theta), z, k * std::sin(theta)));
	};

auto frand = [](float a, float b) {
	return a + ((float)std::rand() / (float)RAND_MAX) * (b - a);
	};

static inline glm::vec3 surface_swapped(float u, float v) {
	// u in [0, ¦Ð], v in [0, 2¦Ð]
	float su = std::sin(u), cu = std::cos(u);
	float sv = std::sin(v), s3v = std::sin(3.0f * v);
	float cv = std::cos(v), c2v = std::cos(2.0f * v), c3v = std::cos(3.0f * v);


	float x = su * (15.0f * sv - 4.0f * s3v) * 3.1f;
	float y_orig = 8.0f * cu * 3.1f;
	float z_orig = su * (14.0f * cv - 5.0f * c2v - 2.0f * c3v) * 3.1f;


	// Swap y and z ¡ú (x, z_orig, y_orig)
	return glm::vec3(x, z_orig, y_orig);
}

static std::vector<glm::vec3> sample_random(int N) {
	std::vector<glm::vec3> P; P.reserve(N);
	for (int i = 0; i < N; ++i) {
		float u = frand(0.0f, glm::pi<float>()); // [0, ¦Ð]
		float v = frand(0.0f, glm::two_pi<float>()); // [0, 2¦Ð]
		P.push_back(surface_swapped(u, v));
	}
	return P;
}

static std::vector<glm::vec3> sample_grid(int Nu, int Nv) {
	std::vector<glm::vec3> P; P.reserve(Nu * Nv);
	for (int i = 0; i < Nu; ++i) {
		for (int j = 0; j < Nv; ++j) {
			float u = (i + 0.5f) / Nu * glm::pi<float>();
			float v = (j + 0.5f) / Nv * glm::two_pi<float>();
			P.push_back(surface_swapped(u, v));
		}
	}
	return P;
}

static std::vector<Node> paint_nodes; // one node per tiny sphere
static bool show_painted = false; // toggle on/off
static int paint_N = 800; // for random mode
static int paint_Nu = 60, paint_Nv = 120; // for grid mode
static float paint_radius = 0.20f; // little sphere size
static bool use_grid = false; // choose sampler

// Shaking (super simple: vertical bob)
static bool  shake_enable = true;
static float shake_amp = 1.0f;   // amplitude in world units
static float shake_freq = 2.2f;    // Hz
static float shake_time = 0.0f;

// Per-sphere data aligned with paint_nodes
static std::vector<glm::vec3> paint_anchors; // original positions
static std::vector<float>     paint_phase;   // random phase per sphere



static std::vector<SphereAgent> spheres;



// World bounds (axis-aligned box)
static glm::vec3 BMIN{ -200.0f, -1000.0f, -200.0f };
static glm::vec3 BMAX{ 200.0f, 1000.0f, 200.0f };

// Wander & physics knobs
static float obs_drag = 1.5f;	// linear damping (s^-1)
static float obs_vmax = 24.0f;	// speed cap (m/s)
static float vmin = 0.5f;		// min wander speed
static float vmax = 24.0f;		// max wander speed
static float Tmin = 10.0f;		// min seconds before changing path
static float Tmax = 40.0f;		// max seconds before changing path
static float align_rate = 2.5f; // how fast velocity aligns to target (s^-1)
static float boundary_margin = 1.0f; // start steering away when this close to walls

// Collision knobs
static float restitution = 0.5f; // bounciness on wall hits (0..1)
static float pad = 0.01f; // tiny padding to avoid jitter
static bool game_over = false;
static bool test_winning = false;
static float equal_eps_rel = 0.02f; // 2% tolerance for "same size"
static float bounce_speed = 12.0f; // how fast to shoot equal-size sphere away

static float best_score = 0.0f; // highest player_radius reached this session

static float spin_min = -10.0f; // rad/s  (negative allows clockwise)
static float spin_max = 10.0f; // rad/s



void
edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 1.0f, 80.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(4.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
	                                         { { ShaderType::vertex, "common/fallback.vert" },
	                                           { ShaderType::fragment, "common/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
		{ { ShaderType::vertex, "EDAF80/diffuse.vert" },
		  { ShaderType::fragment, "EDAF80/diffuse.frag" } },
		diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	GLuint normal_shader = 0u;
	program_manager.CreateAndRegisterProgram("Normal",
		{ { ShaderType::vertex, "EDAF80/normal.vert" },
		  { ShaderType::fragment, "EDAF80/normal.frag" } },
		normal_shader);
	if (normal_shader == 0u)
		LogError("Failed to load normal shader");

	GLuint texcoord_shader = 0u;
	program_manager.CreateAndRegisterProgram("Texture coords",
		{ { ShaderType::vertex, "EDAF80/texcoord.vert" },
		  { ShaderType::fragment, "EDAF80/texcoord.frag" } },
		texcoord_shader);
	if (texcoord_shader == 0u)
		LogError("Failed to load texcoord shader");

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader);
	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");

	GLuint phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
		{ { ShaderType::vertex, "EDAF80/phong.vert" },
		  { ShaderType::fragment, "EDAF80/phong.frag" } },
		phong_shader);
	if (phong_shader == 0u)
		LogError("Failed to load phong shader");

	auto light_position = glm::vec3(0.0f, 100.0f, 0.0f);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		};


	bool use_normal_mapping = true;
	camera_position = mCamera.mWorld.GetTranslation();
	auto const phong_set_uniforms = [&use_normal_mapping, &light_position, &camera_position](GLuint program) {
		glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		};

	GLuint tex_diffuse = bonobo::loadTexture2D(config::resources_path("textures/cobblestone_floor_08_diff_2k.jpg"));
	GLuint tex_spec = bonobo::loadTexture2D(config::resources_path("textures/cobblestone_floor_08_rough_2k.jpg"));
	GLuint tex_normal = bonobo::loadTexture2D(config::resources_path("textures/cobblestone_floor_08_nor_2k.jpg"));
	bonobo::material_data demo_material;
	demo_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	demo_material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	demo_material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	demo_material.shininess = 90.0f;

	//
	// Todo: Load your geometry
	// This is the player
	float player_radius = 0.5f;
	const float player_mesh_radius = player_radius;
	auto player_shape = parametric_shapes::createSphere(player_mesh_radius, 20u, 20u);
	glm::vec3 player_velocity(0.0f);
	auto unit_sphere = parametric_shapes::createSphere(1.0f, 20u, 20u);

	auto build_painted_spheres = [&]() {
		paint_nodes.clear();
		std::vector<glm::vec3> P = use_grid ? sample_grid(paint_Nu, paint_Nv) : sample_random(paint_N);

		// Keep anchors and random phases aligned with paint_nodes
		paint_anchors = P;
		paint_phase.resize(P.size());
		for (size_t i = 0; i < P.size(); ++i)
			paint_phase[i] = frand(0.0f, glm::two_pi<float>());

		// Reset time so the shake starts clean
		shake_time = 0.0f;

		paint_nodes.resize(P.size());

		for (size_t i = 0; i < P.size(); ++i) {
			auto& n = paint_nodes[i];
			n.set_geometry(unit_sphere);
			n.set_program(&normal_shader, set_uniforms);
			n.set_material_constants(demo_material);
			n.get_transform().SetTranslate(P[i]);
			n.get_transform().SetScale(glm::vec3(paint_radius));
		}
	};

	auto spawn_spheres = [&](int N, float rmin, float rmax, uint32_t seed) {
		std::mt19937 rng(seed);
		std::uniform_real_distribution<float> urx(BMIN.x, BMAX.x);
		std::uniform_real_distribution<float> ury(BMIN.y, BMAX.y);
		std::uniform_real_distribution<float> urz(BMIN.z, BMAX.z);
		std::uniform_real_distribution<float> urr(rmin, rmax);

		spheres.clear();
		spheres.reserve(N);

		auto no_overlap = [&](glm::vec3 p, float r) {
			for (auto const& s : spheres) {
				float min_d = (r + s.r + pad);
				if (glm::length(p - s.pos) < min_d) return false;
			}
			return true;
			};


		const int MAX_TRIES = 300;
		for (int i = 0; i < N; ++i) {
			float r = urr(rng);
			bool placed = false;
			for (int tries = 0; tries < MAX_TRIES && !placed; ++tries) {
				glm::vec3 p{ urx(rng), ury(rng), urz(rng) };
				// Ensure the sphere is fully inside bounds
				p = glm::clamp(p, BMIN + glm::vec3(r), BMAX - glm::vec3(r));
				if (!no_overlap(p, r)) continue;


				SphereAgent a; a.r = r; a.pos = p; a.vel = glm::vec3(0.0f);
				a.node.set_geometry(unit_sphere);
				a.node.get_transform().SetScale(glm::vec3(r));
				a.node.set_material_constants(demo_material);
				a.node.set_program(&phong_shader, phong_set_uniforms);
				a.node.add_texture("diffuse_map", tex_diffuse, GL_TEXTURE_2D);
				a.node.add_texture("specular_map", tex_spec, GL_TEXTURE_2D);
				a.node.add_texture("normal_map", tex_normal, GL_TEXTURE_2D);
				a.target_dir = rand_unit();
				a.target_speed = frand(vmin, vmax);
				a.retarget_timer = frand(Tmin, Tmax);
				// start moving already toward that heading
				a.vel = a.target_dir * a.target_speed;
				a.spin_axis = glm::normalize(glm::vec3(frand(-1, 1), frand(-1, 1), frand(-1, 1)));


				// --- self-rotation init ---
				a.spin_speed = frand(spin_min, spin_max);                 // random speed in rad/s
				a.spin_angle = frand(0.0f, 2.0f * glm::pi<float>());      // random initial angle

				spheres.push_back(std::move(a));
				placed = true;
			}
			if (!placed) {
				LogError("Spawn: could not place sphere %d after %d tries; stopping at %d.", i, MAX_TRIES, (int)spheres.size());
				break; // give up gracefully
			}
		}
		};

	// Remove spheres quickly: swap with last, then pop (O(1))
	auto swap_pop = [&](size_t i) {
		spheres[i] = std::move(spheres.back());
		spheres.pop_back();
		};


	// Relative-epsilon equality: |a - b| <= eps * max(1, max(a,b))
	auto radii_equal = [&](float a, float b) {
		float m = std::max(1.0f, std::max(a, b));
		return std::abs(a - b) <= equal_eps_rel * m;
		};


	// Grow player by conserving volume: r' = (r^3 + 0.8 * reaten^3)^(1/3)
	auto grow_player = [&](float eaten_r) {
		float r3 = player_radius * player_radius * player_radius + 0.8f *eaten_r * eaten_r * eaten_r;
		player_radius = std::cbrt(r3);
		};


	spawn_spheres(300, 0.1f, 3.0f, /*seed*/ 2471u);

	auto sky_shape = parametric_shapes::createSphere(380.0f, 50u, 50u);
	// player state (world_position and size)
	glm::vec3 player_position(0.0f, 0.0f, mCamera.mWorld.GetTranslation().z-10.0f);


	// Feel tuning (tweak in ImGui later)
	float thrust_base = 20.0f;  // m/s^2  ¡ª how hard W accelerates
	float thrust_boost = 2.5f;   // ¡Á when holding Space
	float max_speed = 40.0f;  // m/s  ¡ª top speed before boost
	float drag = 1.5f;   // s^-1   ¡ª linear damping

	// render node
	Node player_node;
	player_node.set_geometry(player_shape);
	player_node.set_program(&normal_shader, set_uniforms);

	player_node.get_transform().SetTranslate(player_position);


	Node sky_node;
	sky_node.set_geometry(sky_shape);
	sky_node.set_program(&skybox_shader, [](GLuint) {});
	sky_node.get_transform().SetTranslate(glm::vec3(0.0f));


	// load the cubemap:
	GLuint cubemap = bonobo::loadTextureCubeMap(
		config::resources_path("cubemaps/CyberUniverse/posx.jpg"),
		config::resources_path("cubemaps/CyberUniverse/negx.jpg"),
		config::resources_path("cubemaps/CyberUniverse/posy.jpg"),
		config::resources_path("cubemaps/CyberUniverse/negy.jpg"),
		config::resources_path("cubemaps/CyberUniverse/posz.jpg"),
		config::resources_path("cubemaps/CyberUniverse/negz.jpg"));

	sky_node.add_texture("cubemap", cubemap, GL_TEXTURE_CUBE_MAP);


	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	auto lastTime = std::chrono::high_resolution_clock::now();

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();


		mCamera.Update(deltaTimeUs, inputHandler, /*ignoreKeyEvents*/ true, /*ignoreMouseEvents*/ false);

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);



		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);


		//
		// Todo: If you need to handle inputs, you can do it here
		//
		// 1) Read keys
		bool W = inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED;
		bool A = inputHandler.GetKeycodeState(GLFW_KEY_A) & PRESSED;
		bool S = inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED;
		bool D = inputHandler.GetKeycodeState(GLFW_KEY_D) & PRESSED;
		bool SP = inputHandler.GetKeycodeState(GLFW_KEY_SPACE) & PRESSED;
		bool Q = inputHandler.GetKeycodeState(GLFW_KEY_Q) & PRESSED; // up
		bool E = inputHandler.GetKeycodeState(GLFW_KEY_E) & PRESSED; // down


		// 2) Build a camera-relative basis on the ground
		glm::vec3 cam_fwd_h = xz_unit(mCamera.mWorld.GetFront());    // horizontal forward
		glm::vec3 cam_right = glm::normalize(glm::cross(cam_fwd_h, glm::vec3(0, 1, 0)));

		// 3) Combine WASD into a desired direction
		glm::vec3 wishdir(0.0f);
		if (W) wishdir += cam_fwd_h;
		if (S) wishdir -= cam_fwd_h;
		if (D) wishdir += cam_right;
		if (A) wishdir -= cam_right;
		if (Q) wishdir += glm::vec3(0, 1, 0); // Q: up
		if (E) wishdir -= glm::vec3(0, 1, 0); // E: down
		if (wishdir != glm::vec3(0))
			wishdir = glm::normalize(wishdir);

		// Delta time (seconds) with a safety cap to avoid huge steps
		float dt = std::chrono::duration<float>(deltaTimeUs).count();
		dt = glm::min(dt, 0.033f); // ~30 Hz cap for physics safety

		// Boost on Space: increase both accel and top speed so it feels consistent
		float boost = (SP ? thrust_boost : 1.0f);

		// Acceleration from wishdir
		glm::vec3 acc = wishdir * thrust_base * boost;

		// Integrate velocity with simple linear damping
		player_velocity += acc * dt;
		player_velocity -= player_velocity * (drag * dt);

		// Clamp top speed (post-damping)
		float speed = glm::length(player_velocity);
		float player_vmax = max_speed * boost;
		if (speed > player_vmax && SP) player_velocity = (player_velocity / speed) * player_vmax;
		else if (speed > max_speed) player_velocity = (player_velocity / speed) * max_speed;

		// Integrate position (no ground clamp ¡ª Q/E controls vertical)
		player_position += player_velocity * dt;
		player_position.y = 0.0f; // keep on ground plane

		// Camera follows player
		float follow_distance_boost = 0.0f;
		static float follow_distance = 28.0f;
		static float follow_height = 10.0f;
		static float follow_smooth = 20.0f; // 0 = hard snap, higher = smoother
		// Where to put the camera this frame
		glm::vec3 back_h = xz_unit(mCamera.mWorld.GetFront()); // camera forward projected; use as heading reference
		glm::vec3 desired_cam_pos = player_position - back_h * follow_distance + glm::vec3(0, follow_height, 0);
		if (SP) {
			follow_distance_boost = follow_distance * 1.2f; // zoom in when boosting
			glm::vec3 desired_cam_pos = player_position - back_h * follow_distance_boost + glm::vec3(0, follow_height, 0);
		}
		// Optional critically damped like smoothing (simple exponential smoothing here)
		glm::vec3 current_cam_pos = mCamera.mWorld.GetTranslation();
		glm::vec3 new_cam_pos = glm::mix(current_cam_pos, desired_cam_pos, glm::clamp(follow_smooth * dt, 0.0f, 1.0f));
		mCamera.mWorld.SetTranslate(new_cam_pos);


		for (auto& s : spheres) {
			// retarget timing
			s.retarget_timer -= dt;

			// proximity bias (steer away from walls before hitting them)
			bool near_wall = false;
			glm::vec3 inward_bias(0.0f);
			for (int ax = 0; ax < 3; ++ax) {
				float minA = (&BMIN.x)[ax] + s.r + boundary_margin;
				float maxA = (&BMAX.x)[ax] - s.r + -boundary_margin;
				float x = (&s.pos.x)[ax];
				if (x < minA) { near_wall = true; (&inward_bias.x)[ax] += 1.0f; }
				if (x > maxA) { near_wall = true; (&inward_bias.x)[ax] -= 1.0f; }
			}

			if (s.retarget_timer <= 0.0f || near_wall) {
				glm::vec3 new_dir = rand_unit();
				if (near_wall) new_dir = glm::normalize(new_dir + 0.8f * glm::normalize(inward_bias));
				if (glm::abs(new_dir.y) > 0.95f) new_dir.y *= 0.7f; // discourage vertical stalls
				s.target_dir = new_dir;
				s.target_speed = frand(vmin, vmax);
				s.retarget_timer = frand(Tmin, Tmax);
			}

			// steer velocity toward target (exponential smoothing)
			glm::vec3 desired_vel = s.target_dir * s.target_speed;
			float k = 1.0f - std::exp(-align_rate * dt); // 0..1
			s.vel = glm::mix(s.vel, desired_vel, glm::clamp(k, 0.0f, 1.0f));

			// mild obstacle drag + speed cap
			s.vel -= s.vel * (obs_drag * dt);
			float sp = glm::length(s.vel);
			if (sp > obs_vmax) s.vel = (s.vel / sp) * obs_vmax;

			// integrate position
			s.pos += s.vel * dt;
			s.pos.y = 0.0f; // keep on ground plane

			// Bounce against AABB walls: keep fully inside bounds
			for (int axis = 0; axis < 3; ++axis) {
				float minA = (&BMIN.x)[axis] + s.r;
				float maxA = (&BMAX.x)[axis] - s.r;
				float& x = (&s.pos.x)[axis];
				float& v = (&s.vel.x)[axis];
				float& hd = (&s.target_dir.x)[axis];
				if (x < minA) { x = minA; v = -v * restitution; hd = glm::abs(hd); s.retarget_timer = glm::min(s.retarget_timer, frand(0.3f, 1.0f)); }
				if (x > maxA) { x = maxA; v = -v * restitution; hd = -glm::abs(hd); s.retarget_timer = glm::min(s.retarget_timer, frand(0.3f, 1.0f)); }
			}

			// self-rotation (accumulate angle)
			s.spin_angle += s.spin_speed * dt;

			// keep angle in [0, 2¦Ð) to avoid huge numbers (optional but neat)
			if (s.spin_angle >= glm::two_pi<float>()) s.spin_angle -= glm::two_pi<float>();
			if (s.spin_angle < 0.0f)                  s.spin_angle += glm::two_pi<float>();


		}

		// Resolve sphere-sphere overlaps: pairwise push apart
		for (size_t i = 0; i < spheres.size(); ++i) {
			for (size_t j = i + 1; j < spheres.size(); ++j) {
				auto& a = spheres[i];
				auto& b = spheres[j];
				glm::vec3 d = b.pos - a.pos;
				float dist = glm::length(d);
				float min_d = a.r + b.r + pad;
				if (dist < min_d && dist > 1e-6f) {
					glm::vec3 n = d / dist;
					float penetration = (min_d - dist);
					// push both half the penetration
					a.pos -= 0.5f * penetration * n;
					b.pos += 0.5f * penetration * n;
					// nudge headings so they diverge next frame (keeps motion smooth)
					a.target_dir = glm::normalize(glm::mix(a.target_dir, -n, 0.2f));
					b.target_dir = glm::normalize(glm::mix(b.target_dir, n, 0.2f));
				}
			}
		}

		// Player & Sphere interactions (eat / equal-size bounce / death)
		if (!game_over) {
			for (size_t i = 0; i < spheres.size(); /* increment inside */) {
				auto& s = spheres[i];


				// 1) Touch test: use squared distance for speed
				glm::vec3 d = s.pos - player_position;
				float dist2 = glm::dot(d, d);
				float rsum = player_radius + s.r;
				if (dist2 > rsum * rsum) { ++i; continue; } // not touching


				// 2) We are touching: compute contact normal (player to sphere)
				float dist = std::sqrt(std::max(dist2, 1e-12f));
				glm::vec3 n = (dist > 0.0f) ? d / dist : glm::vec3(1, 0, 0);


				if (player_radius > s.r && !radii_equal(player_radius, s.r)) {
					// Case A: Player bigger ¡ú eat the sphere
					grow_player(s.r);


					// Optional juice: a tiny momentum feel
					// player_velocity += 0.1f * s.vel;


					swap_pop(i); // remove sphere in O(1); do NOT i++ here
					continue; // the swapped sphere now sits at index i
				}
				else if (radii_equal(player_radius, s.r)) {
					// Case B: Same size ¡ú bounce the SPHERE away


					// Ensure minimal separation just outside contact
					float target_dist = rsum + pad;
					float push = target_dist - dist;
					if (push > 0.0f) s.pos += n * push; // move sphere outward


					// Give it a clear new velocity in that direction
					float current = glm::length(s.vel);
					float shoot = std::max(bounce_speed, current);
					s.vel = n * shoot;


					// Keep its coherent wander pointing that way for a while
					s.target_dir = n;
					s.target_speed = shoot;
					s.retarget_timer = frand(3.0f, 8.0f);


					++i; // handled; move to next sphere
				}
				else {
					// Case C: Player smaller ¡ú game over
					game_over = true;
					break;
				}
			}
		}

		// Update node transforms for rendering
		for (auto& s : spheres) {
			s.node.get_transform().SetTranslate(s.pos);
			s.node.get_transform().SetScale(glm::vec3(s.r));
			s.node.get_transform().RotateX(s.spin_axis.x* s.spin_speed* dt);
			s.node.get_transform().RotateY(s.spin_axis.y* s.spin_speed* dt);
			s.node.get_transform().RotateZ(s.spin_axis.z* s.spin_speed* dt);
		}

		if (inputHandler.GetKeycodeState(GLFW_KEY_P) & JUST_RELEASED) {
			show_painted = !show_painted;
			if (show_painted) build_painted_spheres();
		}


		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		if (!shader_reload_failed) {
			//
			// Todo: Render all your geometry here.
			//
			if (!game_over) {
				sky_node.render(mCamera.GetWorldToClipMatrix());
				player_node.get_transform().SetScale(glm::vec3(player_radius / player_mesh_radius));
				player_node.get_transform().SetTranslate(player_position);
				player_node.render(mCamera.GetWorldToClipMatrix()); // parent defaults to identity


				if (show_painted) {
					if (shake_enable)
						shake_time += dt;
					if (show_painted && shake_enable) {
						float w = glm::two_pi<float>() * shake_freq; // rad/s
						for (size_t i = 0; i < paint_nodes.size(); ++i) {
							float y_off = shake_amp * std::sin(w * shake_time + paint_phase[i]);
							paint_nodes[i].get_transform().SetTranslate(paint_anchors[i] + glm::vec3(0.0f, y_off, 0.0f));
						}
					}


					for (auto& n : paint_nodes)
						n.render(mCamera.GetWorldToClipMatrix());
				}

				for (auto& s : spheres)
					s.node.render(mCamera.GetWorldToClipMatrix());
			}
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//
		bool const opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);

			ImGui::SliderFloat("Follow distance", &follow_distance, 1.0f, 60.0f);
			ImGui::SliderFloat("Follow height", &follow_height, 0.0f, 60.0f);
			ImGui::SliderFloat("Follow smooth", &follow_smooth, 0.0f, 60.0f);
			ImGui::Separator();
			ImGui::SliderFloat("Thrust", &thrust_base, 0.0f, 50.0f);
			ImGui::SliderFloat("Drag", &drag, 0.0f, 10.0f);
			ImGui::SliderFloat("Vmax", &max_speed, 0.0f, 40.0f);

			ImGui::Separator();
			ImGui::Text("Painted Surface (Easy Mode)");
			ImGui::Checkbox("Use grid sampler", &use_grid);
			if (use_grid) {
				ImGui::SliderInt("Nu", &paint_Nu, 4, 128);
				ImGui::SliderInt("Nv", &paint_Nv, 4, 256);
			}
			else {
				ImGui::SliderInt("N random", &paint_N, 10, 10000);
			}
			ImGui::SliderFloat("Dot radius", &paint_radius, 0.03f, 0.6f);
			if (ImGui::Button("Build / Rebuild")) build_painted_spheres();
			ImGui::SameLine(); ImGui::Checkbox("Show painted", &show_painted);
			ImGui::Checkbox("Enable shake", &shake_enable);
			ImGui::SliderFloat("Shake amp", &shake_amp, 0.0f, 1.0f);
			ImGui::SliderFloat("Shake freq", &shake_freq, 0.0f, 10.0f);

			ImGui::Text("Score (radius): %.3f", player_radius);
			// Optional extra info:
			ImGui::Text("Spheres Left: %d", (int)spheres.size());
			ImGui::Text("Best Score: %.3f", best_score);

			ImGui::Separator();
			if (ImGui::Button("Restart")) {
				// Reset player
				player_position = glm::vec3(0.0f, 0.0f, mCamera.mWorld.GetTranslation().z - 10.0f);
				player_velocity = glm::vec3(0.0f);
				if (best_score < player_radius) best_score = player_radius;
				player_radius = player_mesh_radius; // your initial size

				// Respawn obstacles (same call you used at startup)
				spawn_spheres(300, 0.1f, 3.0f, 2471u);

				test_winning = false;
				show_painted = false;
				shake_enable = false;
				game_over = false; // back to playing
			}
			ImGui::SameLine();
			if (ImGui::Button("Test Winning!") || test_winning) {
				game_over = true;
				test_winning = true;
				show_painted = true;
				shake_enable = true;
				use_grid = true;

				camera_position = glm::vec3(-27.5f, -40.0f, 120.0f);
				mCamera.mWorld.SetTranslate(camera_position);
				mCamera.mWorld.LookAt(glm::vec3(0.0f, -10.0f, 0.0f));
				build_painted_spheres();

				if (show_painted) {
					if (shake_enable)
						shake_time += dt;
					if (show_painted && shake_enable) {
						float w = glm::two_pi<float>() * shake_freq; // rad/s
						for (size_t i = 0; i < paint_nodes.size(); ++i) {
							float y_off = shake_amp * std::sin(w * shake_time + paint_phase[i]);
							paint_nodes[i].get_transform().SetTranslate(paint_anchors[i] + glm::vec3(0.0f, y_off, 0.0f));
						}
					}
					for (auto& n : paint_nodes)
						n.render(mCamera.GetWorldToClipMatrix());
				}



			}



			if (game_over && !test_winning) {
				// Center the small window
				ImGuiViewport* vp = ImGui::GetMainViewport();
				ImVec2 center = vp->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
				ImGui::SetNextWindowBgAlpha(0.75f); // dim a bit


				ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;


				ImGui::Begin("Game Over", nullptr, flags);
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "You lose!");

				if (ImGui::Button("Restart")) {
					// Reset player
					test_winning = false;
					player_position = glm::vec3(0.0f, 0.0f, mCamera.mWorld.GetTranslation().z - 10.0f);
					player_velocity = glm::vec3(0.0f);
					if (best_score < player_radius) best_score = player_radius;
					player_radius = player_mesh_radius; // your initial size

					// Respawn obstacles (same call you used at startup)
					spawn_spheres(300, 0.1f, 3.0f, 2471u);

					game_over = false; // back to playing
				}
				ImGui::End();
			}
		}
		ImGui::End();

		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glfwSwapBuffers(window);
	}
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
