#pragma once

#include <OpenGLRendering/OGLRenderer.h>

#include "RenderBackend.h"

namespace NCL::CSC8503::Render {
	class OpenGlBackend : public RenderBackend, public OGLRenderer {

	};
}
