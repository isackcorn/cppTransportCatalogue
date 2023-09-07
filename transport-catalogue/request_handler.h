#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <sstream>

class RequestHandler {
public:
    RequestHandler(const TCatalogue::TransportCatalogue &catalogue, const Render::MapRenderer& renderer, const TRouting::TRouter& router)
            : catalogue_(catalogue)
            , renderer_(renderer)
            , router_(router)
    {
    }

    [[nodiscard]] std::optional<TCatalogue::BusRouteInfo> GetBusStat(std::string_view bus_number) const;
    [[nodiscard]] std::set<std::string> GetBusesByStop(std::string_view stop_name) const;
    [[nodiscard]] bool IsBusNumber(std::string_view bus_number) const;
    [[nodiscard]] bool IsStopName(std::string_view stop_name) const;

    [[nodiscard]] Svg::Document RenderMap() const;

    [[nodiscard]] std::optional<graph::Router<double>::RouteInfo> FetchRoute(std::string_view stop_from, const std::string_view stop_to) const;
    [[nodiscard]] const graph::DirectedWeightedGraph<double>& ParseGraph() const;

private:
    const TCatalogue::TransportCatalogue& catalogue_;
    const Render::MapRenderer& renderer_;
    const TRouting::TRouter& router_;
};