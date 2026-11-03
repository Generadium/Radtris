#include "LevelProgression.h"

#include <cstdlib>
#include <iostream>

void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main()
{
    using namespace std::chrono;

    LevelProgression progression;
    check(progression.lines() == 0 && progression.level() == 0,
          "A new game must start at level zero with no cleared lines.");
    check(progression.gravity_interval() == milliseconds{800},
          "Level zero must use the classic 48-frame drop interval.");

    progression.add_lines(9);
    check(progression.level() == 0, "Nine lines must not advance the level.");
    progression.add_lines(4);
    check(progression.level() == 1 && progression.lines() == 13,
          "A multi-line clear must advance the level and retain excess lines.");
    progression.add_lines(6);
    check(progression.level() == 1, "Excess lines must count toward the next level.");
    progression.add_lines(1);
    check(progression.level() == 2, "Twenty total lines must reach level two.");

    progression.reset();
    for (int expected_level = 1; expected_level <= 10; ++expected_level)
    {
        const auto previous_interval = progression.gravity_interval();
        progression.add_lines(9);
        check(progression.level() == expected_level - 1,
              "Every level must require ten more cleared lines.");
        progression.add_lines(1);
        check(progression.level() == expected_level, "A level threshold was missed.");
        check(progression.gravity_interval() < previous_interval,
              "Gravity must get faster at every level through ten.");
    }

    const auto maximum_speed = progression.gravity_interval();
    check(maximum_speed == microseconds{83333},
          "Level ten must use the classic five-frame drop interval.");
    progression.add_lines(104);
    check(progression.level() == 10 && progression.lines() == 204,
          "Level ten must be capped while cleared lines continue accumulating.");
    check(progression.gravity_interval() == maximum_speed,
          "Gravity must remain capped at the level-ten speed.");

    progression.reset();
    check(progression.level() == 0 && progression.lines() == 0 &&
          progression.gravity_interval() == milliseconds{800},
          "Replaying must reset the level, lines, and gravity together.");
}
