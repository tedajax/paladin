#pragma once

#include "types.h"

#include <vector>
#include <unordered_map>

struct SDL_Window;

namespace tdjx
{
    namespace gfx
    {
        struct Rect
        {
            int x0, y0, x1, y1;
        };

        struct Palette
        {
            std::vector<uint8> data;
            std::unordered_map<uint32, int> colorIndexMap;
            int size;
            int mask;
            int scalar;
        };

        // 1 byte per pixel, index into palette
        struct ByteImage
        {
            std::vector<uint8> data;
            int width;
            int height;
        };

        typedef int ImageHandle;
        const int kInvalidHandle = -1;

        void init_with_window(int width, int height, SDL_Window* window);
        void shutdown();

        ImageHandle load_image(const char* filename);
        void free_image(ImageHandle imageHandle);

        void clear(int color);
        void point(int x, int y, int color);
        void line(int x0, int y0, int x1, int y1, int color);
        void circle(int x0, int y0, int radius, int color);
        void circle_fill(int x0, int y0, int radius, int color);
        void rectangle(int x0, int y0, int x1, int y1, int color);
        void rectangle_fill(int x0, int y0, int x1, int y1, int color);
        void blit(ImageHandle imageHandle, int x0, int y0);

        void* get_context();
        uint8* get_pixels();

        namespace rect
        {
            inline int width(const Rect& self);
            inline int height(const Rect& self);

            inline bool contains_point(const Rect& self, int x, int y);
            inline bool intersect(const Rect& a, const Rect& b);

            bool clip_rect(const Rect& source, Rect& dest);
            bool clip_line(const Rect& r, int& x0, int& y0, int& x1, int& y1);
        }

        namespace palette
        {
            bool try_create_palette_from_file(const char* filename, Palette& out);
            const uint8* data(const Palette& self);
            int index_from_color(const Palette& self, uint8 r, uint8 g, uint8 b);
        }

        namespace byte_image
        {
            bool try_create_from_image_with_palette(uint8* data, int width, int height, int bpp, const Palette& palette, ByteImage& out);
        }
    }
}
