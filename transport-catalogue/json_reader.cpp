#include "json_reader.h"
#include "json_builder.h"

void JsonReader::ReadJson(const Json::Node &requests, RequestHandler &rh) const {
    Json::Array result;
    for (auto &request: requests.AsArray()) {
        const auto &request_map = request.AsDict();
        const auto &type = request_map.at("type").AsString();
        if (type == "Stop") result.emplace_back(OutStop(request_map, rh).AsDict());
        if (type == "Bus") result.emplace_back(OutRoute(request_map, rh).AsDict());
        if (type == "Map") result.emplace_back(OutMap(request_map, rh).AsDict());
        if (type == "Route") result.emplace_back(OutRouting(request_map, rh).AsDict());
    }
    Json::Print(Json::Document{result}, std::cout);
}

const Json::Node &JsonReader::ProcessBaseRequests() const {
    if (!input_.GetRoot().AsDict().count("base_requests")) {
        return nothing_;
    }
    return input_.GetRoot().AsDict().at("base_requests");
}

const Json::Node &JsonReader::ProcessStatRequests() const {
    if (!input_.GetRoot().AsDict().count("stat_requests")) {
        return nothing_;
    }
    return input_.GetRoot().AsDict().at("stat_requests");
}

const Json::Node &JsonReader::ProcessRenderSettings() const {
    if (!input_.GetRoot().AsDict().count("render_settings")) {
        return nothing_;
    }
    return input_.GetRoot().AsDict().at("render_settings");
}


const Json::Node &JsonReader::ProcessRoutingSettings() const {
    if (!input_.GetRoot().AsDict().count("routing_settings")) {
        return nothing_;
    }
    return input_.GetRoot().AsDict().at("routing_settings");
}


void JsonReader::FillCatalogue(TCatalogue::TransportCatalogue &TCatalogue) {
    const Json::Array &arr = ProcessBaseRequests().AsArray();
    for (auto &request_stops: arr) {

        const auto &stops = request_stops.AsDict();
        const auto &type = stops.at("type").AsString();

        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = ProcessStop(stops);
            TCatalogue.AddStop(stop_name, coordinates);
        }
    }

    ProcessDistances(TCatalogue);

    for (auto &request_bus: arr) {
        const auto &request_bus_map = request_bus.AsDict();
        const auto &type = request_bus_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, isLoop] = ProcessBus(request_bus_map, TCatalogue);
            TCatalogue.AddBus(bus_number, stops, isLoop);
        }
    }
}

std::tuple<std::string_view, Geo::Coordinates, std::map<std::string_view, int>>
JsonReader::ProcessStop(const Json::Dict &request_map) {
    std::string_view stop_name = request_map.at("name").AsString();
    Geo::Coordinates coordinates = {request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble()};
    std::map<std::string_view, int> stopDistances;
    auto &distances = request_map.at("road_distances").AsDict();
    for (auto &[stop, dist]: distances) {
        stopDistances.emplace(stop, dist.AsInt());
    }
    return {stop_name, coordinates, stopDistances};
}

void JsonReader::ProcessDistances(TCatalogue::TransportCatalogue &TCatalogue) const {
    const Json::Array &array = ProcessBaseRequests().AsArray();
    for (auto &stops: array) {
        const auto &stopsMap = stops.AsDict();
        const auto &type = stopsMap.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stopDistances] = ProcessStop(stopsMap);
            for (auto &[to_name, dist]: stopDistances) {
                auto from = TCatalogue.FindStop(stop_name);
                auto to = TCatalogue.FindStop(to_name);
                TCatalogue.SetDistanseToTwoStops(from, to, dist);
            }
        }
    }
}

std::tuple<std::string_view, std::vector<std::string>, bool>
JsonReader::ProcessBus(const Json::Dict &request_map, TCatalogue::TransportCatalogue &catalogue) {
    std::string_view bus_number = request_map.at("name").AsString();
    std::vector<std::string> stops;
    for (auto &stop: request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.FindStop(stop.AsString())->name);
    }
    bool is_loop = request_map.at("is_roundtrip").AsBool();

    return std::make_tuple(bus_number, stops, is_loop);
}

TRouting::TRouter JsonReader::FillRouting(const Json::Node &requests) {
    TRouting::TRouter routing_settings;
    return TRouting::TRouter{requests.AsDict().at("bus_wait_time").AsInt(),
                             requests.AsDict().at("bus_velocity").AsDouble()};
}

Json::Node JsonReader::OutMap(const Json::Dict &request_map, RequestHandler &rh) {
    Json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    std::ostringstream stream;
    Svg::Document map = rh.RenderMap();
    map.Render(stream);
    result["map"] = stream.str();

    return Json::Node{result};
}

Json::Node JsonReader::OutRoute(const Json::Dict &request_map, RequestHandler &rh) {
    Json::Dict result;
    const std::string &route_number = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsBusNumber(route_number)) {
        result["error_message"] = Json::Node{static_cast<std::string>("not found")};
    } else {
        result["curvature"] = rh.GetBusStat(route_number)->curvature;
        result["route_length"] = rh.GetBusStat(route_number)->road_lenght;
        result["stop_count"] = static_cast<int>(rh.GetBusStat(route_number)->stops_count);
        result["unique_stop_count"] = static_cast<int>(rh.GetBusStat(route_number)->unique_stops);
    }

    return Json::Node{result};
}

Json::Node JsonReader::OutStop(const Json::Dict &request_map, RequestHandler &rh) {
    Json::Dict result;
    const std::string &stop_name = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsStopName(stop_name)) {
        result["error_message"] = Json::Node{static_cast<std::string>("not found")};
    } else {
        Json::Array buses;
        for (auto &bus: rh.GetBusesByStop(stop_name)) {
            buses.emplace_back(bus);
        }
        result["buses"] = buses;
    }

    return Json::Node{result};
}

Json::Node JsonReader::OutRouting(const Json::Dict &request_map, RequestHandler &rh) {
    const int id = request_map.at("id").AsInt();
    const std::string_view stop_from = request_map.at("from").AsString();
    const std::string_view stop_to = request_map.at("to").AsString();
    const auto &routing = rh.FetchRoute(stop_from, stop_to);

    if (!routing) {
        return CreateRouteErrorMessageNode(id, "not found");
    }

    Json::Array items;
    double total_time = 0.0;

    for (auto &edge_id: routing.value().edges) {
        const graph::Edge<double> edge = rh.ParseGraph().GetEdge(edge_id);
        items.emplace_back(CreateRouteItemNode(edge));
        total_time += edge.weight;
    }

    return CreateRouteResultNode(id, total_time, items);
}

Json::Node JsonReader::CreateRouteErrorMessageNode(int id, const std::string &message) {
    return Json::Builder{}
            .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value(message)
            .EndDict()
            .Build();
}

Json::Node JsonReader::CreateRouteItemNode(const graph::Edge<double> &edge) {
    Json::Dict item;

    if (edge.span == 0) {
        item = {
                {"stop_name", edge.name},
                {"time",      edge.weight},
                {"type",      "Wait"}
        };
    } else {
        item = {
                {"bus",        edge.name},
                {"span_count", static_cast<int>(edge.span)},
                {"time",       edge.weight},
                {"type",       "Bus"}
        };
    }

    return item;
}

Json::Node JsonReader::CreateRouteResultNode(int id, double total_time, const Json::Array &items) {
    return Json::Builder{}
            .StartDict()
            .Key("request_id").Value(id)
            .Key("total_time").Value(total_time)
            .Key("items").Value(items)
            .EndDict()
            .Build();
}

const Json::Node &JsonReader::ProcessSerializationSettings() const {
    if (input_.GetRoot().AsDict().count("serialization_settings"))
        return input_.GetRoot().AsDict().at("serialization_settings");
    else return nothing_;
}

