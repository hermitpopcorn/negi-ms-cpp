#pragma once

#include <chrono>
#include <ctime>

// Helper function to create time points for testing
inline std::chrono::system_clock::time_point makeTimePoint(int year, int month, int day, int hour = 0, int minute = 0, int second = 0)
{
    std::tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    t.tm_isdst = -1;

    std::time_t tt = std::mktime(&t);

    return std::chrono::system_clock::from_time_t(tt);
}
