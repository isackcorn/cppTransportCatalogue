/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

#include "request_handler.h"

std::optional<TCatalogue::BusRouteInfo> RequestHandler::GetBusStat(const std::string_view bus_number) const {
    return catalogue_.GetRouteInfo(bus_number);
}

std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.GetBusesOnStop(stop_name);
}

bool RequestHandler::IsBusNumber(const std::string_view bus_number) const {
    return catalogue_.GetBusInfo(bus_number);
}

bool RequestHandler::IsStopName(const std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name);
}

Svg::Document RequestHandler::RenderMap() const {
    return renderer_.ParseSvg(catalogue_.ReturnAllBus());
}

std::optional<graph::Router<double>::RouteInfo>
RequestHandler::FetchRoute(const std::string_view stop_from, const std::string_view stop_to) const {
    return router_.FindRoute(stop_from, stop_to);
}

const graph::DirectedWeightedGraph<double> &RequestHandler::ParseGraph() const {
    return router_.GetGraph();
}
