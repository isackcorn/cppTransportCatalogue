#pragma once

#include <memory>
#include "transport_catalogue.h"
#include "router.h"
#include "json.h"

namespace TRouting {
    class TRouter {
    public:
        TRouter() = default;

        TRouter(int wait_time, double speed) : wait_time_(wait_time), speed_(speed) {}

        TRouter(const TRouter &base, const TCatalogue::TransportCatalogue &catalogue) {
            wait_time_ = base.wait_time_;
            speed_ = base.speed_;
            MakeRoute(catalogue);
        }

        TRouter(const Json::Node &settings) {
            if (settings.IsNull()) return;
            SetSettings(settings);
        }

        using Graph = graph::DirectedWeightedGraph<double>;
        using BusR = TCatalogue::Bus;

        const Graph &MakeRoute(const TCatalogue::TransportCatalogue &catalogue);

        [[nodiscard]] std::optional<graph::Router<double>::RouteInfo>
        FindRoute(std::string_view stop_from, std::string_view stop_to) const;

        [[nodiscard]] const Graph &GetGraph() const;

        [[nodiscard]] Json::Node GetBusSettings() const;

        const std::map<std::string, graph::VertexId> &GetStopIds() const;

        void SetGraph(graph::DirectedWeightedGraph<double> &&graph,
                      std::map<std::string, graph::VertexId> &&stop_ids);

    private:
        void BuildStopsGraph(const TCatalogue::TransportCatalogue &catalogue,
                             Graph &stops_graph);

        void AddEdgesForBus(const BusR &bus, const TCatalogue::TransportCatalogue &catalogue,
                            Graph &stops_graph);

        void AddEdgesForStops(const BusR &bus, const std::vector<const TCatalogue::Stop *> &stops,
                              Graph &stops_graph,
                              const TCatalogue::TransportCatalogue &catalogue);

        void AddEdgeForStops(const BusR &bus, size_t stop_from_index, size_t stop_to_index,
                             const std::vector<const TCatalogue::Stop *> &stops,
                             Graph &stops_graph,
                             const TCatalogue::TransportCatalogue &catalogue);

        void SetSettings(const Json::Node &settings_node);

        int wait_time_ = 0;
        double speed_ = 0.0;
        std::map<std::string, graph::VertexId> stop_ids_;
        Graph graph_;
        std::unique_ptr<graph::Router<double>> router_;
    };
}
