// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>

class StreamFps
{
public:
    static constexpr int64_t noFrame = std::numeric_limits<int64_t>::min();

    void reset()
    {
        std::lock_guard<std::mutex> lock(mutex);
        lastPts = noFrame;
        startUs = 0;
        frames = 0;
    }

    void presented(int64_t pts, int64_t nowUs)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (pts <= lastPts)
            return;
        if (lastPts == noFrame)
            startUs = nowUs;
        else
            ++frames;
        lastPts = pts;
    }

    int sample(int64_t nowUs)
    {
        std::lock_guard<std::mutex> lock(mutex);
        const auto elapsed = nowUs - startUs;
        if (lastPts == noFrame || elapsed < 500000)
            return -1;
        const int fps = static_cast<int>(std::lround(frames * 1000000.0 / elapsed));
        startUs = nowUs;
        frames = 0;
        return fps;
    }

private:
    std::mutex mutex;
    int64_t lastPts = noFrame;
    int64_t startUs = 0;
    int frames = 0;
};
