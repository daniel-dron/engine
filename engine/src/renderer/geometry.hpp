#pragma once

#include <utility>
#include "resources/vertex_array.hpp"
#include "resources/buffer.hpp"
#include "resources/vertex_layout.hpp"

namespace geometry {

	struct Geometry {
		std::shared_ptr<GlBuffer> vbo;
		std::shared_ptr<VertexArray> vao;
	};

	std::shared_ptr<Geometry> get_cube();
}