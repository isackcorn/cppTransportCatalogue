#include "transport_catalogue.h"
#include <stdexcept>
#include "geo.h"
#include <set>

namespace TCatalogue {

    void TransportCatalogue::AddStop(const std::string_view &stop_name, const Geo::Coordinates &coordinates) {
        stops_.push_back({std::string(stop_name), coordinates});
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    const Stop *TransportCatalogue::FindStop(const std::string_view &stop_name) const {
        if (stopname_to_stop_.count(stop_name)) {
            return stopname_to_stop_.at(stop_name);
        } else return nullptr;

    }

    void
    TransportCatalogue::AddBus(const std::string_view &bus_name, const std::vector<std::string> &stops, bool is_loop) {
        buses_.push_back({std::string(bus_name), GetStopsPtrFromString(stops), is_loop});
        busname_to_bus_[buses_.back().name] = &buses_.back();

        for (const auto &stop: stops) {
            for (auto &inner_stop: stops_) {
                if (inner_stop.name == stop) {
                    stop_to_busnames_[stopname_to_stop_[stop]].insert(busname_to_bus_.at(bus_name)); //?
                }
            }
        }

        bus_to_route_info_[busname_to_bus_.at(bus_name)] = CalculateRouteInfo(bus_name);
    }

    void TransportCatalogue::AddBusFromDb(const std::string_view &bus_name, const std::vector<std::string> &stops,
                                          bool is_loop, BusRouteInfo busRouteInfo) {
        buses_.push_back({std::string(bus_name), GetStopsPtrFromString(stops), is_loop});
        busname_to_bus_[buses_.back().name] = &buses_.back();

        for (const auto &stop: stops) {
            for (auto &inner_stop: stops_) {
                if (inner_stop.name == stop) {
                    stop_to_busnames_[stopname_to_stop_[stop]].insert(busname_to_bus_.at(bus_name)); //?
                }
            }
        }

        bus_to_route_info_[busname_to_bus_.at(bus_name)] = busRouteInfo;
    }

    const Bus *TransportCatalogue::GetBusInfo(const std::string_view &bus) const {
        if (busname_to_bus_.count(bus)) {
            return busname_to_bus_.at(bus);
        } else return nullptr;
    }

    std::vector<const Stop *> TransportCatalogue::GetStopsPtrFromString(const std::vector<std::string> &stops) {

        std::vector<const Stop *> to_return;

        to_return.reserve(stops.size());
        for (const auto &stop: stops) {
            to_return.push_back(stopname_to_stop_.at(stop));
        }

        return to_return;

    }

    BusRouteInfo TransportCatalogue::GetRouteInfo(const std::string_view &route_name) const {
        return bus_to_route_info_.at(busname_to_bus_.at(route_name));
    }

    BusRouteInfo TransportCatalogue::CalculateRouteInfo(const std::string_view &route_name) const {

        const Bus *bus = GetBusInfo(route_name);
        if (bus == nullptr) {
            throw std::invalid_argument("not found");
        }

        double geoLenght = 0.0;
        double roadLenght = 0.0;

        int stopsCount = static_cast<int>(bus->stops.size());

        if (bus->is_loop) {
            for (int i = 0; i + 1 != static_cast<int>(bus->stops.size()); ++i) {
                const Stop *start = bus->stops[i];
                const Stop *end = bus->stops[i + 1];
                roadLenght += GetDistanceFromTwoStops(start, end);
                geoLenght += ComputeDistance(start->coordinates, end->coordinates);
            }
        } else {
            for (int i = 0; i + 1 != static_cast<int>(bus->stops.size()); ++i) {
                const Stop *start = bus->stops[i];
                const Stop *end = bus->stops[i + 1];
                roadLenght += (GetDistanceFromTwoStops(start, end) + GetDistanceFromTwoStops(end, start));
                geoLenght += ComputeDistance(start->coordinates, end->coordinates) * 2.0;
            }
        }


        if (!bus->is_loop) {
            stopsCount = stopsCount * 2 - 1;
        }

        auto unique_stops = GetUniqueStops(route_name);

        auto to_test = roadLenght / geoLenght;

        return {stopsCount, unique_stops, roadLenght, roadLenght / geoLenght};
    }

    int TransportCatalogue::GetUniqueStops(const std::string_view &routetofind) const {
        std::set<std::string> unique_stops;

        for (const auto &elem: busname_to_bus_.at(routetofind)->stops) {
            unique_stops.insert(elem->name);
        }

        return static_cast<int>(unique_stops.size());
    }

    std::set<std::string> TransportCatalogue::GetBusesOnStop(const std::string_view &stop_name) const {
        std::set<const Bus *> buses;
        std::set<std::string> to_return;
        if (!stop_to_busnames_.count(stopname_to_stop_.at(stop_name))) {
            return {};
        } else {
            buses = stop_to_busnames_.at(stopname_to_stop_.at(stop_name));
        }

        for (const auto &bus: buses) {
            to_return.insert(bus->name);
        }

        return to_return;
    }

    int TransportCatalogue::GetDistanceFromTwoStops(const Stop *from, const Stop *to) const {
        if (stops_pair_to_distance_.count({from, to})) {
            return stops_pair_to_distance_.at({from, to});
        } else if (stops_pair_to_distance_.count({to, from})) {
            return stops_pair_to_distance_.at({to, from});
        }
        return 0;
    }

    void TransportCatalogue::SetDistanseToTwoStops(const Stop *from, const Stop *to, const int &distance) {
        stops_pair_to_distance_[{from, to}] = distance;
    }

    std::map<std::string_view, const Bus *> TransportCatalogue::ReturnAllBus() const {
        std::map<std::string_view, const Bus *> result;
        for (const auto &bus: busname_to_bus_) {
            result.emplace(bus);
        }
        return result;
    }

    std::map<std::string_view, const Stop *> TransportCatalogue::ReturnAllStops() const {
        std::map<std::string_view, const Stop *> result;
        for (const auto &stop: stopname_to_stop_) {
            result.emplace(stop);
        }
        return result;
    }


}