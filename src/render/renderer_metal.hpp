#pragma once

#include "render/renderer.hpp"

#include <memory>

std::unique_ptr<Renderer> createMetalRenderer();
