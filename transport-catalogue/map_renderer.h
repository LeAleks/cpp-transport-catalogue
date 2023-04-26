#pragma once

#include "svg.h"
#include "geo.h"
#include "domain.h"


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <string_view>
#include <deque>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace map_renderer {

// ---------- Данные для отрисовки ----------

struct RenderSettings {
    // Размеры изображения
    double width = 0.0;
    double height = 0.0;

    // Величина отступа от края изображения
    double padding = 0;

    // Радиус метки остановки
    double stop_radius = 0.0;

    // толщина линий, которыми рисуются автобусные маршруты
    double line_width = 0.0;

    // Размер названия маршрута
    int bus_label_font_size = 0;

    // Cмещение надписи с названием маршрута относительно координат конечной остановки на карте
    double bus_label_dx = 0.0;
    double bus_label_dy = 0.0;


    // Размер названия остановки
    int stop_label_font_size = 0;

    // Смещение названия остановки относительно её координат на карте
    double stop_label_dx = 0.0;
    double stop_label_dy = 0.0;

    // Цвет подложки
    svg::Color underlayer_color;

    // Толщина подложки под названиями остановок и маршрутов.
    // Задаёт значение атрибута stroke-width элемента <text>
    double underlayer_width = 0.0;

    // Цвета в палитре
    std::vector<svg::Color> color_pallete;
};


// Структура для хранения информации об остановке для вывода
struct StopImage {
    svg::Circle stop_mark;
    std::pair<svg::Text, svg::Text> stop_name;
};

// ---------- Преобразование координат ----------

inline const double EPSILON = 1e-6;

bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


// --------- Отрисовка карты маршрутов ----------

class MapRenderer {
public:
    MapRenderer() = default;

    MapRenderer(RenderSettings& settings)
        : settings_(settings) {}

    MapRenderer(MapRenderer& other) {
        settings_ = other.settings_;
    }

    // Применить настройки извне
    void SetSettings(RenderSettings& settings);

    svg::Document GenerateMap(const std::vector<const transport_catalogue::Bus*>& bus_list) const;

private:
    RenderSettings settings_;

    svg::Polyline CreatePolyline(
        const SphereProjector& map_converter,
        const transport_catalogue::Bus* bus,
        size_t color_counter) const;

    std::pair<svg::Text, svg::Text> CreateBusName(
        const SphereProjector& map_converter,
        const transport_catalogue::Bus* bus,
        size_t color_counter) const;

    StopImage CreateStopImage(
        const SphereProjector& map_converter,
        const transport_catalogue::Stop* stop_ptr) const;
};


} // End of map_renderer