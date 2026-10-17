#pragma once

// The landing screen's wording, kept free of widgets so it can be tested.

#include <cstddef>
#include <string>

// Late at night "good night" would read as a farewell, so it says hello.
inline const char* greeting_for_hour(int hour) {
    if (hour >= 5 && hour < 12) {
        return "Good morning";
    }
    if (hour >= 12 && hour < 18) {
        return "Good afternoon";
    }
    if (hour >= 18 && hour < 23) {
        return "Good evening";
    }
    return "Hello";
}

// "Today" and "Yesterday" are how people place something recent. Anything older,
// or a date from a clock that has since moved backwards, is shown as a date and
// gets an empty string here.
inline const char* relative_day(long long days_ago) {
    if (days_ago == 0) {
        return "Today";
    }
    if (days_ago == 1) {
        return "Yesterday";
    }
    return "";
}

// "No transcripts", "1 transcript", "12 transcripts".
inline std::string count_phrase(std::size_t count, const char* singular, const char* plural) {
    if (count == 0) {
        return std::string("No ") + plural;
    }
    return std::to_string(count) + " " + (count == 1 ? singular : plural);
}
