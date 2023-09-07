#include "map_renderer.h"

namespace Render {

    std::vector<Svg::Polyline>
    MapRenderer::ParseBusLines(const std::map<std::string_view, const TCatalogue::Bus *> &buses,
                               const Visualization &visualization) const {
        std::vector<Svg::Polyline> result;
        size_t color = 0;
        for (const auto &[bus_number, bus]: buses) {
            if (bus->stops.empty()) continue;
            std::vector<const TCatalogue::Stop *> route_stops{bus->stops.begin(), bus->stops.end()};
            if (!bus->is_loop) route_stops.insert(route_stops.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
            Svg::Polyline line;
            for (const auto &stop: route_stops) {
                line.AddPoint(visualization(stop->coordinates));
            }
            line.SetStrokeColor(color_palette[color]);
            line.SetFillColor("none");
            line.SetStrokeWidth(line_width);
            line.SetStrokeLineCap(Svg::StrokeLineCap::ROUND);
            line.SetStrokeLineJoin(Svg::StrokeLineJoin::ROUND);

            if (color < (color_palette.size() - 1)) {
                ++color;
            } else color = 0;

            result.push_back(line);
        }

        return result;
    }

    std::vector<Svg::Text> MapRenderer::ParseBusLabel(const std::map<std::string_view, const TCatalogue::Bus *> &buses,
                                                      const Visualization &visualization) const {
        std::vector<Svg::Text> result;
        size_t color_num = 0;
        for (const auto &[bus_number, bus]: buses) {
            if (bus->stops.empty()) continue;
            Svg::Text text;
            Svg::Text underlayer;
            text.SetPosition(visualization(bus->stops[0]->coordinates));
            text.SetOffset(bus_label_offset);
            text.SetFontSize(bus_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetFontWeight("bold");
            text.SetData(bus->name);
            text.SetFillColor(color_palette[color_num]);
            if (color_num < (color_palette.size() - 1)) ++color_num;
            else color_num = 0;

            underlayer.SetPosition(visualization(bus->stops[0]->coordinates));
            underlayer.SetOffset(bus_label_offset);
            underlayer.SetFontSize(bus_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetFontWeight("bold");
            underlayer.SetData(bus->name);
            underlayer.SetFillColor(underlayer_color);
            underlayer.SetStrokeColor(underlayer_color);
            underlayer.SetStrokeWidth(underlayer_width);
            underlayer.SetStrokeLineCap(Svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(Svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);

            if (!bus->is_loop && bus->stops[0] != bus->stops[bus->stops.size() - 1]) {
                Svg::Text text2{text};
                Svg::Text underlayer2{underlayer};
                text2.SetPosition(visualization(bus->stops[bus->stops.size() - 1]->coordinates));
                underlayer2.SetPosition(visualization(bus->stops[bus->stops.size() - 1]->coordinates));

                result.push_back(underlayer2);
                result.push_back(text2);
            }
        }

        return result;
    }

    std::vector<Svg::Circle>
    MapRenderer::ParseStopsSymbols(const std::map<std::string_view, const TCatalogue::Stop *> &stops,
                                   const Visualization &visualization) const {
        std::vector<Svg::Circle> result;
        for (const auto &[stop_name, stop]: stops) {
            Svg::Circle symbol;
            symbol.SetCenter(visualization(stop->coordinates));
            symbol.SetRadius(stop_radius);
            symbol.SetFillColor("white");

            result.push_back(symbol);
        }

        return result;
    }

    std::vector<Svg::Text>
    MapRenderer::ParseStopsLabels(const std::map<std::string_view, const TCatalogue::Stop *> &stops,
                                  const Visualization &visualization) const {
        std::vector<Svg::Text> result;
        Svg::Text text;
        Svg::Text underlayer;
        for (const auto &[stop_name, stop]: stops) {
            text.SetPosition(visualization(stop->coordinates));
            text.SetOffset(stop_label_offset);
            text.SetFontSize(stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name);
            text.SetFillColor("black");

            underlayer.SetPosition(visualization(stop->coordinates));
            underlayer.SetOffset(stop_label_offset);
            underlayer.SetFontSize(stop_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetData(stop->name);
            underlayer.SetFillColor(underlayer_color);
            underlayer.SetStrokeColor(underlayer_color);
            underlayer.SetStrokeWidth(underlayer_width);
            underlayer.SetStrokeLineCap(Svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(Svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);
        }

        return result;
    }

    Svg::Document MapRenderer::ParseSvg(const std::map<std::string_view, const TCatalogue::Bus *> &buses) const {
        Svg::Document result;
        std::vector<Geo::Coordinates> route_stops_coord;
        std::map<std::string_view, const TCatalogue::Stop *> all_stops;

        for (const auto &[bus_number, bus]: buses) {
            if (bus->stops.empty()) continue;
            for (const auto &stop: bus->stops) {
                route_stops_coord.push_back(stop->coordinates);
                all_stops[stop->name] = stop;
            }
        }
        Visualization visualization(route_stops_coord.begin(), route_stops_coord.end(), width,
                                    height, padding);

        for (const auto &line: ParseBusLines(buses, visualization)) result.Add(line);
        for (const auto &text: ParseBusLabel(buses, visualization)) result.Add(text);
        for (const auto &circle: ParseStopsSymbols(all_stops, visualization)) result.Add(circle);
        for (const auto &text: ParseStopsLabels(all_stops, visualization)) result.Add(text);

        return result;
    }

    MapRenderer::MapRenderer(const Json::Node &render_settings) {
        if (render_settings.IsNull()) return;
        const Json::Dict &request_map = render_settings.AsDict();

        height = request_map.at("height").AsDouble();
        padding = request_map.at("padding").AsDouble();
        stop_radius = request_map.at("stop_radius").AsDouble();
        width = request_map.at("width").AsDouble();
        line_width = request_map.at("line_width").AsDouble();
        bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
        const Json::Array &busLabelOffset = request_map.at("bus_label_offset").AsArray();
        bus_label_offset = {busLabelOffset[0].AsDouble(), busLabelOffset[1].AsDouble()};
        stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
        const Json::Array &stopLabelOffset = request_map.at("stop_label_offset").AsArray();
        stop_label_offset = {stopLabelOffset[0].AsDouble(), stopLabelOffset[1].AsDouble()};

        if (request_map.at("underlayer_color").IsString())
            underlayer_color = request_map.at("underlayer_color").AsString();
        else if (request_map.at("underlayer_color").IsArray()) {

            const Json::Array &underlayercolor = request_map.at("underlayer_color").AsArray();
            if (underlayercolor.size() == 3) {
                underlayer_color = Svg::Rgb(underlayercolor[0].AsInt(), underlayercolor[1].AsInt(),
                                            underlayercolor[2].AsInt());
            } else if (underlayercolor.size() == 4) {
                underlayer_color = Svg::Rgba(underlayercolor[0].AsInt(), underlayercolor[1].AsInt(),
                                             underlayercolor[2].AsInt(),
                                             underlayercolor[3].AsDouble());
            } else {
                throw std::logic_error("wrong underlayer colortype");
            }
        } else {
            throw std::logic_error("wrong underlayer color");
        }
        underlayer_width = request_map.at("underlayer_width").AsDouble();

        const Json::Array &palette = request_map.at("color_palette").AsArray();
        for (const auto &color_element: palette) {
            if (color_element.IsString()) color_palette.emplace_back(color_element.AsString());
            else if (color_element.IsArray()) {
                const Json::Array &colorType = color_element.AsArray();
                if (colorType.size() == 3) {
                    color_palette.emplace_back(
                            Svg::Rgb(colorType[0].AsInt(), colorType[1].AsInt(), colorType[2].AsInt()));
                } else if (colorType.size() == 4) {
                    color_palette.emplace_back(
                            Svg::Rgba(colorType[0].AsInt(), colorType[1].AsInt(), colorType[2].AsInt(),
                                      colorType[3].AsDouble()));
                } else {
                    throw std::logic_error("wrong color_palette type");
                }
            } else {
                throw std::logic_error("wrong color_palette");
            }
        }
    }

    Json::Node MapRenderer::ConvertPointToNode(const Svg::Point &point) {
        return Json::Node(Json::Array{{point.x},
                                      {point.y}});
    }

    Json::Node MapRenderer::ConvertColorToNode(const Svg::Color &color) {
        if (std::holds_alternative<std::string>(color)) {
            return {std::get<std::string>(color)};
        } else if (std::holds_alternative<Svg::Rgb>(color)) {
            const auto &rgb = std::get<Svg::Rgb>(color);
            return Json::Node(Json::Array{{rgb.red},
                                          {rgb.green},
                                          {rgb.blue}});
        } else if (std::holds_alternative<Svg::Rgba>(color)) {
            const auto &rgba = std::get<Svg::Rgba>(color);
            return Json::Node(Json::Array{{rgba.red},
                                          {rgba.green},
                                          {rgba.blue},
                                          {rgba.opacity}});
        } else
            return Json::Node("none");
    }

    Json::Node MapRenderer::ConvertVecColorToNode(const std::vector<Svg::Color> &colorVec) {
        Json::Array result;
        result.reserve(colorVec.size());
        for (const auto &c: colorVec) {
            result.emplace_back(ConvertColorToNode(c));
        }
        return {std::move(result)};
    }

    Json::Node MapRenderer::GetRenderSetup() const {
        return Json::Node(Json::Dict{
                {{"width"},                {width}},
                {{"height"},               {height}},
                {{"padding"},              {padding}},
                {{"stop_radius"},          {stop_radius}},
                {{"line_width"},           {line_width}},
                {{"bus_label_font_size"},  {bus_label_font_size}},
                {{"bus_label_offset"},     ConvertPointToNode(bus_label_offset)},
                {{"stop_label_font_size"}, {stop_label_font_size}},
                {{"stop_label_offset"},    ConvertPointToNode(stop_label_offset)},
                {{"underlayer_color"},     ConvertColorToNode(underlayer_color)},
                {{"underlayer_width"},     {underlayer_width}},
                {{"color_palette"},        ConvertVecColorToNode(color_palette)},
        });
    }
}