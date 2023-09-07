#include "transport_router.h"

namespace TRouting {

    void TRouter::BuildStopsGraph(const TCatalogue::TransportCatalogue &catalogue,
                                  Graph &stops_graph) {
        const auto &all_stops = catalogue.ReturnAllStops();
        graph::VertexId vertex_id = 0;

        for (const auto &[stop_name, stop_info]: all_stops) {
            stop_ids_[stop_info->name] = vertex_id;
            stops_graph.AddEdge({stop_info->name, 0, vertex_id, ++vertex_id, static_cast<double>(wait_time_)});
            ++vertex_id;
        }
    }

    void TRouter::AddEdgeForStops(const BusR &bus, size_t stop_from_index, size_t stop_to_index,
                                  const std::vector<const TCatalogue::Stop *> &stops,
                                  Graph &stops_graph,
                                  const TCatalogue::TransportCatalogue &catalogue) {

        const auto *stop_from = stops[stop_from_index];
        const auto *stop_to = stops[stop_to_index];

        int length = 0;
        int lengthInv = 0;

        for (size_t i = stop_from_index + 1; i <= stop_to_index; ++i) {
            length += catalogue.GetDistanceFromTwoStops(stops[i - 1], stops[i]);
            lengthInv += catalogue.GetDistanceFromTwoStops(stops[i], stops[i - 1]);
        }

        stops_graph.AddEdge({bus.name, stop_to_index - stop_from_index, stop_ids_.at(stop_from->name) + 1,
                             stop_ids_.at(stop_to->name),
                             static_cast<double>(length) / (speed_ * (1000.0 / 60.0))});

        if (!bus.is_loop) {
            stops_graph.AddEdge({bus.name, stop_to_index - stop_from_index, stop_ids_.at(stop_to->name) + 1,
                                 stop_ids_.at(stop_from->name),
                                 static_cast<double>(lengthInv) / (speed_ * (1000.0 / 60.0))});
        }
    }

    void TRouter::AddEdgesForStops(const BusR &bus, const std::vector<const TCatalogue::Stop *> &stops,
                                   Graph &stops_graph,
                                   const TCatalogue::TransportCatalogue &catalogue) {
        for (size_t i = 0; i < stops.size(); ++i) {
            for (size_t j = i + 1; j < stops.size(); ++j) {
                AddEdgeForStops(bus, i, j, stops, stops_graph, catalogue);
            }
        }
    }

    void TRouter::AddEdgesForBus(const BusR &bus, const TCatalogue::TransportCatalogue &catalogue,
                                 Graph &stops_graph) {
        const auto &stops = bus.stops;
        AddEdgesForStops(bus, stops, stops_graph, catalogue);
    }

    const TRouter::Graph &TRouter::MakeRoute(const TCatalogue::TransportCatalogue &catalogue) {
        Graph stops_graph(catalogue.ReturnAllStops().size() * 2);
        BuildStopsGraph(catalogue, stops_graph);

        for (const auto &[bus_name, bus_info]: catalogue.ReturnAllBus()) {
            AddEdgesForBus(*bus_info, catalogue, stops_graph);
        }

        graph_ = std::move(stops_graph);
        router_ = std::make_unique<graph::Router<double>>(graph_);
        return graph_;
    }

    std::optional<graph::Router<double>::RouteInfo>
    TRouter::FindRoute(std::string_view stop_from, std::string_view stop_to) const {
        return router_->BuildRoute(stop_ids_.at(std::string(stop_from)), stop_ids_.at(std::string(stop_to)));
    }

    const TRouter::Graph &TRouter::GetGraph() const {
        return graph_;
    }

    Json::Node TRouter::GetBusSettings() const {
        return Json::Node(Json::Dict{
                {{"bus_wait_time"}, {wait_time_}},
                {{"bus_velocity"},  {speed_}}
        });
    }

    const std::map<std::string, graph::VertexId> &TRouter::GetStopIds() const {
        return stop_ids_;
    }

    void TRouter::SetSettings(const Json::Node &settings_node) {
        wait_time_ = settings_node.AsDict().at("bus_wait_time").AsInt();
        speed_ = settings_node.AsDict().at("bus_velocity").AsDouble();
    }

    void
    TRouter::SetGraph(graph::DirectedWeightedGraph<double> &&graph, std::map<std::string, graph::VertexId> &&stop_ids) {
        graph_ = std::move(graph);
        stop_ids_ = std::move(stop_ids);
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }
}
