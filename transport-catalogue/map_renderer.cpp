#include "map_renderer.h"




/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */


namespace map_renderer {

using namespace std;

// ---------- Преобразование координат ----------
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}


// ---------- MapRenderer ----------

// Применить настройки извне
void MapRenderer::SetSettings(RenderSettings& settings) {
    settings_ = settings;
}

svg::Document MapRenderer::GenerateMap(
    const std::vector<const transport_catalogue::Bus*>& bus_list) const {
    // Хранилище на указатели координат
    std::deque<geo::Coordinates> coordinates_list;

    // Получение всех координат из  всех маршрутов
    for (auto& bus : bus_list) {
        for (auto& stop : bus->stops_) {
            coordinates_list.push_back(stop->location_);
        }
    }

    // Создаем преобразователь координат
    SphereProjector map_converter(
        coordinates_list.begin(),
        coordinates_list.end(),
        settings_.width,
        settings_.height,
        settings_.padding);

    // Документ для вывода
    svg::Document result;

    // Хранилище линий маршрутов
    deque<svg::Polyline> bus_lines;

    // Хранилище названий маршрутов
    deque<svg::Text> bus_names;

    // Хранилище информации об остановках
    map<string_view, StopImage> stop_marks;

    // Счетчик маршрутов для определения цвета маршрута
    size_t color_counter = 0;

    // Проходим по списку маршрутов для отрисовки линий
    for (auto& bus : bus_list) {
        // Если на маршруте нет остановок, то пропускаем
        if (bus->stops_.empty()) {
            continue;
        }
        // Добавление линии маршрута
        bus_lines.push_back(CreatePolyline(map_converter, bus, color_counter));

        // Добавление названя маршрута (первой остановки)
        auto [bus_name_bg, bus_name_fg] = CreateBusName(map_converter, bus, color_counter);
        bus_names.push_back(bus_name_bg);
        bus_names.push_back(bus_name_fg);

        // Если маршрут не кольцевой, и начало и конец не совпадают, то добавляем еще 
        // одно название маршрута для конечной остановки
        if (!bus->is_circle_ && (bus->stops_[0] != bus->stops_[bus->stops_.size() - 1])) {
            bus_name_bg.SetPosition(map_converter(bus->stops_[bus->stops_.size() - 1]->location_));
            bus_name_fg.SetPosition(map_converter(bus->stops_[bus->stops_.size() - 1]->location_));

            bus_names.push_back(bus_name_bg);
            bus_names.push_back(bus_name_fg);
        }

        // Проходим по списку остановок маршрута
        for (const transport_catalogue::Stop* stop_ptr : bus->stops_) {
            // Если остановка уже внесена, то пропускаем
            if (stop_marks.count(stop_ptr->name_)) {
                continue;
            }

            // Переносим остановку в хранилище
            stop_marks[stop_ptr->name_] = CreateStopImage(map_converter, stop_ptr);

        }

        // Меняем шаг цвета
        ++color_counter;
        if (color_counter >= settings_.color_pallete.size()) {
            color_counter = 0;
        }
    }

    // Объединяем данные в один документ

    for (auto& line : bus_lines) {
        result.Add(line);
    }

    for (auto& bus_name : bus_names) {
        result.Add(bus_name);
    }

    for (auto& stop : stop_marks) {
        result.Add(stop.second.stop_mark);
    }

    for (auto& stop : stop_marks) {
        result.Add(stop.second.stop_name.first);
        result.Add(stop.second.stop_name.second);
    }

    return result;
}



svg::Polyline MapRenderer::CreatePolyline(
    const SphereProjector& map_converter,
    const transport_catalogue::Bus* bus,
    size_t color_counter) const {

    // Линия для отрисовки
    svg::Polyline route;

    // Определение цвета линии
    route.SetStrokeColor(settings_.color_pallete[color_counter]);

    route.SetFillColor(svg::NoneColor);
    route.SetStrokeWidth(settings_.line_width);
    route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    // В маршруте проходим по списку остановок и добавляем точки в линию
    for (auto& stop : bus->stops_) {
        route.AddPoint(map_converter(stop->location_));
    }

    // Если маршрут не кольцевой, и имеет более 1 остановки, то проходим
    // в обратном порядке без последней в списке остановки
    if (!bus->is_circle_ && bus->stops_.size() > 1) {
        for (int i = bus->stops_.size() - 2; i >= 0; --i) {
            route.AddPoint(map_converter(bus->stops_[i]->location_));
        }
    }

    return route;
}

pair<svg::Text, svg::Text> MapRenderer::CreateBusName(
    const SphereProjector& map_converter,
    const transport_catalogue::Bus* bus,
    size_t color_counter) const {

    // Определение базовых параметров текста названия маршрута
    svg::Text bus_name_base =
        svg::Text()
        .SetFontFamily("Verdana"s)
        .SetFontSize(settings_.bus_label_font_size)
        .SetPosition(map_converter(bus->stops_[0]->location_))
        .SetOffset({ settings_.bus_label_dx, settings_.bus_label_dy })
        .SetFontWeight("bold"s)
        .SetData(bus->name_);

    return {
        svg::Text{ bus_name_base }
        .SetStrokeColor(settings_.underlayer_color)
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeWidth(settings_.underlayer_width),
        svg::Text{ bus_name_base }
        .SetFillColor(settings_.color_pallete[color_counter]) };
}

StopImage MapRenderer::CreateStopImage(
    const SphereProjector& map_converter,
    const transport_catalogue::Stop* stop_ptr) const {

    StopImage stop_image;

    // Задаем параметры круга
    stop_image.stop_mark
        .SetCenter(map_converter(stop_ptr->location_))
        .SetRadius(settings_.stop_radius)
        .SetFillColor("white"s);

    // Задаем базовые параметры названия
        // Определение базовых параметров текста названия маршрута
    svg::Text stop_name_base =
        svg::Text()
        .SetFontFamily("Verdana"s)
        .SetFontSize(settings_.stop_label_font_size)
        .SetPosition(map_converter(stop_ptr->location_))
        .SetOffset({ settings_.stop_label_dx, settings_.stop_label_dy })
        .SetData(stop_ptr->name_);

    stop_image.stop_name.first =
        svg::Text{ stop_name_base }
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    stop_image.stop_name.second =
        svg::Text{ stop_name_base }
    .SetFillColor("black"s);

    return stop_image;
}


} // End of map_renderer