#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include "geo.h"
#include <unordered_map>
#include <set>

namespace TCatalogue {
    struct Bus;

    struct Stop {
        std::string name;
        Geo::Coordinates coordinates;
    };


    struct BusRouteInfo {
        int stops_count = 0;
        int unique_stops = 0;
        double road_lenght = 0.0;
        double curvature = 0.0;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop *> stops;
        bool is_loop;
    };
}