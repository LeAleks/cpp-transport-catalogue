#include "json_reader.h"
#include "json_builder.h"

#include <exception>
#include <string_view>
#include <deque>
#include <vector>
#include <sstream>

#include <iostream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {
using namespace std;


namespace details{

// Преобразование данных об остановке
transport_catalogue::Stop ParseStop(const json::Dict& request){
	transport_catalogue::Stop stop;
	stop.name_ = request.at("name"s).AsString();
	stop.location_.lat = request.at("latitude"s).AsDouble();
	stop.location_.lng = request.at("longitude"s).AsDouble();

	return stop;
}

// Преобразование данных о расстоянии
void ParseDistances(deque<transport_catalogue::StopsDistance>& distances, const json::Dict& request) {
	string_view this_stop_name = request.at("name"s).AsString();

	for (auto& other_stop : request.at("road_distances"s).AsDict()) {
		transport_catalogue::StopsDistance distance;

		distance.stop1_name_ = this_stop_name;
		distance.stop2_name_ = other_stop.first;
		distance.distance_ = other_stop.second.AsInt();

		distances.push_back(move(distance));
	}
}

// Преобразование даннх о маршруте
tuple<string_view, bool, vector<string_view>> ParceBus(const json::Dict& request) {
	// Создание массива названий остановок
	vector<string_view> stops;
	stops.reserve(request.at("stops"s).AsArray().size());

	// Перенос названий в массив
	for (auto& stop_name_ptr : request.at("stops"s).AsArray()) {
		stops.push_back(stop_name_ptr.AsString());
	}


	return { request.at("name"s).AsString(),
			 request.at("is_roundtrip"s).AsBool(),
			stops };
}

// Заполнение транспортного справочника
void FillCatalogue(transport_catalogue::TransportCatalogue& catalog, const json::Node& input_base) {
	// Проверка, что base_requests - Array
	if (!input_base.IsArray()) {
		throw invalid_argument("Base requests isn't an Array"s);
	}

	// Хранение узлов остановок
	deque<json::Node> stops_in_ptr;

	// Хранение узлов маршрутов
	deque<json::Node> buses_in_ptr;

	// Хранение расстояний между остановками
	deque<transport_catalogue::StopsDistance> distances;

	// Проход по массиву запросов для разбиения на группы
	for (auto& request_ptr : input_base.AsArray()) {
		// Выделение одного запроса
		auto& request_as_map = request_ptr.AsDict();

		if (request_as_map.at("type"s).AsString() == "Stop"s) {
			stops_in_ptr.push_back(request_ptr);
		}
		else {
			buses_in_ptr.push_back(request_ptr);
		}
	}

	// Проход по массиву остановок
	for (auto& stop_ptr : stops_in_ptr) {
		// Перевод данных в словарь
		auto& stop_in_dict = stop_ptr.AsDict();

		// Добавление остановки в базу
		catalog.AddStop(move(details::ParseStop(stop_in_dict)));

		// Выделение расстояний в отдельный массив
		details::ParseDistances(distances, stop_in_dict);
	}

	// Перенос расстояний в базу
	for (auto& distance : distances) {
		catalog.SetDistance(distance);
	}

	// Проход по массиву маршрутов
	for (auto& bus_ptr : buses_in_ptr) {
		auto [bus_name, circle, stop_names] = details::ParceBus(bus_ptr.AsDict());

		// Передача данных в базу
		catalog.AddBus(bus_name, circle, stop_names);
	}
}

// Создание документа с картой маршрутов
svg::Document RenderMap(
	transport_catalogue::TransportCatalogue& catalog,
	map_renderer::MapRenderer& renderer) {
	// Получение списка маршрутов из справочника
	const auto bus_list = catalog.GetBusList();

	// Передача данных и генерация документа с картой
	svg::Document bus_map = renderer.GenerateMap(bus_list);


	return bus_map;
}


// Создание узла с информацией об остановке
json::Node GenerateStop(
	transport_catalogue::TransportCatalogue& catalog, 
	const json::Dict& request_map) {

	// Узел для возврата
	json::Builder build_result{};

	build_result.StartDict();

	build_result.Key("request_id"s).Value(request_map.at("id"s).GetValue());

	// Запрашиваем остановку
	auto stop_ptr = catalog.FindStop(request_map.at("name"s).AsString());

	if (stop_ptr == nullptr) {
		build_result.Key("error_message"s).Value("not found"s);
	}
	else {
		build_result.Key("buses"s).StartArray();

		for (auto& bus : *stop_ptr) {
			build_result.Value(string(bus));
		}

		build_result.EndArray();
	}

	build_result.EndDict();

	return build_result.Build();
}

json::Node GenerateBus(
	transport_catalogue::TransportCatalogue& catalog,
	const json::Dict& request_map) {

	// Узел для возврата
	json::Builder result{};

	auto bus_info = catalog.GetBusInfo(request_map.at("name"s).AsString());

	if (bus_info.has_value()) {
		result
			.StartDict()
				.Key("request_id"s).Value(request_map.at("id"s).GetValue())
				.Key("curvature").Value(bus_info.value().curvature)
				.Key("route_length"s).Value(bus_info.value().route_length)
				.Key("stop_count"s).Value(bus_info.value().stop_count)
				.Key("unique_stop_count"s).Value(bus_info.value().unique_stop_count)
			.EndDict().Build();
	}
	else {
		result
			.StartDict()
				.Key("request_id"s).Value(request_map.at("id"s).GetValue())
				.Key("error_message"s).Value("not found"s)
			.EndDict().Build();
	}

	return result.Build();
}

json::Node GenerateMap(
	transport_catalogue::TransportCatalogue& catalog,
	map_renderer::MapRenderer& renderer,
	const json::Dict& request_map) {

	// Рендер карты в строку
	svg::Document doc = RenderMap(catalog, renderer);
	ostringstream strm;
	doc.Render(strm);

	// Узел для возврата
	json::Builder result{};

	result
		.StartDict()
			.Key("request_id"s).Value(request_map.at("id"s).GetValue())
			.Key("map"s).Value(strm.str())
		.EndDict();

	return result.Build();
}


void PrintRequest(
	transport_catalogue::TransportCatalogue& catalog,
	map_renderer::MapRenderer& renderer,
	const json::Node& input_base,
	std::ostream& output) {
	// Проверка, что stat_requests - Array
	if (!input_base.IsArray()) {
		throw invalid_argument("Base requests isn't an Array"s);
	}

	// Проверка, что запросы были
	if (input_base.AsArray().empty()) {
		return;
	}

	// Создаем корневой массив для вывода
	json::Array response_output;

	// Проходим по запросам
	for (auto& request_ptr : input_base.AsArray()) {
		auto& request_map = request_ptr.AsDict();

		// Выводим данные в зависимости от типа запроса
		if (request_map.at("type"s).AsString() == "Stop"s) {
			response_output.push_back(GenerateStop(catalog, request_map));
		}
		else if (request_map.at("type"s).AsString() == "Bus"s){
			response_output.push_back(GenerateBus(catalog, request_map));
		}
		else if (request_map.at("type"s).AsString() == "Map"s) {
			response_output.push_back(GenerateMap(catalog, renderer, request_map));
		}
		else {
			throw invalid_argument("unknown request"s);
		}
	}

	json::Print(json::Document(json::Builder{}.Value(response_output).Build()), output);

}


// Декодирование цвета из json формата
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

// Перенос данных из json в map_render
void ParseRenderSettings(map_renderer::MapRenderer& renderer, const json::Node& input_base) {
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

	// Перенос параметров в отрисовщик
	renderer.SetSettings(settings);
}


} // End of details


// Вход в функцию работы с json-файлами
void ReadJson(transport_catalogue::TransportCatalogue& catalog,
	map_renderer::MapRenderer& renderer,
	std::istream& input,
	std::ostream& output) {

	// JSON база данных
	const auto input_database = json::Load(input);

	// { - Dict
	// [ - Array

	// Проверка, что корневой узел - словарь
	if (!input_database.GetRoot().IsDict()) {
		throw invalid_argument("Root isn't a Dict");
	}


	// Проверка, что есть запросы типа base_request
	if (input_database.GetRoot().AsDict().count("base_requests"s) != 0) {

		// Указатель на массив с base_requests
		auto& base_requests_ptr = input_database.GetRoot().AsDict().at("base_requests"s);

		// Обработка запросов на внесение информации
		details::FillCatalogue(catalog, base_requests_ptr);
	}

	// Проверка, что есть запросы типа render_settings
	if (input_database.GetRoot().AsDict().count("render_settings"s) != 0) {
		// Указатель на массив с render_settings
		auto& render_settings_ptr = input_database.GetRoot().AsDict().at("render_settings"s);

		// Перенос параметров отрисовки в map_render
		details::ParseRenderSettings(renderer, render_settings_ptr);
	}


	// Проверка, что есть запросы типа stat_requests
	if (input_database.GetRoot().AsDict().count("stat_requests"s) != 0) {
		// Указатель на массив с stat_requests
		auto& stat_requests_ptr = input_database.GetRoot().AsDict().at("stat_requests"s);

		// Вывод данных в формате json
		details::PrintRequest(catalog, renderer, stat_requests_ptr, output);
	}

	//svg::Document doc = details::RenderMap(catalog, renderer);
	//doc.Render(output);
}


}