#include "json_reader.h"
#include "json_builder.h"

#include <exception>
#include <string_view>
#include <deque>
#include <vector>
#include <sstream>

#include <iostream>

namespace json_reader {

using namespace std;

namespace details {

svg::Color ParseColor(const json::Node& color_ptr) {
	if (color_ptr.IsString()) {
		// Если цвет задан строкой, то присваивает строку
		return color_ptr.AsString();
	}
	else {
		auto& color_code = color_ptr.AsArray();

		if (color_code.size() == 3) {
			return svg::Rgb{
				uint8_t(color_code[0].AsInt()),
				uint8_t(color_code[1].AsInt()),
				uint8_t(color_code[2].AsInt()) };
		}
		else if (color_code.size() == 4) {
			return svg::Rgba{
				uint8_t(color_code[0].AsInt()),
				uint8_t(color_code[1].AsInt()),
				uint8_t(color_code[2].AsInt()),
				color_code[3].AsDouble() };
		}
		else {
			throw invalid_argument("Incorrect color code"s);
		}
	}
}

deque<transport_catalogue::StopToAdd> GetStopsList(const json::Array& requests) {
	using namespace transport_catalogue;

	deque<StopToAdd> result;

	for (auto& request : requests) {
		if (request.AsDict().at("type"s).AsString() == "Stop"s) {
			StopToAdd stop;

			stop.name = request.AsDict().at("name"s).AsString();
			stop.location.lat = request.AsDict().at("latitude"s).AsDouble();
			stop.location.lng = request.AsDict().at("longitude"s).AsDouble();

			const json::Dict& stops_distances = request.AsDict().at("road_distances"s).AsDict();

			for (const auto& [stop_name, distance] : stops_distances) {
				stop.distances_to_stops.emplace_back(stop_name, distance.AsInt());
			}

			result.push_back(move(stop));
		}
	}

	return result;
}

deque<transport_catalogue::BusToAdd> GetBusesList(const json::Array& requests) {
	using namespace transport_catalogue;

	deque<BusToAdd> result;

	for (auto& request : requests) {
		if (request.AsDict().at("type"s).AsString() == "Bus"s) {
			BusToAdd bus;

			bus.name = request.AsDict().at("name"s).AsString();
			bus.is_circle = request.AsDict().at("is_roundtrip"s).AsBool();

			const json::Array& stops_list = request.AsDict().at("stops"s).AsArray();

			for (auto& stop : stops_list) {
				bus.stops.push_back(stop.AsString());
			}

			result.push_back(move(bus));
		}
	}

	return result;
}

map_renderer::RenderSettings GetRenderSettings(const json::Node& input_base) {
	map_renderer::RenderSettings settings;

	// Словарь с данными рендера
	auto& set_map = input_base.AsDict();

	// размеры поля
	settings.width = set_map.at("width"s).AsDouble();
	settings.height = set_map.at("height"s).AsDouble();

	// отступ
	settings.padding = set_map.at("padding"s).AsDouble();

	// Знак остановки
	settings.stop_radius = set_map.at("stop_radius"s).AsDouble();

	// Толщина линий маршрутов
	settings.line_width = set_map.at("line_width"s).AsDouble();

	// Отображение названия маршрута
	settings.bus_label_font_size = set_map.at("bus_label_font_size"s).AsInt();

	// Положение названия маршрута
	auto& bus_name_loc = set_map.at("bus_label_offset"s).AsArray();
	settings.bus_label_dx = bus_name_loc[0].AsDouble();
	settings.bus_label_dy = bus_name_loc[1].AsDouble();

	// Отображение названия остановки
	settings.stop_label_font_size = set_map.at("stop_label_font_size"s).AsInt();

	// Положение названия остановки
	auto& stop_name_loc = set_map.at("stop_label_offset"s).AsArray();
	settings.stop_label_dx = stop_name_loc[0].AsDouble();
	settings.stop_label_dy = stop_name_loc[1].AsDouble();

	// Цвет подложки
	auto& underlayer_color_ptr = set_map.at("underlayer_color"s);
	settings.underlayer_color = ParseColor(underlayer_color_ptr);

	// Толщина подложки под названиями остановок и маршрутов
	settings.underlayer_width = set_map.at("underlayer_width"s).AsDouble();

	// Набор цветов для отрисовки
	auto& color_pallet_array = set_map.at("color_palette"s).AsArray();
	settings.color_pallete.reserve(color_pallet_array.size());
	for (auto& color : color_pallet_array) {
		settings.color_pallete.push_back(ParseColor(color));
	}

	return settings;
}

deque<transport_catalogue::RequestInfo> GetRequestsList(const json::Array& requests) {
	using namespace transport_catalogue;

	deque<RequestInfo> result;

	for (auto& request_json : requests) {
		RequestInfo request;

		for (const auto& [key, item] : request_json.AsDict()) {
			if (key == "id"s) {
				request.id = item.AsInt();
			}
			else {
				request.request_items[key] = item.AsString();
			}
		}

		result.push_back(move(request));
	}

	return result;
}

} // End of details

// Чтение json для транспортного каталога
InputQueries ReadTransportJson(std::istream& input) {
	
	InputQueries result;

	// JSON база данных
	const auto input_database = json::Load(input);

	// { - Dict
	// [ - Array

	// Проверка, что корневой узел - словарь
	if (!input_database.GetRoot().IsDict()) {
		throw invalid_argument("Root isn't a Dict");
	}

	const json::Dict& command_list = input_database.GetRoot().AsDict();
	
	// Проверка, что есть запросы типа base_request
	if (command_list.count("base_requests"s) != 0) {
		auto& base_requests_array = command_list.at("base_requests"s).AsArray();

		result.stops_to_add = details::GetStopsList(base_requests_array);
		result.buses_to_add = details::GetBusesList(base_requests_array);
	}

	// Проверка, что есть запросы типа render_settings
	if (command_list.count("render_settings"s) != 0) {
		// Указатель на массив с render_settings
		auto& render_settings_ptr = command_list.at("render_settings"s);

		// Парсинг парметров
		result.render_settings = details::GetRenderSettings(render_settings_ptr);
	}

	// Проверка, что есть запросы типа routing_settings
	if (command_list.count("routing_settings"s) != 0) {
		// Указатель на массив с route_settings
		auto& route_settings_dict = command_list.at("routing_settings"s).AsDict();

		// Парсинг парметров
		result.routing_settings.bus_velocity = route_settings_dict.at("bus_velocity"s).AsDouble();
		result.routing_settings.bus_wait_time = route_settings_dict.at("bus_wait_time"s).AsInt();
	}

	// Проверка, что есть запросы типа stat_requests
	if (command_list.count("stat_requests"s) != 0) {
		// Указатель на массив с stat_requests
		auto& stat_requests_array = command_list.at("stat_requests"s).AsArray();

		result.requests = details::GetRequestsList(stat_requests_array);

		// Вывод данных в формате json
	}
	
	return result;
}

} // End of json_reader