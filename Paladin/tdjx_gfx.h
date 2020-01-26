#pragma once

#include "types.h"
#include "tdjx_math.h"

#include <vector>
#include <unordered_map>

struct SDL_Window;

namespace tdjx
{
    namespace gfx
    {
        using ::tdjx::math::Rect;

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

        using Canvas = ByteImage;

        typedef int ImageHandle;
        const int kInvalidHandle = -1;

        void init_with_window(int width, int height, SDL_Window* window);
        void load_palette(const char* filename);
        void shutdown();

        ImageHandle load_image(const char* filename);
        void free_image(ImageHandle imageHandle);

        void set_canvas(Canvas& canvas);
        void set_canvas();
        void draw_canvas_to_screen(Canvas& canvas);

        void clear(int color);
        void point(int x, int y, int color);
        void line(int x0, int y0, int x1, int y1, int color);
        void circle(int x0, int y0, int radius, int color);
        void circle_fill(int x0, int y0, int radius, int color);
        void rectangle(int x0, int y0, int x1, int y1, int color);
        void rectangle(Rect<int> r, int color);
        void rectangle_fill(int x0, int y0, int x1, int y1, int color);
        void rectangle_fill(Rect<int> r, int color);
        void triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color);
        void blit(ImageHandle imageHandle, int x0, int y0);

        void flip();

        void* get_context();
        uint8* get_pixels();
        void query_screen_dimensions(int& width, int& height);
        void query_palette_size(int& size);

        namespace palette
        {
            bool try_create_palette_from_file(const char* filename, Palette& out);
            const uint8* data(const Palette& self);
            int index_from_color(const Palette& self, uint8 r, uint8 g, uint8 b);
        }

        namespace byte_image
        {
            bool try_create_from_image_with_palette(uint8* data, int width, int height, int bpp, const Palette& palette, ByteImage& out);
            bool get_image_rect(const ByteImage& image, Rect<int>& destination);
        }

        namespace canvas
        {
            uint8* pixel_xy(Canvas& canvas, int x, int y);
            Canvas create_blank_from_screen();
            Canvas create_copy_from_screen();
        }
    }
}
