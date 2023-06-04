#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

constexpr float cEpsilon = 1e-6f;
constexpr float PI = 3.141592f;

glm::vec3 lerp(glm::vec3 init, glm::vec3 end, float t);
glm::vec2 lerp(glm::vec2 init, glm::vec2 end, float t);
float lerp(float init, float end, float t);