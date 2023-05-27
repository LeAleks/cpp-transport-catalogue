#include "serialization.h"

#include <transport_catalogue.pb.h>

#include <variant>

#include <fstream>
#include <filesystem>
#include <iostream>

#include "svg.h"

namespace serialization {

namespace details {

serialization::Color SerialiseColor(const svg::Color& color) {
	serialization::Color result;

	if (std::holds_alternative<std::monostate>(color)) {
		result.set_color_type(1);
	}
	else if (std::holds_alternative<svg::Rgb>(color)) {
		svg::Rgb rgb = std::get<svg::Rgb>(color);
		// Нумерация типа цвета согласно его позиции в proto
		result.set_color_type(3);
		result.mutable_rgb_color()->set_r(rgb.red);
		result.mutable_rgb_color()->set_g(rgb.green);
		result.mutable_rgb_color()->set_b(rgb.blue);
	}
	else if (std::holds_alternative<svg::Rgba>(color)) {
		svg::Rgba rgba = std::get<svg::Rgba>(color);
		result.set_color_type(4);
		result.mutable_rgba_color()->set_r(rgba.red);
		result.mutable_rgba_color()->set_g(rgba.green);
		result.mutable_rgba_color()->set_b(rgba.blue);
		result.mutable_rgba_color()->set_a(rgba.opacity);
	}
	else if (std::holds_alternative<std::string>(color)) {
		std::string name = std::get<std::string>(color);
		result.set_color_type(2);
		result.set_str_color(name);
	}
	return result;
}

serialization::Stop SerialiseStop(const transport_catalogue::StopToAdd& stop_add) {
	serialization::Stop result;

	result.set_stop_name(stop_add.name);
	result.mutable_coordinates()->set_lat(stop_add.location.lat);
	result.mutable_coordinates()->set_lng(stop_add.location.lng);

	for (auto& distance : stop_add.distances_to_stops) {
		auto to_stop_ptr = result.add_distance();

		to_stop_ptr->set_stop_name_to(distance.first);
		to_stop_ptr->set_distance(distance.second);
	}

	return result;
}

serialization::Bus SerialiseBus(const transport_catalogue::BusToAdd& bus_add) {
	serialization::Bus result;

	result.set_is_round(bus_add.is_circle);
	result.set_bus_name(bus_add.name);

	for (auto& stop : bus_add.stops) {
		result.add_stop_names(stop);
	}

	return result;
}

serialization::RenderSettings SerialiseRenderSettings(const map_renderer::RenderSettings& settings) {
	serialization::RenderSettings result;

	result.set_width(settings.width);
	result.set_height(settings.height);
	result.set_padding(settings.padding);
	result.set_stop_radius(settings.stop_radius);
	result.set_line_width(settings.line_width);
	result.set_bus_label_font_size(settings.bus_label_font_size);
	result.set_stop_label_font_size(settings.stop_label_font_size);
	result.set_underlayer_width(settings.underlayer_width);

	result.mutable_bus_label()->set_dx(settings.bus_label_dx);
	result.mutable_bus_label()->set_dy(settings.bus_label_dy);
	result.mutable_stop_label()->set_dx(settings.stop_label_dx);
	result.mutable_stop_label()->set_dy(settings.stop_label_dy);

	*result.mutable_underlayer_color() = details::SerialiseColor(settings.underlayer_color);
	for (auto& color : settings.color_pallete) {
		*result.add_color_pallete() = details::SerialiseColor(color);
	}

	return result;
}

svg::Color DeserialiseColor(const serialization::Color& color) {
	svg::Color result;
	switch (color.color_type())
	{
	case 1:
	{
		return std::monostate();
		break;
	}

	case 2:
	{
		return color.str_color();
		break;
	}
	case 3:
	{
		svg::Rgb rgb;
		rgb.red = color.rgb_color().r();
		rgb.green = color.rgb_color().g();
		rgb.blue = color.rgb_color().b();
		return rgb;
		break;
	}
	case 4:
	{
		svg::Rgba rgba;
		rgba.red = color.rgba_color().r();
		rgba.green = color.rgba_color().g();
		rgba.blue = color.rgba_color().b();
		rgba.opacity = color.rgba_color().a();
		return rgba;
		break;
	}
	default:
		std::cout << "Incorrect color code: " << color.color_type() << std::endl;
		break;
	}
}

std::deque<transport_catalogue::StopToAdd> DeserialiseStops(const serialization::TransportDB& db) {
	std::deque<transport_catalogue::StopToAdd> result;

	for (auto& stop : db.stops()) {
		transport_catalogue::StopToAdd stop_add;

		stop_add.name = stop.stop_name();
		stop_add.location.lat = stop.coordinates().lat();
		stop_add.location.lng = stop.coordinates().lng();

		for (auto& distance : stop.distance()) {
			stop_add.distances_to_stops.push_back({ distance.stop_name_to(), distance.distance() });
		}

		result.push_back(std::move(stop_add));
	}

	return result;
}

std::deque<transport_catalogue::BusToAdd> DeserialiseBuses(const serialization::TransportDB& db) {
	std::deque<transport_catalogue::BusToAdd> result;

	for (auto& bus : db.buses()) {
		transport_catalogue::BusToAdd bus_add;

		bus_add.is_circle = bus.is_round();
		bus_add.name = bus.bus_name();

		for (auto& stop : bus.stop_names()) {
			bus_add.stops.push_back(stop);
		}

		result.push_back(std::move(bus_add));
	}

	return result;
}

map_renderer::RenderSettings DeserialiseRenderSettings(const serialization::TransportDB& db) {
	map_renderer::RenderSettings result;

	auto& render_in = db.render_settings();

	result.width = render_in.width();
	result.height = render_in.height();
	result.padding = render_in.padding();
	result.stop_radius = render_in.stop_radius();
	result.line_width = render_in.line_width();
	result.bus_label_font_size = render_in.bus_label_font_size();
	result.stop_label_font_size = render_in.stop_label_font_size();
	result.underlayer_width = render_in.underlayer_width();

	result.bus_label_dx = render_in.bus_label().dx();
	result.bus_label_dy = render_in.bus_label().dy();
	result.stop_label_dx = render_in.stop_label().dx();
	result.stop_label_dy = render_in.stop_label().dy();

	result.underlayer_color = details::DeserialiseColor(render_in.underlayer_color());

	result.color_pallete.reserve(render_in.color_pallete().size());
	for (auto& color : render_in.color_pallete()) {
		result.color_pallete.push_back(details::DeserialiseColor(color));
	}

	return result;
}

} // End of details 

void Serialize(
	std::deque<transport_catalogue::StopToAdd>& stops_to_add,
	std::deque<transport_catalogue::BusToAdd>& buses_to_add,
	map_renderer::RenderSettings& render_settings,
	transport_router::RoutingSettings& routing_settings,
	serialization::Settings& ser_settings) {

	// Объект для работы с выводом в файл
	serialization::TransportDB db_out;

	// Заполнение вывода остановками
	for (auto& stop : stops_to_add) {
		auto stop_ptr = db_out.add_stops();
		*stop_ptr = details::SerialiseStop(stop);
	}

	// Заполение вывода маршрутами
	for (auto& bus : buses_to_add) {
		auto bus_ptr = db_out.add_buses();
		*bus_ptr = details::SerialiseBus(bus);
	}

	// Заполение настроек отрисовки
	auto render_ptr = db_out.mutable_render_settings();
	*render_ptr = details::SerialiseRenderSettings(render_settings);

	// Заполнение настроек роутера
	auto router_ptr = db_out.mutable_routing_settings();
	router_ptr->set_bus_velocity(routing_settings.bus_velocity);
	router_ptr->set_bus_wait_time(routing_settings.bus_wait_time);

	const std::filesystem::path path = ser_settings.file_name;
	std::ofstream out_file(path, std::ios::binary);
	db_out.SerializePartialToOstream(&out_file);

}

std::optional<DeserializedParameters> Deserialize(serialization::Settings& ser_settings) {
	const std::filesystem::path path = ser_settings.file_name;
	std::ifstream in_file(path, std::ios::binary);
	
	// Объект для работы с выводом в файл
	serialization::TransportDB db_in;
	
	if (!db_in.ParseFromIstream(&in_file)) {
		return std::nullopt;
	}

	DeserializedParameters result;

	// Заполнение остановок
	result.stops_to_add = std::move(details::DeserialiseStops(db_in));

	// Заполнение маршрутов
	result.buses_to_add = std::move(details::DeserialiseBuses(db_in));

	// Заполнение параметров отрисовки
	result.render_settings = std::move(details::DeserialiseRenderSettings(db_in));

	// Заполенение параметров движения
	result.routing_settings.bus_velocity = db_in.routing_settings().bus_velocity();
	result.routing_settings.bus_wait_time = db_in.routing_settings().bus_wait_time();

	return result;
}

}