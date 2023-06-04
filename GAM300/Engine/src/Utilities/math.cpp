#include "math.h"

glm::vec3 lerp(glm::vec3 init, glm::vec3 end, float t) {
	return init * (1.0f - t) + end * t;
}

glm::vec2 lerp(glm::vec2 init, glm::vec2 end, float t) {
	return init * (1.0f - t) + end * t;
}

float lerp(float init, float end, float t) {
	return init * (1.0f - t) + end * t;
}