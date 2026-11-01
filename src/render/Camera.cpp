#include "render/Camera.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 Camera::position() const {
    float yaw = glm::radians(yawDeg), pitch = glm::radians(pitchDeg);
    glm::vec3 dir(std::cos(pitch) * std::cos(yaw), std::cos(pitch) * std::sin(yaw), std::sin(pitch));
    return target + distance * dir;
}

glm::mat4 Camera::view() const {
    // En la vista en planta (pitch 90°) el vector "arriba" Z es paralelo a la
    // dirección de visión; usamos X como "arriba" de la pantalla en ese caso.
    glm::vec3 up = std::abs(pitchDeg) > 89.0f ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1);
    return glm::lookAt(position(), target, up);
}

glm::mat4 Camera::projection(float aspect) const {
    float nearP = distance * 0.01f, farP = distance * 100.0f;
    if (orthographic) {
        float h = distance * std::tan(glm::radians(fovDeg) * 0.5f);
        return glm::ortho(-h * aspect, h * aspect, -h, h, -farP, farP);
    }
    return glm::perspective(glm::radians(fovDeg), aspect, nearP, farP);
}

void Camera::orbit(float dx, float dy) {
    yawDeg   -= dx * 0.3f;
    pitchDeg  = std::clamp(pitchDeg + dy * 0.3f, -89.9f, 89.9f);
}

void Camera::pan(float dx, float dy, float viewportHeight) {
    // Desplazar el objetivo en el plano de la pantalla, proporcional a la distancia
    glm::mat4 v = view();
    glm::vec3 right(v[0][0], v[1][0], v[2][0]);
    glm::vec3 up(v[0][1], v[1][1], v[2][1]);
    float worldPerPixel = 2.0f * distance * std::tan(glm::radians(fovDeg) * 0.5f) / std::max(viewportHeight, 1.0f);
    target += (-dx * right + dy * up) * worldPerPixel;
}

void Camera::zoom(float steps) {
    distance = std::clamp(distance * std::pow(0.88f, steps), 50.0f, 50000.0f);
}

void Camera::viewFront() { yawDeg = 0.0f;  pitchDeg = 0.0f; }
void Camera::viewSide()  { yawDeg = 90.0f; pitchDeg = 0.0f; }
void Camera::viewTop()   { yawDeg = 0.0f;  pitchDeg = 90.0f; }
void Camera::viewIso()   { yawDeg = 35.0f; pitchDeg = 20.0f; }
