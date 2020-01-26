#pragma once

#include "types.h"

#include <limits>
#include <cmath>

namespace tdjx
{
    namespace math
    {
        const float32 PI = 3.1415927410125732421875;
        const float32 TWO_PI = PI * 2;
        const float32 TAU = TWO_PI;
        const float32 PI_OVER_2 = PI / 2;

        const float32 RAD_TO_DEG = 180.f / PI;
        const float32 DEG_TO_RAD = PI / 180.f;

        const float32 EPSILON = std::numeric_limits<float32>::epsilon();

        inline static float32 abs(float32 v)
        {
            return std::fabsf(v);
        }

        inline float32 cos(float32 degrees)
        {
            return std::cosf(degrees * DEG_TO_RAD);
        }

        inline float32 sin(float32 degrees)
        {
            return std::sinf(degrees * DEG_TO_RAD);
        }

        inline float32 tan(float32 degrees)
        {
            return std::tanf(degrees * DEG_TO_RAD);
        }

        inline float32 acos(float32 v)
        {
            return std::acosf(v) * RAD_TO_DEG;
        }

        inline float32 asin(float32 v)
        {
            return std::asinf(v) * RAD_TO_DEG;
        }

        inline float32 atan(float32 v)
        {
            return std::atan(v) * RAD_TO_DEG;
        }

        inline float32 atan2(float32 dy, float32 dx)
        {
            return std::atan2f(dy, dx) * RAD_TO_DEG;
        }

        inline float32 min(float32 a, float32 b)
        {
            return (a < b) ? a : b;
        }

        inline float32 max(float32 a, float32 b)
        {
            return (a > b) ? a : b;
        }

        inline float32 sign(float32 v)
        {
            return (v >= 0) ? 1.f : -1.f;
        }

        inline float32 pow(float32 f, float32 p)
        {
            return std::powf(f, p);
        }

        inline float32 exp(float32 p)
        {
            return std::expf(p);
        }

        inline float32 log(float32 f)
        {
            return std::logf(f);
        }

        inline float32 log(float32 f, float32 b)
        {
            return std::logf(f) / std::logf(b);
        }

        inline float32 log10(float32 f)
        {
            return std::log10f(f);
        }

        inline float32 log2(float32 f)
        {
            return std::log2f(f);
        }

        inline float32 floor(float32 v)
        {
            return std::floorf(v);
        }

        inline float32 ceil(float32 v)
        {
            return std::ceilf(v);
        }

        inline float32 round(float32 v)
        {
            return std::roundf(v);
        }

        inline float32 sqrt(float32 v)
        {
            return std::sqrtf(v);
        }

        inline int floor_int(float32 v)
        {
            return (int)floor(v);
        }

        inline int ceil_int(float32 v)
        {
            return (int)ceil(v);
        }

        inline int round_int(float32 v)
        {
            return (int)round(v);
        }

        inline float32 clamp(float32 v, float32 min, float32 max)
        {
            if (v < min)
            {
                return min;
            }
            else if (v > max)
            {
                return max;
            }
            else
            {
                return v;
            }
        }

        inline float32 clamp01(float32 v)
        {
            return clamp(v, 0, 1);
        }

        inline static bool approx(float32 a, float32 b)
        {
            return abs(b - a) * max(0.000001f * max(abs(a), abs(b)), EPSILON * 8);
        }

        inline static bool approx_zero(float32 v)
        {
            return approx(v, 0);
        }

        inline float32 repeat(float32 v, float32 max)
        {
            return clamp(v - floor(v / max) * max, 0.f, max);
        }

        inline float32 ping_pong(float32 v, float32 len)
        {
            v = repeat(v, len * 2.f);
            return len - abs(v - len);
        }

        //inline vec2 vec2_from_angle(float32 degrees)
        //{
        //    return vec2(cos(degrees), sin(degrees));
        //}

        //inline float32 angle_from_vec2(const vec2& normVec)
        //{
        //    return repeat(atan2(normVec.y, normVec.x), 360.f);
        //}

        template <typename t_type>
        inline t_type lerp(t_type a, t_type b, float32 t)
        {
            float32 af = static_cast<float32>(a);
            float32 bf = static_cast<float32>(b);
            return static_cast<t_type>(af + (bf - af) * t);
        }

        inline float32 lerp_angle(float32 a, float32 b, float32 t)
        {
            float32 delta = repeat(b - a, 360.f);
            if (delta > 180.f)
            {
                delta -= 360.f;
            }
            return a + delta * t;
        }

        // determines scalar value between [0-1] of v between a and b
        inline float32 inv_lerp(float32 a, float32 b, float32 v)
        {
            if (a != b)
            {
                return clamp01((v - a) / (b - a));
            }
            return 0.f;
        }

        inline float32 delta_angle(float32 a, float32 b)
        {
            float32 delta = repeat(b - a, 360.f);
            if (delta > 180.f)
            {
                delta -= 360.f;
            }
            return delta;
        }

        inline float32 move_to(float32 from, float32 to, float32 delta)
        {
            if (abs(to - from) <= delta)
            {
                return to;
            }
            return from + sign(to - from) * delta;
        }

        inline float32 rotate_to(float32 from, float32 to, float32 delta)
        {
            float32 da = delta_angle(from, to);
            if (-delta < da && da < delta)
            {
                return to;
            }
            return move_to(from, from + da, delta);
        }

        inline float32 smooth_step(float32 from, float32 to, float32 t)
        {
            t = clamp01(t);
            t = -2.f * t * t * t + 3.f * t * t;
            return to * t + from * (1.f - t);
        }

        inline float32 fade(float32 t)
        {
            return t * t * t * (t * (t * 6 - 15) + 10);
        }

        inline float32 grad(int hash, float32 x, float32 y, float32 z)
        {
            int h = hash & 15;
            float32 u = (h < 8) ? x : y;
            float32 v = (h < 4) ? y : h == 12 || h == 14 ? x : z;
            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        }

        inline float32 smooth_damp(float32 from, float32 to, float32& velocity, float32 time, float32 maxSpeed = std::numeric_limits<float32>::infinity(), float32 dt = 1.f / 60.f)
        {
            // from Unity math reference implementation
            // https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Mathf.cs
            // originally from Game Programming Gems 4 Chapter 1.10

            time = max(0.0001f, time);
            float32 omega = 2.f / time;

            float32 x = omega * dt;
            float32 exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);
            float32 delta = to - from;
            float32 origTo = to;

            float32 maxDelta = maxSpeed * time;
            delta = clamp(delta, -maxDelta, maxDelta);
            to = from - delta;

            float32 temp = (velocity + omega * delta) * dt;
            velocity = (velocity - omega * temp) * exp;
            float32 ret = to + (delta + temp) * exp;

            // prevent overshoots
            if (origTo - from > 0.0f == ret > origTo)
            {
                ret = origTo;
                velocity = (ret - origTo) / dt;
            }

            return ret;
        }

        inline float32 smooth_damp_angle(float32 from, float32 to, float32& velocity, float32 time, float32 maxSpeed = std::numeric_limits<float32>::infinity(), float32 dt = 1.f / 60.f)
        {
            float32 target = from + delta_angle(from, to);
            return smooth_damp(from, target, velocity, time, maxSpeed, dt);
        }

        template <typename t_type>
        struct Rect
        {
            t_type x0, y0, x1, y1;
        };

        template <typename t_type>
        static inline bool operator==(const Rect<t_type>& a, const Rect<t_type>& b)
        {
            return a.x0 == b.x0 && a.x1 == b.x1 && a.y0 == b.y0 && a.y1 == b.y1;
        };

        template <typename t_type>
        static inline bool operator!=(const Rect<t_type>& a, const Rect<t_type>& b)
        {
            return !(a == b);
        }

        namespace rect
        {
            const Rect<int> kInvalidIntRect = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };
            
            template <typename t_type>
            void unpack(const Rect<t_type>& self, t_type& x0, t_type& y0, t_type& x1, t_type& y1);

            template <typename t_type>
            int width(const Rect<t_type>& self);

            template <typename t_type>
            int height(const Rect<t_type>& self);

            template <typename t_type>
            Rect<t_type> lerp(const Rect<t_type>& a, const Rect<t_type>& b, float32 t);

            template <typename t_type>
            void constrain_to_aspect_ratio(Rect<t_type>& self, float32 aspectRatio);

            template <typename t_type>
            bool contains_point(const Rect<t_type>& self, t_type x, t_type y);

            template <typename t_type>
            bool intersect(const Rect<t_type>& a, const Rect<t_type>& b);

            template <typename t_type>
            bool clip_rect(const Rect<t_type>& source, Rect<t_type>& dest);

            template <typename t_type>
            bool clip_line(const Rect<t_type>& r, t_type& x0, t_type& y0, t_type& x1, t_type& y1);
        }

        namespace rect
        {
            template <typename t_type>
            inline void unpack(const Rect<t_type>& self, t_type& x0, t_type& y0, t_type& x1, t_type& y1)
            {
                x0 = self.x0;
                y0 = self.y0;
                x1 = self.x1;
                y1 = self.y1;
            }

            template <typename t_type>
            inline int width(const Rect<t_type>& self)
            {
                return std::abs(self.x1 - self.x0);
            }

            template <typename t_type>
            inline int height(const Rect<t_type>& self)
            {
                return std::abs(self.y1 - self.y0);
            }

            template <typename t_type>
            inline Rect<t_type> lerp(const Rect<t_type>& a, const Rect<t_type>& b, float32 t)
            {
                Rect<t_type> result;

                result.x0 = tdjx::math::lerp(a.x0, b.x0, t);
                result.y0 = tdjx::math::lerp(a.y0, b.y0, t);
                result.x1 = tdjx::math::lerp(a.x1, b.x1, t);
                result.y1 = tdjx::math::lerp(a.y1, b.y1, t);

                return result;
            }

            template <typename t_type>
            void constrain_to_aspect_ratio(Rect<t_type>& self, float32 aspectRatio)
            {
                t_type w = width(self);
                t_type h = height(self);
                
                float32 wf = static_cast<float32>(w);
                float32 hf = static_cast<float32>(h);

                aspectRatio = std::max(aspectRatio, 0.0001f);
                
                if (wf > hf * aspectRatio)
                {
                    hf = wf / aspectRatio;
                }
                else if (hf > wf / aspectRatio)
                {
                    wf = hf * aspectRatio;
                }

                self.x1 = self.x0 + static_cast<t_type>(wf);
                self.y1 = self.y0 + static_cast<t_type>(hf);
            }

            template <typename t_type>
            inline bool contains_point(const Rect<t_type>& self, t_type x, t_type y)
            {
                return x >= self.x0 && y >= self.y0 && x <= self.x1 && y <= self.y1;
            }

            template <typename t_type>
            inline bool intersect(const Rect<t_type>& a, const Rect<t_type>& b)
            {
                return a.x0 <= b.x1 &&
                    a.x1 >= b.x0 &&
                    a.y0 <= b.y1 &&
                    a.y1 >= b.y0;
            }

            template <typename t_type>
            bool clip_rect(const Rect<t_type>& source, Rect<t_type>& dest)
            {
                dest.x0 = std::max(dest.x0, source.x0);
                dest.y0 = std::max(dest.y0, source.y0);
                dest.x1 = std::min(dest.x1, source.x1);
                dest.y1 = std::min(dest.y1, source.y1);

                if (dest.x0 > dest.x1 || dest.y0 > dest.y1)
                {
                    return false;
                }

                return true;
            }

            template <typename t_type>
            bool clip_line(const Rect<t_type>& r, t_type& x0, t_type& y0, t_type& x1, t_type& y1)
            {
                struct point { t_type x, y; };

                const t_type xmin = r.x0;
                const t_type xmax = r.x1;
                const t_type ymin = r.y0;
                const t_type ymax = r.y1;

                enum side
                {
                    k_none = 0,
                    k_left = 1, k_right = 2, k_top = 4, k_bottom = 8
                };

                auto calcRegionCode = [=](const point& p)
                {
                    return static_cast<int>(
                        ((p.x < xmin) ? k_left : k_none) +
                        ((p.x > xmax) ? k_right : k_none) +
                        ((p.y < ymin) ? k_top : k_none) +
                        ((p.y > ymax) ? k_bottom : k_none));
                };

                point a = { x0, y0 };
                point b = { x1, y1 };

                while (true)
                {
                    int code1 = calcRegionCode(a);
                    int code2 = calcRegionCode(b);

                    if (code1 == 0 && code2 == 0)
                    {
                        // completely inside
                        goto _is_visible;
                    }
                    else
                    {
                        if ((code1 & code2) != 0)
                        {
                            // completely outside
                            return false;
                        }
                        else
                        {
                            // pick a point outside of the rectangle
                            point* outside = &a;
                            int outCode = code1;
                            if (code1 == 0)
                            {
                                outside = &b;
                                outCode = code2;
                            }

                            if (outCode & k_top)
                            {
                                outside->x = a.x + (b.x - a.x) * (ymin - a.y) / (b.y - a.y);
                                outside->y = ymin;
                            }
                            else if (outCode & k_bottom)
                            {
                                outside->x = a.x + (b.x - a.x) * (ymax - a.y) / (b.y - a.y);
                                outside->y = ymax;
                            }
                            else if (outCode & k_right)
                            {
                                outside->y = a.y + (b.y - a.y) * (xmax - a.x) / (b.x - a.x);
                                outside->x = xmax;
                            }
                            else if (outCode & k_left)
                            {
                                outside->y = a.y + (b.y - a.y) * (xmin - a.x) / (b.x - a.x);
                                outside->x = xmin;
                            }
                        }
                    }
                }

            _is_visible:
                {
                    x0 = a.x;
                    y0 = a.y;
                    x1 = b.x;
                    y1 = b.y;
                    return true;
                }
            }
        }
    }
}