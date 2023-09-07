#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

void Serialize(const TCatalogue::TransportCatalogue &TCatalog,
               const Render::MapRenderer &renderer,
               const TRouting::TRouter &router,
               std::ostream &output);

serialization::Stop Serialize(const TCatalogue::TransportCatalogue &transportCatalogue,
                          const TCatalogue::Stop *stop);

serialization::Bus Serialize(const TCatalogue::Bus *bus, const TCatalogue::TransportCatalogue &transportCatalogue);

serialization::RenderSettings GetRenderSettingSerialize(const Json::Node &render_settings);

serialization::RouterSettings GetRouterSettingSerialize(const Json::Node &router_settings);

serialization::Router Serialize(const TRouting::TRouter &router);

std::tuple<TCatalogue::TransportCatalogue, Render::MapRenderer, TRouting::TRouter,
        graph::DirectedWeightedGraph<double>, std::map<std::string, graph::VertexId>> Deserialize(std::istream &input);