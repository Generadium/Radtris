#ifndef LEVEL_PROGRESSION_H
#define LEVEL_PROGRESSION_H

#include <algorithm>
#include <array>
#include <chrono>

class LevelProgression
{
public:
    static constexpr int MAX_LEVEL = 10;
    static constexpr int LINES_PER_LEVEL = 10;

    void reset() { lines_cleared = 0; }
    void add_lines(int count) { lines_cleared += count; }

    int lines() const { return lines_cleared; }
    int level() const { return std::min(lines_cleared / LINES_PER_LEVEL, MAX_LEVEL); }

    std::chrono::microseconds gravity_interval() const
    {
        // Classic NES gravity, in frames per row at 60 frames per second.
        // https://github.com/CelestialAmber/TetrisNESDisasm/blob/master/main.asm
        static constexpr std::array<int, MAX_LEVEL + 1> frames_per_row = {
            48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5
        };
        return std::chrono::microseconds{frames_per_row[level()] * 1000000 / 60};
    }

private:
    int lines_cleared = 0;
};

#endif
