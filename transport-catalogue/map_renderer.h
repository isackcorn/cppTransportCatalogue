/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#pragma once

#include "svg.h"
#include "geo.h"
#include "json.h"
#include "domain.h"

#include <algorithm>

namespace Render {

    class Visualization {
    public:
        template<typename Container>
        Visualization(Container begin, Container end,
                      double max_width, double max_height, double padding)
                : padding_(padding) //
        {
            if (begin == end) {
                return;
            }

            const auto [leftIt, rightIt] = std::minmax_element(begin, end,
                                                               [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = leftIt->lng;
            const double max_lon = rightIt->lng;

            const auto [bottomIt, topIt] = std::minmax_element(begin, end,
                                                               [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottomIt->lat;
            max_lat_ = topIt->lat;

            std::optional<double> width_zoom;
            if (std::abs(max_lon - min_lon_) >= 1e-6) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (std::abs(max_lat_ - min_lat) >= 1e-6) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                zoom_ = *width_zoom;
            } else if (height_zoom) {
                zoom_ = *height_zoom;
            }
        }

        Svg::Point operator()(Geo::Coordinates coords) const {
            return {
                    (coords.lng - min_lon_) * zoom_ + padding_,
                    (max_lat_ - coords.lat) * zoom_ + padding_
            };
        }


    private:
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_ = 0;
        double padding_;
    };

    class MapRenderer {
    public:

        MapRenderer(const Json::Node &render_settings);

        [[nodiscard]] std::vector<Svg::Polyline>
        ParseBusLines(const std::map<std::string_view, const TCatalogue::Bus *> &buses,
                      const Visualization &sp) const;

        [[nodiscard]] std::vector<Svg::Text>
        ParseBusLabel(const std::map<std::string_view, const TCatalogue::Bus *> &buses, const Visualization &sp) const;

        [[nodiscard]] std::vector<Svg::Circle>
        ParseStopsSymbols(const std::map<std::string_view, const TCatalogue::Stop *> &stops,
                          const Visualization &sp) const;

        [[nodiscard]] std::vector<Svg::Text>
        ParseStopsLabels(const std::map<std::string_view, const TCatalogue::Stop *> &stops,
                         const Visualization &sp) const;

        [[nodiscard]] Svg::Document ParseSvg(const std::map<std::string_view, const TCatalogue::Bus *> &buses) const;

        Json::Node GetRenderSetup() const;

        static Json::Node ConvertPointToNode(const Svg::Point &point);

        static Json::Node ConvertColorToNode(const Svg::Color &color);

        static Json::Node ConvertVecColorToNode(const std::vector<Svg::Color> &colorVec);

    private:
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double stop_radius = 0.0;
        double line_width = 0.0;
        int bus_label_font_size = 0;
        Svg::Point bus_label_offset = {0.0, 0.0};
        int stop_label_font_size = 0;
        Svg::Point stop_label_offset = {0.0, 0.0};
        Svg::Color underlayer_color = {Svg::NoneColor};
        double underlayer_width = 0.0;
        std::vector<Svg::Color> color_palette{};

    };

}