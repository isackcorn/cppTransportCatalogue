#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include "geo.h"
#include <unordered_map>
#include <set>
#include "domain.h"
#include <map>

namespace TCatalogue {

    class TransportCatalogue {
    public:

        void AddStop(const std::string_view &stop_name, const Geo::Coordinates &coordinates);

        const Stop *FindStop(const std::string_view &stop_name) const;

        void AddBus(const std::string_view &bus_name, const std::vector<std::string> &stops, bool is_loop);

        void AddBusFromDb(const std::string_view &bus_name, const std::vector<std::string> &stops, bool is_loop,
                          BusRouteInfo busRouteInfo);

        const Bus *GetBusInfo(const std::string_view &bus) const;

        BusRouteInfo GetRouteInfo(const std::string_view &route_name) const;

        int GetUniqueStops(const std::string_view &routetofind) const;

        std::set<std::string> GetBusesOnStop(const std::string_view &stop_name) const;

        int GetDistanceFromTwoStops(const Stop *from, const Stop *to) const;

        BusRouteInfo CalculateRouteInfo(const std::string_view &route_name) const;

        void SetDistanseToTwoStops(const Stop *from, const Stop *to, const int &distance);

        std::map<std::string_view, const Bus *> ReturnAllBus() const;

        std::map<std::string_view, const Stop *> ReturnAllStops() const;


        struct StopHasher {
            size_t operator()(const std::pair<const Stop *, const Stop *> &pair) const {
                return ((std::hash<const void *>{}(pair.first) * 53) +
                        (std::hash<const void *>{}(pair.second) * 53 * 53));
            }
        };

        std::unordered_map<std::pair<const Stop *, const Stop *>, int, StopHasher> stops_pair_to_distance_;

    private:
        std::vector<const Stop *> GetStopsPtrFromString(const std::vector<std::string> &stops);

        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Stop *> stopname_to_stop_;
        std::unordered_map<const Stop *, std::set<const Bus *>> stop_to_busnames_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, const Bus *> busname_to_bus_;
        std::unordered_map<const Bus *, BusRouteInfo> bus_to_route_info_;


    };
}