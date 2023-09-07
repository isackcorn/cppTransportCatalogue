#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

using namespace std::literals;

void PrintUsage(std::ostream &stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        JsonReader input(Json::Load(std::cin));
        TCatalogue::TransportCatalogue transportCatalogue;
        input.FillCatalogue(transportCatalogue);
        Render::MapRenderer renderer(input.ProcessRenderSettings());
        TRouting::TRouter router(JsonReader::FillRouting(input.ProcessRoutingSettings()), transportCatalogue);
        std::ofstream fout(input.ProcessSerializationSettings().AsDict().at("file"s).AsString(), std::ios::binary);
        if (fout.is_open()) {
            Serialize(transportCatalogue, renderer, router, fout);
        }
    } else if (mode == "process_requests"sv) {
        JsonReader input_json(Json::Load(std::cin));
        std::ifstream database(input_json.ProcessSerializationSettings().AsDict().at("file"s).AsString(),
                               std::ios::binary);
        if (database) {
            auto [transportcatalogue, renderer, router, graph, stop_ids] = Deserialize(database);
            router.SetGraph(std::move(graph), std::move(stop_ids));
            RequestHandler handler(transportcatalogue, renderer, router);
            input_json.ReadJson(input_json.ProcessStatRequests(), handler);
        }
    } else {
        PrintUsage();
        return 1;
    }
}