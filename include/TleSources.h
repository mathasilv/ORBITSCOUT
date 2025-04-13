#ifndef TLESOURCES_H
#define TLESOURCES_H

struct TleSource {
    const char* url;
    const char* fileName;
    const char* group;
};

static const TleSource tleSources[] = {
    { "https://celestrak.org/NORAD/elements/gp.php?GROUP=noaa&FORMAT=tle", "/tle_noaa.txt", "NOAA" },
    { "https://celestrak.org/NORAD/elements/gp.php?GROUP=weather&FORMAT=tle", "/tle_weather.txt", "Weather" },
    { "https://celestrak.org/NORAD/elements/gp.php?GROUP=engineering&FORMAT=tle", "/tle_engineering.txt", "Engineering" },
    { "https://celestrak.org/NORAD/elements/gp.php?GROUP=science&FORMAT=tle", "/tle_science.txt", "Space & Earth Science" },
    { "https://celestrak.org/NORAD/elements/gp.php?GROUP=cubesat&FORMAT=tle", "/tle_cubesat.txt", "Cubesats" },  
    { "https://celestrak.org/NORAD/elements/gp.php?GROUP=education&FORMAT=tle", "/tle_educational.txt", "Educational" }

};


static constexpr int TLE_SOURCE_COUNT = sizeof(tleSources) / sizeof(tleSources[0]);

#endif