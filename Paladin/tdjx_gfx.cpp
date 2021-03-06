#include "tdjx_gfx.h"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "util.h"
#include "renderer.h"

using namespace tdjx::math;

namespace tdjx
{
    namespace gfx
    {
        struct image_loader
        {
            int w, h, bpp;
            uint8* data;

            image_loader(const char* filename, int channels)
            {
                data = stbi_load(filename, &w, &h, &bpp, channels);
                bpp = channels;
            }

            ~image_loader()
            {
                stbi_image_free(data);
                w = 0; h = 0; bpp = 0;
            }

            inline bool success() const { return data != nullptr; }
        };

        void fixup_palette_image(const char* sourceFilename, const char* destFilename)
        {
            int w, h, n;
            uint8* data = stbi_load(sourceFilename, &w, &h, &n, 4);

            if (data)
            {
                stbi_write_png(destFilename, w * h, 1, 4, data, w * h * 4);
            }

            stbi_image_free(data);
        }

        // lol
        const int kMaxLoadedImages = 32;

        struct
        {
            Canvas screenCanvas;
            Canvas& activeCanvas = screenCanvas;
            Rect<int> clipArea = Rect<int>{ 0, 0, 0, 0 };
            Palette palette;
            ByteImage imageBank[kMaxLoadedImages];
            int nextImageId = 0;
        } g_gfx;

        uint8* pixel_xy(Canvas& canvas, int x, int y)
        {
            return canvas.data.data() + x + y * canvas.width;
        }

        uint8* pixel_xy(int x, int y)
        {
            return pixel_xy(g_gfx.activeCanvas, x, y);
        }

        bool gfx_clip_rect(Rect<int>& r)
        {
            return rect::clip_rect(g_gfx.clipArea, r);
        }

        bool gfx_clip_line(int& x0, int& y0, int& x1, int& y1)
        {
            return rect::clip_line(g_gfx.clipArea, x0, y0, x1, y1);
        }

        bool gfx_point_visible(int x0, int y0)
        {
            return rect::contains_point(g_gfx.clipArea, x0, y0);
        }

        void scanline(int y, int x0, int x1, int color)
        {
            std::fill(pixel_xy(x0, y), pixel_xy(x1, y) + 1, color);
        }

        void scanline_safe(int y, int x0, int x1, int color)
        {
            if (x1 < x0)
            {
                std::swap(x0, x1);
            }
            int y0 = y, y1 = y;
            if (!rect::clip_line(g_gfx.clipArea, x0, y0, x1, y1))
            {
                return;
            }
            scanline(y0, x0, x1, color);
        }

        void init_with_window(int width, int height, SDL_Window* window)
        {
            tdjx::render::init(window, width, height);

            g_gfx.screenCanvas = {};
            g_gfx.screenCanvas.data.resize(width * height);
            std::fill(g_gfx.screenCanvas.data.begin(), g_gfx.screenCanvas.data.end(), 0);
            g_gfx.screenCanvas.width = width;
            g_gfx.screenCanvas.height = height;

            set_canvas();

            g_gfx.clipArea = { 0, 0, width - 1, height - 1 };

            load_palette("assets/palettes/arne32.png");
        }

        void load_palette(const char* filename)
        {
            if (palette::try_create_palette_from_file(filename, g_gfx.palette))
            {
                tdjx::render::set_palette(palette::data(g_gfx.palette), g_gfx.palette.size);
            }
            else
            {
                printf("Failed to create palette from '%s'.\n", filename);
            }
        }

        void shutdown()
        {
            tdjx::render::shutdown();
        }

        ImageHandle load_image(const char* filename)
        {
            // todo: free list thing so spots can open up instead of being sequential

            image_loader loader(filename, 4);
            if (loader.success())
            {
                ByteImage& image = g_gfx.imageBank[g_gfx.nextImageId];
                if (byte_image::try_create_from_image_with_palette(loader.data, loader.w, loader.h, loader.bpp, g_gfx.palette, image))
                {
                    ImageHandle ret = g_gfx.nextImageId;
                    g_gfx.nextImageId++;
                    return ret;
                }
            }
            return kInvalidHandle;
        }

        void free_image(ImageHandle imageHandle)
        {
            // one day
        }

        void mask_color(int& color)
        {
            color = (color & g_gfx.palette.mask);
        }

        void set_canvas(Canvas& canvas)
        {
            g_gfx.activeCanvas = canvas;
        }

        void set_canvas()
        {
            g_gfx.activeCanvas = g_gfx.screenCanvas;
        }

        void draw_canvas_to_screen(Canvas& canvas)
        {
            std::copy(canvas.data.begin(), canvas.data.end(), g_gfx.screenCanvas.data.begin());
        }

        void clear(int color)
        {
            mask_color(color);

            for (int y = g_gfx.clipArea.y0; y <= g_gfx.clipArea.y1; ++y)
            {
                scanline(y, g_gfx.clipArea.x0, g_gfx.clipArea.x1, color);
            }
        }

        void point(int x, int y, int color)
        {
            if (rect::contains_point(g_gfx.clipArea, x, y))
            {
                mask_color(color);
                *pixel_xy(x, y) = color;
            }
        }

        void line(int x0, int y0, int x1, int y1, int color)
        {
            if (!rect::clip_line(g_gfx.clipArea, x0, y0, x1, y1))
            {
                return;
            }

            int dx = std::abs(x1 - x0);
            int dy = std::abs(y1 - y0);

            mask_color(color);

            if (dy == 0)
            {
                if (x1 < x0)
                {
                    std::swap(x0, x1);
                }
                scanline(y0, x0, x1, color);
                return;
            }
            else if (dx == 0)
            {
                if (y1 < y0)
                {
                    std::swap(y0, y1);
                }

                for (int y = y0; y <= y1; ++y)
                {
                    *pixel_xy(x0, y) = color;
                }
                return;
            }

            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;
            int err = (dx > dy ? dx : -dy) / 2, e2;

            while (true)
            {
                *pixel_xy(x0, y0) = color;
                if (x0 == x1 && y0 == y1) break;
                e2 = err;
                if (e2 > -dx) { err -= dy; x0 += sx; }
                if (e2 < dy) { err += dx; y0 += sy; }
            }
        }

        void line(const Rect<int>& segment, int color)
        {
            line(segment.x0, segment.y0, segment.x1, segment.y1, color);
        }

        void circle(int x0, int y0, int radius, int color)
        {
            Rect<int> bounds = Rect<int>{ x0 - radius, y0 - radius, x0 + radius, y0 + radius };
            if (!rect::clip_rect(g_gfx.clipArea, bounds))
            {
                return;
            }

            mask_color(color);

            auto putpixel = [](int x, int y, uint8 c)
            {
                if (gfx_point_visible(x, y))
                {
                    *pixel_xy(x, y) = c;
                }
            };

            {
                int x = radius;
                int y = 0;
                int err = 0;

                while (x >= y)
                {
                    putpixel(x0 + x, y0 + y, color);
                    putpixel(x0 + y, y0 + x, color);
                    putpixel(x0 + x, y0 - y, color);
                    putpixel(x0 + y, y0 - x, color);
                    putpixel(x0 - x, y0 + y, color);
                    putpixel(x0 - y, y0 + x, color);
                    putpixel(x0 - x, y0 - y, color);
                    putpixel(x0 - y, y0 - x, color);

                    if (err <= 0)
                    {
                        ++y;
                        err += 2 * y + 1;
                    }

                    if (err > 0)
                    {
                        --x;
                        err -= 2 * x + 1;
                    }
                }
            }
        }

        void circle_fill(int x0, int y0, int radius, int color)
        {
            Rect<int> bounds = Rect<int>{ x0 - radius, y0 - radius, x0 + radius, y0 + radius };
            if (!rect::clip_rect(g_gfx.clipArea, bounds))
            {
                return;
            }

            mask_color(color);

            auto clipped_scanline = [=](int y, int x0, int x1, int color)
            {
                if (y >= g_gfx.clipArea.y0 && y <= g_gfx.clipArea.y1)
                {
                    scanline(y, std::max(g_gfx.clipArea.x0, x0), std::min(g_gfx.clipArea.x1, x1), color);
                }
            };

            {
                int x = radius;
                int y = 0;
                int err = 0;

                while (x >= y)
                {
                    clipped_scanline(y0 - y, x0 - x, x0 + x, color);
                    clipped_scanline(y0 - x, x0 - y, x0 + y, color);
                    clipped_scanline(y0 + y, x0 - x, x0 + x, color);
                    clipped_scanline(y0 + x, x0 - y, x0 + y, color);

                    if (err <= 0)
                    {
                        ++y;
                        err += 2 * y + 1;
                    }

                    if (err > 0)
                    {
                        --x;
                        err -= 2 * x + 1;
                    }
                }
            }
        }

        void rectangle(Rect<int> r, int color)
        {
            rectangle(r.x0, r.y0, r.x1, r.y1, color);
        }

        void rectangle(int x0, int y0, int x1, int y1, int color)
        {
            Rect<int> segmentT = { x0, y0, x1, y0 };
            Rect<int> segmentB = { x0, y1, x1, y1 };
            Rect<int> segmentL = { x0, y0, x0, y1 };
            Rect<int> segmentR = { x1, y0, x1, y1 };

            if (rect::clip_line(g_gfx.clipArea, segmentT))
            {
                line(segmentT, color);
            }

            if (rect::clip_line(g_gfx.clipArea, segmentB))
            {
                line(segmentB, color);
            }

            if (rect::clip_line(g_gfx.clipArea, segmentL))
            {
                line(segmentL, color);
            }

            if (rect::clip_line(g_gfx.clipArea, segmentR))
            {
                line(segmentR, color);
            }
        }

        void rectangle_fill(Rect<int> r, int color)
        {
            if (!rect::clip_rect(g_gfx.clipArea, r))
            {
                return;
            }

            mask_color(color);

            for (int y = r.y0; y <= r.y1; ++y)
            {
                scanline(y, r.x0, r.x1, color);
            }
        }

        void rectangle_fill(int x0, int y0, int x1, int y1, int color)
        {
            rectangle_fill(Rect<int>{ x0, y0, x1, y1 }, color);
        }

        void triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color)
        {
            mask_color(color);

            struct Point { int x, y; };

            Point points[3] =
            {
                { x0, y0 },
                { x1, y1 },
                { x2, y2 }
            };

            [&points]()
            {
                // insertion sort but unrolled for 3 items
                if (points[0].y > points[1].y)
                {
                    std::swap(points[1], points[0]);
                }
                if (points[1].y > points[2].y)
                {
                    std::swap(points[2], points[1]);
                }
                if (points[0].y > points[1].y)
                {
                    std::swap(points[1], points[0]);
                }
            }();

            // TODO: currently assuming inputs result in valid triangles, should maybe not do that

            auto flatTop = [color](Point* points)
            {
                int botx = points[2].x;
                int boty = points[2].y;
                int y = points[0].y;
                int leftx = points[0].x;
                int rightx = points[1].x;

                float32 mleft = static_cast<float32>(botx - leftx) / static_cast<float32>(boty - y);
                float32 mright = static_cast<float32>(botx - rightx) / static_cast<float32>(boty - y);

                float32 left = static_cast<float32>(botx);
                float32 right = static_cast<float32>(botx);

                for (int i = boty; i > y; --i)
                {
                    scanline_safe(i, static_cast<int>(left), static_cast<int>(right), color);
                    left -= mleft;
                    right -= mright;
                }
            };

            auto flatBottom = [color](Point* points)
            {
                int topx = points[0].x;
                int topy = points[0].y;
                int y = points[1].y;
                int leftx = points[1].x;
                int rightx = points[2].x;

                float32 mleft = static_cast<float32>(leftx - topx) / static_cast<float32>(y - topy);
                float32 mright = static_cast<float32>(rightx - topx) / static_cast<float32>(y - topy);

                float32 left = static_cast<float32>(topx);
                float32 right = static_cast<float32>(topx);

                for (int i = topy; i <= y; ++i)
                {
                    scanline_safe(i, static_cast<int>(left), static_cast<int>(right), color);
                    left += mleft;
                    right += mright;
                }
            };

            // flat top
            if (points[0].y == points[1].y)
            {
                flatTop(points);
            }
            // flat bottom
            else if (points[1].y == points[2].y)
            {
                flatBottom(points);
            }
            // both
            else
            {
                float32 dy1 = static_cast<float32>(points[1].y - points[0].y);
                float32 dy = static_cast<float32>(points[2].y - points[0].y);
                float32 dx = static_cast<float32>(points[2].x - points[0].x);
                
                int x4 = points[0].x + static_cast<int>(dy1 / dy * dx);
                int y4 = points[1].y;

                Point top[3] = {
                    points[0],
                    points[1],
                    { x4, y4 }
                };

                Point bottom[3] = {
                    points[1],
                    { x4, y4 },
                    points[2]
                };

                flatBottom(top);
                flatTop(bottom);

                scanline_safe(y4, points[1].x, x4, 8);
            }
        }

        void blit(ImageHandle imageHandle, int x0, int y0)
        {
            const ByteImage image = g_gfx.imageBank[imageHandle];

            Rect<int> r = { x0, y0, x0 + image.width - 1, y0 + image.height - 1 };
            if (!rect::clip_rect(g_gfx.clipArea, r))
            {
                return;
            }

            for (int y = 0; y < image.height; ++y)
            {
                for (int x = 0; x < image.width; ++x)
                {
                    uint8 i = image.data[y * image.width + x];
                    *pixel_xy(x + x0, y + y0) = i;
                }
            }
        }

        void flip()
        {
            tdjx::render::set_intensity(get_pixels());
        }

        void* get_context()
        {
            return &g_gfx;
        }

        uint8* get_pixels()
        {
            return g_gfx.screenCanvas.data.data();
        }

        void query_screen_dimensions(int& width, int& height)
        {
            width = g_gfx.screenCanvas.width;
            height = g_gfx.screenCanvas.height;
        }

        void query_palette_size(int& size)
        {
            size = g_gfx.palette.size;
        }

        namespace palette
        {
            bool try_create_palette_from_file(const char* filename, Palette& out)
            {
                out.data.clear();
                out.colorIndexMap.clear();

                const int channels = 4;

                // Setup palette
                image_loader loader(filename, channels);

                if (loader.success())
                {
                    uint32 pixelCount = loader.w * loader.h;
                    uint32 paletteSize = tdjx::util::next_or_equal_pow2(pixelCount);

                    out.size = static_cast<int>(paletteSize);
                    out.mask = out.size - 1;
                    out.scalar = 256 / out.size;

                    int paletteBytes = paletteSize * channels;

                    out.data.resize(paletteBytes);
                    std::memcpy(out.data.data(), loader.data, paletteBytes);

                    for (int i = 0; i < paletteBytes; i += channels)
                    {
                        uint8* color = out.data.data() + i;
                        int h = (static_cast<uint32>(color[0]) << 16) + (static_cast<uint32>(color[1]) << 8) + static_cast<uint32>(color[2]);
                        out.colorIndexMap.insert_or_assign(h, i / channels);
                    }

                    return true;
                }

                return false;
            }

            inline const uint8* data(const Palette& self)
            {
                return self.data.data();
            }

            int index_from_color(const Palette& self, uint8 r, uint8 g, uint8 b)
            {
                int h = (static_cast<uint32>(r) << 16) + (static_cast<uint32>(g) << 8) + static_cast<uint32>(b);
                auto search = self.colorIndexMap.find(h);
                if (search != self.colorIndexMap.end())
                {
                    return search->second;
                }
                return -1;
            }
        }

        namespace byte_image
        {
            bool try_create_from_image_with_palette(uint8* data, int width, int height, int bpp, const Palette& palette, ByteImage& out)
            {
                out.data.clear();

                out.width = width;
                out.height = height;

                int size = width * height;

                out.data.resize(size);

                for (int i = 0; i < size; ++i)
                {
                    uint8* pixel = &data[i * bpp];

                    uint8 v = 0;
                    switch (bpp)
                    {
                        // 1 or 2 channel images have an intensity and optional alpha so it's pretty easy to 
                    case 1:
                    case 2:
                        v = *pixel; break;
                        if (v >= palette.size)
                        {
                            return false;
                        }
                    case 3:
                    case 4:
                    {
                        uint8 r = *pixel, g = pixel[1], b = pixel[2];
                        int intensity = palette::index_from_color(palette, r, g, b);
                        if (intensity >= 0)
                        {
                            v = static_cast<uint8>(intensity);
                        }
                        else
                        {
                            return false;
                        }
                    }
                    break;
                    default: break;
                    }
                    out.data[i] = v * palette.scalar;
                }

                return true;
            }

            bool get_image_rect(const ByteImage& image, Rect<int>& destination)
            {
                if (image.width > 0 && image.height > 0)
                {
                    destination = { 0, 0, image.width - 1, image.height - 1 };
                    return true;
                }
                return false;
            }
        }

        namespace canvas
        {
            uint8* pixel_xy(Canvas& canvas, int x, int y)
            {
                return canvas.data.data() + x + y * canvas.width;
            }

            Canvas create_blank_from_screen()
            {
                Canvas result;
                result.width = g_gfx.screenCanvas.width;
                result.height = g_gfx.screenCanvas.height;
                result.data.resize(result.width * result.height);
                std::fill(result.data.begin(), result.data.end(), 0);
                return result;
            }

            Canvas create_copy_from_screen()
            {
                Canvas result;
                result.width = g_gfx.screenCanvas.width;
                result.height = g_gfx.screenCanvas.height;
                result.data.resize(result.width * result.height);
                std::copy(g_gfx.screenCanvas.data.begin(), g_gfx.screenCanvas.data.end(), result.data.begin());
                return result;
            }
        }
    }
}