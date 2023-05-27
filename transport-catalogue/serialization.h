#pragma once

#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>
#include <deque>

namespace serialization {

struct Settings {
	std::string file_name;
};

struct DeserializedParameters {
	std::deque<transport_catalogue::StopToAdd> stops_to_add;
	std::deque<transport_catalogue::BusToAdd> buses_to_add;
	map_renderer::RenderSettings render_settings;
	transport_router::RoutingSettings routing_settings;
};

void Serialize(
	std::deque<transport_catalogue::StopToAdd>& stops_to_add,
	std::deque<transport_catalogue::BusToAdd>& buses_to_add,
	map_renderer::RenderSettings& render_settings,
	transport_router::RoutingSettings& routing_settings,
	serialization::Settings& ser_settings);

std::optional<DeserializedParameters> Deserialize(serialization::Settings& ser_settings);
}