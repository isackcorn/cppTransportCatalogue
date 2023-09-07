#include "serialization.h"

using namespace std;

void Serialize(const TCatalogue::TransportCatalogue &TCatalog,
               const Render::MapRenderer &renderer, const TRouting::TRouter &router,
               std::ostream &output) {
    serialization::TransportCatalogue database;
    for (const auto &[name, s]: TCatalog.ReturnAllStops()) {
        *database.add_stop() = Serialize(TCatalog, s);
    }
    for (const auto &[name, b]: TCatalog.ReturnAllBus()) {
        *database.add_bus() = Serialize(b, TCatalog);
    }
    *database.mutable_render_settings() = GetRenderSettingSerialize(renderer.GetRenderSetup());
    *database.mutable_router() = Serialize(router);
    database.SerializeToOstream(&output);
}

serialization::Stop Serialize(const TCatalogue::TransportCatalogue &transportCatalogue,
                              const TCatalogue::Stop *stop) {
    serialization::Stop result;
    result.set_name(stop->name);
    result.add_coordinate(stop->coordinates.lat);
    result.add_coordinate(stop->coordinates.lng);
    return result;
}


serialization::Bus Serialize(const TCatalogue::Bus *bus, const TCatalogue::TransportCatalogue &transportCatalogue) {

    serialization::Bus result;
    result.set_name(bus->name);
    for (const auto &s: bus->stops) {
        result.add_stop(s->name);
    }
    result.set_is_circle(bus->is_loop);

    auto route_info = transportCatalogue.GetRouteInfo(bus->name);
    result.set_stops_count(route_info.stops_count);
    result.set_unique_stops(route_info.unique_stops);
    result.set_road_lenght(route_info.road_lenght);
    result.set_curvature(route_info.curvature);

    return result;
}

serialization::Point GetPointSerialize(const Json::Array &p) {
    serialization::Point result;
    result.set_x(p[0].AsDouble());
    result.set_y(p[1].AsDouble());
    return result;
}

serialization::Color GetColorSerialize(const Json::Node &node) {
    serialization::Color result;
    if (node.IsArray()) {
        const Json::Array &arr = node.AsArray();
        if (arr.size() == 3) {
            serialization::RGB rgb;
            rgb.set_red(arr[0].AsInt());
            rgb.set_green(arr[1].AsInt());
            rgb.set_blue(arr[2].AsInt());
            *result.mutable_rgb() = rgb;
        } else if (arr.size() == 4) {
            serialization::RGBA rgba;
            rgba.set_red(arr[0].AsInt());
            rgba.set_green(arr[1].AsInt());
            rgba.set_blue(arr[2].AsInt());
            rgba.set_opacity(arr[3].AsDouble());
            *result.mutable_rgba() = rgba;
        }
    } else if (node.IsString()) {
        result.set_name(node.AsString());
    }
    return result;
}

serialization::RenderSettings GetRenderSettingSerialize(const Json::Node &render_settings) {
    const Json::Dict &rs_map = render_settings.AsDict();
    serialization::RenderSettings result;
    result.set_width(rs_map.at("width"s).AsDouble());
    result.set_height(rs_map.at("height"s).AsDouble());
    result.set_padding(rs_map.at("padding"s).AsDouble());
    result.set_stop_radius(rs_map.at("stop_radius"s).AsDouble());
    result.set_line_width(rs_map.at("line_width"s).AsDouble());
    result.set_bus_label_font_size(rs_map.at("bus_label_font_size"s).AsInt());
    *result.mutable_bus_label_offset() = GetPointSerialize(rs_map.at("bus_label_offset"s).AsArray());
    result.set_stop_label_font_size(rs_map.at("stop_label_font_size"s).AsInt());
    *result.mutable_stop_label_offset() = GetPointSerialize(rs_map.at("stop_label_offset"s).AsArray());
    *result.mutable_underlayer_color() = GetColorSerialize(rs_map.at("underlayer_color"s));
    result.set_underlayer_width(rs_map.at("underlayer_width"s).AsDouble());
    for (const auto &c: rs_map.at("color_palette"s).AsArray()) {
        *result.add_color_palette() = GetColorSerialize(c);
    }
    return result;
}

serialization::RouterSettings GetRouterSettingSerialize(const Json::Node &router_settings) {
    const Json::Dict &rs_map = router_settings.AsDict();
    serialization::RouterSettings result;
    result.set_bus_wait_time(rs_map.at("bus_wait_time"s).AsInt());
    result.set_bus_velocity(rs_map.at("bus_velocity"s).AsDouble());
    return result;
}

serialization::Graph GetGraphSerialize(const graph::DirectedWeightedGraph<double> &g) {
    serialization::Graph result;
    size_t vertex_count = g.GetVertexCount();
    size_t edge_count = g.GetEdgeCount();
    for (size_t i = 0; i < edge_count; ++i) {
        const graph::Edge<double> &edge = g.GetEdge(i);
        serialization::Edge s_edge;
        s_edge.set_name(edge.name);
        s_edge.set_quality(edge.span);
        s_edge.set_from(edge.from);
        s_edge.set_to(edge.to);
        s_edge.set_weight(edge.weight);
        *result.add_edge() = s_edge;
    }
    for (size_t i = 0; i < vertex_count; ++i) {
        serialization::Vertex vertex;
        for (const auto &edge_id: g.GetIncidentEdges(i)) {
            vertex.add_edge_id(edge_id);
        }
        *result.add_vertex() = vertex;
    }
    return result;
}

serialization::Router Serialize(const TRouting::TRouter &router) {
    serialization::Router result;
    *result.mutable_router_settings() = GetRouterSettingSerialize(router.GetBusSettings());
    *result.mutable_graph() = GetGraphSerialize(router.GetGraph());
    for (const auto &[n, id]: router.GetStopIds()) {
        serialization::StopId si;
        si.set_name(n);
        si.set_id(id);
        *result.add_stop_id() = si;
    }
    return result;
}

void
AddStopFromDB(TCatalogue::TransportCatalogue &transportCatalogue, const serialization::TransportCatalogue &database) {
    for (size_t i = 0; i < database.stop_size(); ++i) {
        const serialization::Stop &stop_i = database.stop(i);
        transportCatalogue.AddStop(stop_i.name(), {stop_i.coordinate(0), stop_i.coordinate(1)});
    }
}

void
AddBusFromDB(TCatalogue::TransportCatalogue &transportCatalogue, const serialization::TransportCatalogue &database) {
    for (size_t i = 0; i < database.bus_size(); ++i) {
        const serialization::Bus &bus_i = database.bus(i);
        std::vector<std::string> stops(bus_i.stop_size());
        for (size_t j = 0; j < stops.size(); ++j) {
            stops[j] = transportCatalogue.FindStop(bus_i.stop(j))->name;
        }
        transportCatalogue.AddBusFromDb(bus_i.name(), stops, bus_i.is_circle(),
                                        {static_cast<int>(bus_i.stops_count()), static_cast<int>(bus_i.unique_stops()),
                                         bus_i.road_lenght(), bus_i.curvature()});
    }
}

Json::Node ToNode(const serialization::Point &p) {
    return Json::Node(Json::Array{{p.x()},
                                  {p.y()}});
}

Json::Node ToNode(const serialization::Color &c) {
    if (!c.name().empty()) {
        return Json::Node(c.name());
    } else if (c.has_rgb()) {
        const serialization::RGB &rgb = c.rgb();
        return Json::Node(Json::Array{{rgb.red()},
                                      {rgb.green()},
                                      {rgb.blue()}});
    } else if (c.has_rgba()) {
        const serialization::RGBA &rgba = c.rgba();
        return Json::Node(Json::Array{{rgba.red()},
                                      {rgba.green()},
                                      {rgba.blue()},
                                      {rgba.opacity()}});
    } else
        return Json::Node("none"s);
}

Json::Node ToNode(const google::protobuf::RepeatedPtrField<serialization::Color> &cv) {
    Json::Array result;
    result.reserve(cv.size());
    for (const auto &c: cv) {
        result.emplace_back(ToNode(c));
    }
    return Json::Node(std::move(result));
}

Json::Node GetRenderSettingsFromDB(const serialization::TransportCatalogue &database) {
    const serialization::RenderSettings &rs = database.render_settings();
    return Json::Node(Json::Dict{
            {{"width"s},                {rs.width()}},
            {{"height"s},               {rs.height()}},
            {{"padding"s},              {rs.padding()}},
            {{"stop_radius"s},          {rs.stop_radius()}},
            {{"line_width"s},           {rs.line_width()}},
            {{"bus_label_font_size"s},  {rs.bus_label_font_size()}},
            {{"bus_label_offset"s},     ToNode(rs.bus_label_offset())},
            {{"stop_label_font_size"s}, {rs.stop_label_font_size()}},
            {{"stop_label_offset"s},    ToNode(rs.stop_label_offset())},
            {{"underlayer_color"s},     ToNode(rs.underlayer_color())},
            {{"underlayer_width"s},     {rs.underlayer_width()}},
            {{"color_palette"s},        ToNode(rs.color_palette())},
    });
}

Json::Node GetRouterSettingsFromDB(const serialization::Router &router) {
    const serialization::RouterSettings &rs = router.router_settings();
    return Json::Node(Json::Dict{
            {{"bus_wait_time"s}, {rs.bus_wait_time()}},
            {{"bus_velocity"s},  {rs.bus_velocity()}}
    });
}

graph::DirectedWeightedGraph<double> GetGraphFromDB(const serialization::Router &router) {
    const serialization::Graph &g = router.graph();
    std::vector<graph::Edge<double>> edges(g.edge_size());
    std::vector<std::vector<graph::EdgeId>> incidence_lists(g.vertex_size());
    for (size_t i = 0; i < edges.size(); ++i) {
        const serialization::Edge &edg = g.edge(i);
        edges[i] = {edg.name(), static_cast<size_t>(edg.quality()),
                    static_cast<size_t>(edg.from()), static_cast<size_t>(edg.to()), edg.weight()};
    }
    for (size_t i = 0; i < incidence_lists.size(); ++i) {
        const serialization::Vertex &v = g.vertex(i);
        incidence_lists[i].reserve(v.edge_id_size());
        for (const auto &id: v.edge_id()) {
            incidence_lists[i].push_back(id);
        }
    }
    return {edges, incidence_lists};
}

std::map<std::string, graph::VertexId> GetStopIdsFromDB(const serialization::Router &router) {
    std::map<std::string, graph::VertexId> result;
    for (const auto &s: router.stop_id()) {
        result[s.name()] = s.id();
    }
    return result;
}

std::tuple<TCatalogue::TransportCatalogue, Render::MapRenderer, TRouting::TRouter,
        graph::DirectedWeightedGraph<double>, std::map<std::string, graph::VertexId>>
Deserialize(std::istream &input) {
    serialization::TransportCatalogue database;
    database.ParseFromIstream(&input);
    TCatalogue::TransportCatalogue catalogue;
    Render::MapRenderer renderer(GetRenderSettingsFromDB(database));
    TRouting::TRouter router(GetRouterSettingsFromDB(database.router()));
    AddStopFromDB(catalogue, database);
    AddBusFromDB(catalogue, database);
    return {std::move(catalogue), std::move(renderer), std::move(router),
            GetGraphFromDB(database.router()),
            GetStopIdsFromDB(database.router())};
}