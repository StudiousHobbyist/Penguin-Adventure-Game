#pragma once

#include <cmath>
#include <iostream>
#include <algorithm>

// TODO: Clean up code.

/*
Custom Noise Generation.
*/

struct Noise
{
    int seed = 2;
    float rotation_scale = 1.618f;
    float seed_scale = 2.718f;

    std::pair<long double, long double> rotated(long double px, long double py, long double i) const
    {
        long double angle = i * rotation_scale + seed * seed_scale;

        return {
            px * std::cosl(angle) - py * std::sinl(angle),
            px * std::sinl(angle) + py * std::cosl(angle)
        };
    }

    std::pair<long double, long double> rotatedAbout(long double px, long double py, long double ox, long double oy, long double i) const
    {
        long double angle = i * rotation_scale + seed * seed_scale;

        long double tx = px - ox;
        long double ty = py - oy;

        long double rx = tx * std::cosl(angle) - ty * std::sinl(angle);
        long double ry = tx * std::sinl(angle) + ty * std::cosl(angle);

        return { rx + ox, ry + oy };
    }

    long double wrap(long double v, long double d) const
    {
        v = std::fmod(v, d);
        if (v < 0) v += d;
        return v;
    }

    long double twave(long double v) const
    {
        return (std::abs(wrap(v, 4.0l) - 2.0l) - 1.0l) * std::sinl(v);
    }

    long double lerp(long double a, long double b, long double t) const
    {
        return a + (b - a) * t;
    }

    long double distance(long double x1, long double y1, long double x2, long double y2) const {
        long double dx = x2 - x1;
        long double dy = y2 - y1;

        return std::sqrt(dx * dx + dy * dy);
    }

    long double smoothlim(long double x) const {
        long double n = 0.5l;
        long double ax = std::abs(x);
        return (std::pow(ax, n) * x) / (std::pow(ax, n + 1) + std::pow(ax, n) + ax + 1.0l);
    }

    long double falloff(long double x) const {
        long double decay = 1.0e0l;
        return decay / (decay + std::pow(std::abs(x), 1.0l));
    }

    void checkNanInf(long double v) const {
        if (std::isnan(v) || std::isinf(v)) {
            std::cout << "Nan or Inf." << std::endl;
        }
    }
    long double warpedSpeed(long double t) const
    {
        return twave(twave(t) + t) + t;
    }

    long double SampleElevation(long double x, long double y) const {
        // DONE: Main land masses
        // TODO: Montain ridges, rivers, lakes, valleys, distant mountain.

        int octaves = 128;
        int superOctaves = 2;

        long double detailScale = 1.0e1l;

        long double dx = x / detailScale;
        long double dy = y / detailScale;


        long double superTotal = 0.0l;

        for (int j = 1; j < superOctaves + 1; j++) {
            long double total = 0.0l;
            long double j2 = std::pow(1.3l, j);

            for (int i = 1; i < octaves + 1; i++) {

                long double i2 = std::pow(1.3l, i);

                auto [rx, ry] = rotated(dx + i2 * 100, dy + j2 * 100, i2);
                auto [rax, ray] = rotatedAbout(rx, ry, dx + j2, dy + i2, j2);

                total += twave((rax / (i)+i2)) + twave((ray / (i)+i2));
                // total += twave((rax / (i) + i2)) + twave((ray / (i) + i2));
            }

            superTotal += total / (std::sqrt(octaves) * 1.2l);
        }


        long double landmass = superTotal / (std::sqrt(superOctaves) * 1.2l);

        return std::clamp(landmass, -1.0l, 1.0l);
    }

    long double __SampleElevation(long double x, long double y) const
    {
        // version 0 of the elevation function, for comparison
        long double detailScale = 1.0l;
        long double contScale = detailScale * 10.0l;

        long double dx = x / detailScale;
        long double dy = y / detailScale;

        long double cx = x / contScale;
        long double cy = y / contScale;

        int octaves = 1024;

        long double detailDistance = std::sqrt(dx * dx + dy * dy);
        long double contDistance = std::sqrt(cx * cx + cy * cy);

        long double detailDistanceRS = 1 / (1 + detailDistance);
        long double contDistanceRS = 1 / (1 + contDistance);

        long double detailAvg1 = 0.0l;

        for (int i = 1; i < octaves + 1; i++)
        {
            long double fl = (long double)i;

            auto r1 = rotated(dx + fl * fl, dy, fl * fl);
            auto r2 = rotated(dx, dy + fl, fl * (fl / 2));
            auto ra1 = rotatedAbout(r1.first, r1.second, dx, dy, fl * fl);

            detailAvg1 += lerp(
                -1,
                1,
                (twave(r1.first + r2.second) + 0.5) / 2 + 0.25
            );
        }

        detailAvg1 /= octaves;

        long double detailAvg2 = 0.0l;

        for (int i = 1; i < octaves + 1; i++)
        {
            long double fl = (long double)i;

            auto r1 = rotated(dx + fl, dy, fl * (fl / 2));
            auto r2 = rotated(dx, dy + fl * fl, fl * fl);
            auto ra1 = rotatedAbout(r1.first, r1.second, dx, dy, fl * fl);

            detailAvg2 -= lerp(
                -1,
                1,
                (twave(r1.first + r2.second) + 0.5) / 2 - 0.25
            );
        }

        detailAvg2 /= octaves;

        long double contAvg1 = 0.0l;

        for (int i = 1; i < octaves + 1; i++)
        {
            long double fl = (long double)i;

            auto r1 = rotated(cx + fl, cy + contDistanceRS, fl * (fl / 2));
            auto r2 = rotated(cx - contDistanceRS, cy + fl * fl, fl * fl);
            auto ra1 = rotatedAbout(r1.first, r1.second, cx, cy, fl * fl);

            contAvg1 += lerp(
                -1,
                5,
                twave(r1.first - r2.second)
            );
        }

        contAvg1 /= octaves;

        long double detailTotal = twave(detailAvg2 + detailAvg1);

        long double contTotal = twave(contAvg1);
        if (contTotal > 0.7) { contTotal = 0.7l; }

        return detailTotal * 0.7 + contTotal;
    }
};
