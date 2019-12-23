#include "camera.h"
#include <cstdio>

void camera::move(glm::vec3 movement) {
    target += glm::vec3(movement.x, 0, movement.z);
}

void camera::move_relative(glm::vec3 movement) {
    glm::vec4 tl = glm::rotate(glm::mat4(1.f), glm::radians(180 - theta), glm::vec3(0, 1, 0)) * glm::vec4(-movement, 1);
    glm::vec3 mv = glm::vec3(tl.x, 0, tl.z);

    target += mv;
}

void camera::orbit(float32 pitch, float32 yaw) {
    phi += pitch;
    theta += yaw;

    phi = math::clamp(phi, 0.1f, 89.9f);
}

glm::mat4x4 camera::view() const {
    //glm::mat4 v = glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(1, 0, 0));
    //v = glm::rotate(v, glm::radians(0.f), glm::vec3(0, 1, 0));
    //v = glm::translate(v, position + glm::vec3(math::sin(phi) * math::cos(theta) * rho, math::cos(phi) * rho, math::sin(phi) * math::sin(theta) * rho));
    //v = glm::inverse(v);


    return glm::lookAt(position(), target, glm::vec3(0, 1, 0));;
}

glm::vec3 camera::position() const {
    return target + glm::vec3(math::sin(phi) * math::cos(theta + 90.f) * rho, math::cos(phi) * rho, math::sin(phi) * math::sin(theta + 90.f) * rho);
}

glm::mat4x4 camera::projection() const {
    return glm::perspective(glm::radians(fov),  aspect, nearZ, farZ);
}

void camera::get_screen_ray(const vec2& screenPoint, glm::vec3& outOrigin, glm::vec3& outDir) {
    glm::mat4 invView = glm::inverse(projection() * view());

    float32 thfov = math::tan(fov / 2);
    float32 dx = thfov * screenPoint.x * aspect;
    float32 dy = thfov * screenPoint.y;
    glm::vec4 p1(dx * nearZ, dy * nearZ, nearZ, 1.f);
    glm::vec4 p2(dx * farZ, dy * farZ, farZ, 1.f);

    p1 = invView * p1;
    p2 = invView * p2;
    outOrigin = p1;
    outDir = glm::normalize(p2 - p1);
}