#include "box2dSdlDebugDraw.h"
#include <SDL2/SDL.h>
#include "algebra.h"

box2dSdlDebugDraw::box2dSdlDebugDraw(SDL_Renderer* renderer)
    : renderer(renderer) { }

/// Draw a closed polygon provided in CCW order.
void box2dSdlDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
    SDL_SetRenderDrawColor(renderer, math::floor_int(color.r * 255.f), math::floor_int(color.g * 255.f), math::floor_int(color.b * 255.f), math::floor_int(color.a * 255.f));

    for (int i = 0; i < vertexCount; ++i) {
        int j = (i + 1) % vertexCount;
        vec2 a = vertices[i];
        vec2 b = vertices[j];
        SDL_RenderDrawLine(renderer, (int)a.x, (int)a.y, (int)b.x, (int)b.y);
    }
}

/// Draw a solid closed polygon provided in CCW order.
void box2dSdlDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
    SDL_SetRenderDrawColor(renderer, math::floor_int(color.r * 255.f), math::floor_int(color.g * 255.f), math::floor_int(color.b * 255.f), math::floor_int(color.a * 255.f));

    for (int i = 0; i < vertexCount; ++i) {
        int j = (i + 1) % vertexCount;
        vec2 a = vertices[i];
        vec2 b = vertices[j];
        SDL_RenderDrawLine(renderer, (int)a.x, (int)a.y, (int)b.x, (int)b.y);
    }
}

/// Draw a circle.
void box2dSdlDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
    SDL_SetRenderDrawColor(renderer, math::floor_int(color.r * 255.f), math::floor_int(color.g * 255.f), math::floor_int(color.b * 255.f), math::floor_int(color.a * 255.f));

    const int segments = 21;
    const float32 delta = 360.f / segments;

    for (int i = 0; i <= segments; ++i) {
        float32 theta0 = delta * i;
        float32 theta1 = delta * (i + 1);

        int x1 = (int)(center.x + math::cos(theta0) * radius);
        int y1 = (int)(center.y + math::cos(theta0) * radius);
        int x2 = (int)(center.x + math::cos(theta1) * radius);
        int y2 = (int)(center.y + math::cos(theta1) * radius);

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

/// Draw a solid circle.
void box2dSdlDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
    SDL_SetRenderDrawColor(renderer, math::floor_int(color.r * 255.f), math::floor_int(color.g * 255.f), math::floor_int(color.b * 255.f), math::floor_int(color.a * 255.f));

    const int segments = 21;
    const float32 delta = 360.f / segments;

    for (int i = 0; i <= segments; ++i) {
        float32 theta0 = delta * i;
        float32 theta1 = delta * (i + 1);

        int x1 = (int)(center.x + math::cos(theta0) * radius);
        int y1 = (int)(center.y + math::cos(theta0) * radius);
        int x2 = (int)(center.x + math::cos(theta1) * radius);
        int y2 = (int)(center.y + math::cos(theta1) * radius);

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

/// Draw a line segment.
void box2dSdlDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
    SDL_SetRenderDrawColor(renderer, math::floor_int(color.r * 255.f), math::floor_int(color.g * 255.f), math::floor_int(color.b * 255.f), math::floor_int(color.a * 255.f));

    SDL_RenderDrawLine(renderer, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void box2dSdlDebugDraw::DrawTransform(const b2Transform& xf) {

}

/// Draw a point.
void box2dSdlDebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) {
    SDL_SetRenderDrawColor(renderer, math::floor_int(color.r * 255.f), math::floor_int(color.g * 255.f), math::floor_int(color.b * 255.f), math::floor_int(color.a * 255.f));
    SDL_RenderDrawPoint(renderer, (int)p.x, (int)p.y);
}