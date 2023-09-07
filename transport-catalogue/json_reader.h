#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <iostream>
#include <utility>

class JsonReader {
public:
    JsonReader(std::istream &input)
            : input_(Json::Load(input)) {}

    JsonReader(Json::Document input)
            : input_(input) {}

    [[nodiscard]] const Json::Node &ProcessBaseRequests() const;

    [[nodiscard]] const Json::Node &ProcessStatRequests() const;

    [[nodiscard]] const Json::Node &ProcessRenderSettings() const;

    [[nodiscard]] const Json::Node &ProcessRoutingSettings() const;

    [[nodiscard]] const Json::Node &ProcessSerializationSettings() const;

    static Json::Node OutRoute(const Json::Dict &request_map, RequestHandler &rh) ;

    static Json::Node OutStop(const Json::Dict &request_map, RequestHandler &rh) ;

    static Json::Node OutMap(const Json::Dict &request_map, RequestHandler &rh) ;

    static Json::Node OutRouting(const Json::Dict &request_map, RequestHandler &rh);

    void FillCatalogue(TCatalogue::TransportCatalogue &TCatalogue);

    void ReadJson(const Json::Node &requests, RequestHandler &rh) const;

    [[nodiscard]] static TRouting::TRouter FillRouting(const Json::Node &requests) ;


private:
    // For routing
    [[nodiscard]] static Json::Node CreateRouteErrorMessageNode(int id, const std::string &message);

    [[nodiscard]] static Json::Node CreateRouteItemNode(const graph::Edge<double> &edge);

    [[nodiscard]] static Json::Node CreateRouteResultNode(int id, double total_time, const Json::Array &items);

    [[nodiscard]] static std::tuple<std::string_view, Geo::Coordinates, std::map<std::string_view, int>>
    ProcessStop(const Json::Dict &request_map) ;

    void ProcessDistances(TCatalogue::TransportCatalogue &TCatalogue) const;

    static std::tuple<std::string_view, std::vector<std::string>, bool>
    ProcessBus(const Json::Dict &request_map, TCatalogue::TransportCatalogue &catalogue) ;

    Json::Node nothing_ = nullptr;
    Json::Document input_;

};