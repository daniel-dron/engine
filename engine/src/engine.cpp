#include "engine.hpp"

#include "defines.hpp"
#include <renderer/resources/resources.hpp>
#include <Windows.h>
#include <iostream>
#include "glad/glad.h"
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer/resources/gl_errors.hpp"
#include "renderer/mesh.hpp"
#include "renderer/model.hpp"
#include "renderer/resources/buffer.hpp"
#include <utils.hpp>
#include "renderer/geometry.hpp"

std::unique_ptr<Engine> g_engine;

std::string game_logic::get_current_dll_path() const {
	return (ResourceState::get()->_workingDirectory / std::format("game{}.dll", current_dll_id)).string();
}

void game_logic::load_game(const std::string& dll_name) {
	// check if dll exists
	auto dll_path = ResourceState::get()->_workingDirectory / "build" / "testbed" / dll_name;
	KDEBUG("dll_path: {}", dll_path.string());
	assert(std::filesystem::exists(dll_path) && "DLL does not exist");

	// unload the current dll
	if (this->_dll != nullptr) {
		KDEBUG("Unloading current dll");
		this->on_shutdown();
		FreeLibrary((HINSTANCE)this->_dll);
	}

	// delete dll
	std::filesystem::path current_dll_path = this->get_current_dll_path();
	if (std::filesystem::exists(current_dll_path)) {
		KDEBUG("Deleting current dll");
		std::filesystem::remove(current_dll_path);
	}

	// create a copy of the dll with the name of the current dll (incremental)
	current_dll_path = ResourceState::get()->_workingDirectory / std::format("game{}.dll", ++current_dll_id);
	std::filesystem::copy(dll_path, current_dll_path, std::filesystem::copy_options::overwrite_existing);
	KDEBUG("Copied dll to: {}", current_dll_path.string());

	// load the dll
	HINSTANCE _dll = LoadLibrary(current_dll_path.string().c_str());
	KDEBUG("Loaded dll: {}", current_dll_path.string());
	this->_dll = _dll;

	auto default_on_init = []() -> b8 {
		std::cout << "default_on_init()" << std::endl;
		return true;
		};
	auto on_init = GetProcAddress(_dll, "on_init");
	this->on_init = on_init != nullptr ? reinterpret_cast<b8(*)(void)>(on_init) : default_on_init;

	auto default_on_update = []() -> b8 {
		std::cout << "default_on_update()" << std::endl;
		return true;
		};
	auto on_update = GetProcAddress(_dll, "on_update");
	this->on_update = on_update != nullptr ? reinterpret_cast<b8(*)(void)>(on_update) : default_on_update;

	auto default_on_render = []() -> b8 {
		std::cout << "default_on_render()" << std::endl;
		return true;
		};
	auto on_render = GetProcAddress(_dll, "on_render");
	this->on_render = on_render != nullptr ? reinterpret_cast<b8(*)(void)>(on_render) : default_on_render;

	auto default_on_resize = [](u32 width, u32 height) {
		std::cout << "default_on_resize()" << std::endl;
		};
	auto on_resize = GetProcAddress(_dll, "on_resize");
	this->on_resize = on_resize != nullptr ? reinterpret_cast<void(*)(u32, u32)>(on_resize) : default_on_resize;

	auto default_on_shutdown = []() {
		std::cout << "default_on_shutdown()" << std::endl;
		};
	auto on_shutdown = GetProcAddress(_dll, "on_shutdown");
	this->on_shutdown = on_shutdown != nullptr ? reinterpret_cast<void(*)(void)>(on_shutdown) : default_on_shutdown;
}

std::unique_ptr<Engine> Engine::create(std::unique_ptr<app_desc> desc)
{
	auto _app = std::make_unique<Engine>();
	_app->_desc = std::move(desc);
	return _app;
}

void Engine::add_logic(const std::string& dll_name)
{
	//_logic = std::make_unique<game_logic>();
	//_logic->load_game("testbedd.dll");
}

b8 Engine::init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if GRAPHICS_DEBUG	
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwSwapInterval(1);

	if (_desc->fullscreen) {
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		_window = glfwCreateWindow(mode->width, mode->height, _desc->window_name.c_str(), glfwGetPrimaryMonitor(), nullptr);
		if (_window == nullptr) {
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			throw std::exception();
		}

		glfwMakeContextCurrent(_window);
		glfwSetWindowMonitor(_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);

		_desc->width = mode->width;
		_desc->height = mode->height;
	}
	else {
		_window = glfwCreateWindow(_desc->width, _desc->height, _desc->window_name.c_str(), nullptr, nullptr);
		if (!_window) {
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			throw std::exception();
		}

		glfwMakeContextCurrent(_window);
	}

	// set callbacks
	glfwSetWindowSizeCallback(_window, _window_size_callback);
	glfwSetCursorPosCallback(_window, _cursor_callback);
	glfwSetMouseButtonCallback(_window, _mouse_callback);
	glfwSetKeyCallback(_window, _key_callback);
	glfwSetDropCallback(_window, _drop_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		throw std::exception();
	}

#if GRAPHICS_DEBUG	
	int flags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		std::cout << "Initialized debug layer" << std::endl;
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// create renderer
	m_renderer = Renderer::create();
	m_renderer->initialize();

	// create camera
	m_camera = Camera::create((f32)_desc->width / (f32)_desc->height, 45.0f, 0.01f, 10000.0f);
	m_camera->set_position(glm::vec3(0.0f, 1.0f, 3.0f));

	// create screen framebuffer
	{
		TextureSpecification tspec = {};
		tspec.internalFormat = GL_RGBA16F;
		tspec.width = _desc->width;
		tspec.height = _desc->height;
		//tspec.slot = 0;
		auto texture = Texture::create(tspec);

		tspec.slot = 1;
		auto bloomTexture = Texture::create(tspec);

		FramebufferSpecification spec = {};
		spec.clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		spec.depth_stencil = true;
		spec.width = _desc->width;
		spec.height = _desc->height;
		spec.color_attachements = { texture, bloomTexture };
		m_screen = Framebuffer::create(spec);
	}

	// model
	auto model = Model::create("floor");
	model->get_root()->m_transform = utils::create_transform(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.01f)) * model->get_root()->m_transform;

	auto sculpture = Model::create("damaged_helmet");
	sculpture->get_root()->m_transform = utils::create_transform(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f)) * sculpture->get_root()->m_transform;

	m_models.push_back(model);
	m_models.push_back(sculpture);

	// opengl settings
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//return _logic->on_init();
	return true;
}

#include <chrono>

void Engine::run()
{
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;

	init();

	while (!glfwWindowShouldClose(_window))
	{

		auto t1 = high_resolution_clock::now();

		glfwGetCurrentContext();

		update();

		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, _desc->width, _desc->height);
			glClearColor(0.4f, 0.0f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		m_screen->begin_pass();
		render();

		// back to rendering to screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, _desc->width, _desc->height);

		// render quad to screen
		m_renderer->render_screen_framebuffer(m_screen, _desc->width, _desc->height);

		// end frame
		{
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			auto backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup);

			glfwSwapBuffers(glfwGetCurrentContext());
		}

		// clear just-pressed keys (do it before poll new events to avoid clearing keys that were pressed in the same frame)
		this->clear();
		glfwPollEvents();

		auto t2 = high_resolution_clock::now();

		duration<double, std::milli> ms_double = t2 - t1;
		_frame_time = ms_double.count();
	}

	//_logic->on_shutdown();
}

b8 Engine::update()
{
	// calculate delta time
	const auto now = this->now();
	_delta = float(now - _last_frame) / NS_PER_SECOND;
	_last_frame = now;

	// update camera
	if (m_mouse_locked) {
		Camera::Direction direction = Camera::Direction::NONE;
		if (keys[GLFW_KEY_W].down)
			m_camera->move(Camera::Direction::FORWARD, (f32)_delta);
		if (keys[GLFW_KEY_S].down)
			m_camera->move(Camera::Direction::BACKWARD, (f32)_delta);
		if (keys[GLFW_KEY_A].down)
			m_camera->move(Camera::Direction::LEFT, (f32)_delta);
		if (keys[GLFW_KEY_D].down)
			m_camera->move(Camera::Direction::RIGHT, (f32)_delta);
		if (keys[GLFW_KEY_SPACE].down)
			m_camera->move(Camera::Direction::UP, (f32)_delta);
		if (keys[GLFW_KEY_LEFT_SHIFT].down)
			m_camera->move(Camera::Direction::DOWN, (f32)_delta);

		m_camera->rotate((f32)mouse_delta.x, (f32)mouse_delta.y, (f32)_delta);
	}

	// camera lock and unlock
	if (keys[GLFW_KEY_LEFT_CONTROL].pressed) {
		m_mouse_locked = !m_mouse_locked;
		if (m_mouse_locked) glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	// update matrices
	m_renderer->update_view(
		m_camera->get_view_matrix(),
		m_camera->get_projection_matrix(),
		m_camera->get_position()
	);

	// call game logic update
	//_logic->on_update();

	if (keys[GLFW_KEY_ESCAPE].down) {
		glfwSetWindowShouldClose(_window, true);
	}

	return true;
}

b8 Engine::render()
{
	// overlay
	if (false)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
		const float PAD = 10.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = (work_pos.x + PAD);
		window_pos.y = (work_pos.y + PAD);
		window_pos_pivot.x = 0.0f;
		window_pos_pivot.y = 0.0f;
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowViewport(viewport->ID);
		flags |= ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowBgAlpha(0.35f);
		ImGui::Begin("Controls", 0, flags);
		{
			ImGui::Text("Move: W/A/S/D");
			ImGui::Text("Up/Down: SPACE/SHIFT");
			ImGui::Text("Lock/Unlock Mouse: CTRL");
			ImGui::Text("Close: ESCAPE");
			ImGui::Separator();
			ImGui::Text("Drag & Drop HDR files");
		}
		ImGui::End();
	}

	//_logic->on_render();

	// shadow map pass
	auto sm_pass = m_renderer->get_shadow_map_pass();
	sm_pass->render_debug_menu();
	sm_pass->start();
	{
		for (const auto& model : m_models) 
			sm_pass->render(model, glm::mat4(1.0f));
	}
	sm_pass->stop();

	glViewport(0, 0, _desc->width, _desc->height);
	// geometry pass
	auto& gbuffer = m_renderer->get_gbuffer();
	gbuffer->start();
	{
		for (const auto& model : m_models) 
			gbuffer->render(model, glm::mat4(1.0f));
	}
	gbuffer->stop();

	// lighting pass
	auto lighting_pass = m_renderer->get_light_pass();
	lighting_pass->start();
	{
		m_renderer->m_screen_vao->bind();
		glDrawElements(GL_TRIANGLES, m_renderer->m_screen_ibo->get_count(), GL_UNSIGNED_INT, nullptr);
	}
	lighting_pass->stop();

	// bitblt depth buffer to screen framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer->m_framebuffer->get_resource_id());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_screen->get_resource_id());
	glBlitFramebuffer(0, 0, _desc->width, _desc->height, 0, 0, _desc->width, _desc->height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, lighting_pass->m_framebuffer->get_resource_id());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_screen->get_resource_id());
	glBlitFramebuffer(0, 0, _desc->width, _desc->height, 0, 0, _desc->width, _desc->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// draw skybox
	auto cube = geometry::get_cube();
	cube->vao->bind();
	auto skybox_shader = m_renderer->get_shader("cubemap");
	skybox_shader->bind();
	m_renderer->m_ibl->bind_env(0);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	cube->vao->unbind();

	ImGui::Begin("OpenGL PBR + IBL (droon)");
	{
		if (ImGui::CollapsingHeader("FXAA")) {
			m_renderer->render_debug_menu();
		}

		if (ImGui::CollapsingHeader("Camera")) {
			m_camera->render_debug_menu();
		}

		if (ImGui::CollapsingHeader("Debug")) {
			ImGui::Text("FPS: %.1f", 1.0f / _delta);
			ImGui::Text("Frametime: %0.01f", _frame_time);
			ImGui::Text("Triangles: %ld", m_renderer->get_rendered_triangles());
			m_renderer->reset_rendered_triangles();

			ImGui::Checkbox("Deferred", &m_render_deferred);

			ImGui::Text("Mouse Pos: %.1f, %.1f", mouse_pos.x, mouse_pos.y);
			if (ImGui::Button("Reload Shaders")) {
				m_renderer->invalidate_shaders();
			}
		}

		if (ImGui::CollapsingHeader("IBL")) {
			static char hdr_name[256] = "";
			ImGui::InputText("HDR", hdr_name, IM_ARRAYSIZE(hdr_name));
			if (ImGui::Button("Reload HDR")) {
				auto path = ResourceState::get()->getTexturePath(hdr_name);
				m_renderer->m_ibl = IBL::create(path);
			}

			if (m_renderer->m_ibl) {
				utils::imgui_render_hoverable_image(m_renderer->m_ibl->get_hdri(), ImVec2(200.0f, 200.0f));
				utils::imgui_render_hoverable_image(m_renderer->m_ibl->get_brdf(), ImVec2(200.0f, 200.0f));
			}
		}

		//if (ImGui::CollapsingHeader("Rendering")) {
		//	m_model->render_menu_debug();
		//}
	}
	ImGui::End();

	return true;
}

u64 Engine::now()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch())
		.count();
}

f64 Engine::get_delta() const
{
	return _delta;
}

const std::unique_ptr<Renderer>& Engine::get_renderer() const
{
	return m_renderer;
}

void Engine::_window_size_callback(GLFWwindow* window, i32 width, i32 height)
{
	g_engine->_desc->width = width;
	g_engine->_desc->height = height;
	//g_engine->_logic->on_resize(width, height);
}

void Engine::_cursor_callback(GLFWwindow* window, f64 xpos, f64 ypos)
{
	// initialize mouse position
	// this is done only once to avoid the mouse jumping to the center of the screen
	std::call_once(g_engine->m_mouse_init, [&]() {
		g_engine->mouse_pos.x = xpos;
		g_engine->mouse_pos.y = ypos;
		});

	g_engine->mouse_delta.x = xpos - g_engine->mouse_pos.x;
	g_engine->mouse_delta.y = g_engine->mouse_pos.y - ypos;

	g_engine->mouse_pos.x = xpos;
	g_engine->mouse_pos.y = ypos;
}

void Engine::_mouse_callback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
	if (action == GLFW_PRESS) {
		g_engine->mouse_keys[button].down = true;
		g_engine->mouse_keys[button].pressed = true;
	}
	else if (action == GLFW_RELEASE) {
		g_engine->mouse_keys[button].down = false;
		g_engine->mouse_keys[button].released = true;
	}
}

void Engine::_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
	if (action == GLFW_PRESS) {
		g_engine->keys[key].down = true;
		g_engine->keys[key].pressed = true;
	}
	else if (action == GLFW_RELEASE) {
		g_engine->keys[key].down = false;
		g_engine->keys[key].released = true;
	}
}

void Engine::_drop_callback(GLFWwindow* window, int count, const char** paths) {
	for (u32 i = 0; i < count; i++) {
		auto path = ResourceState::get()->getTexturePath(paths[i]);
		g_engine->m_renderer->m_ibl->reload_ibl(path.string());
	}
}

void Engine::clear()
{
	// clear keyboard keys
	for (auto& key : keys) {
		key.pressed = false;
		key.released = false;
	}

	// clear mouse keys
	for (auto& key : mouse_keys) {
		key.pressed = false;
		key.released = false;
	}

	// clear mouse delta
	mouse_delta.x = 0;
	mouse_delta.y = 0;
}