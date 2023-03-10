#include "transport_catalogue.h"

#include <cassert>
#include <unordered_set>

#include <iostream>

using namespace std;


namespace transport_catalogue {


// ���������� ��������� � ����
void TransportCatalogue::AddStop(string_view name, double latitude, double longitude) {
	// ������� ���� ���������
	Stop_ stop;

	// ��������� ����
	stop.name_ = string(name);
	stop.location_.lat = latitude;
	stop.location_.lng = longitude;

	// ��������� ��������� � ������
	Stop_* ptr = &all_stops_.emplace_back(move(stop));

	// ��������� ���������� �� ��������� � ���� ���������
	stops_list_[ptr->name_] = ptr;

	// ��������� ��������� � ������� ��������� �� ������� ���������
	buses_to_stop_[ptr];

}

// ���������� �������� � ����
void TransportCatalogue::AddBus(string_view name, bool is_circle, vector<string_view>& stops_list) {
	// ������� ���� Bus
	Bus_ bus;

	// ����������� ��� � ��� ��������
	bus.name_ = string(name);
	bus.is_circle_ = is_circle;

	// ������� ������ ���������� �� ��������� �� ��������� ���������� ��������
	bus.stops_.reserve(stops_list.size());

	// ��������� ������� � ������ � ����������
	Bus_* bus_ptr = &all_buses_.emplace_back(move(bus));

	// �� ��������� �� ������� ��������� ������ ���������, ������ � ���� ��������
	// ������� ��������� �� ������� ���������
	for (string_view stop_name : stops_list) {
		// ������� ��������� �� ���������
		auto& stop_ptr = stops_list_.at(stop_name);

		// �������� ��������� �� ��������� ������ ��������
		bus_ptr->stops_.push_back(stop_ptr);

		// ������� ��������� � ������� ��������� �� ������� ��������� � ���������
		// � ������ �������
		buses_to_stop_[stop_ptr].insert(bus_ptr->name_);
	}

	// ��������� ���������� � �������� � ���� ���������
	buses_list_[bus_ptr->name_] = bus_ptr;
}



// ����� ��������� �� �����. ���������� ��������� �� ������ ��������� ����� ���������
// ���� ��������� ���, �� ���������� ������� ���������
set<string_view>* TransportCatalogue::FindStop(string_view stop_name) {
	// ��������� �� ������� ���������
	set<string_view>* buses_list_ptr = nullptr;

	// �������� ������� ��������� � ������� ���������
	if (stops_list_.count(stop_name)) {
		// ���� ��������� ����, �� ����� ������� �� ��� � �� ���� ���� ������ ��������� ���
		// ���������. ���������� ��������� �� ������
		buses_list_ptr = &buses_to_stop_.at(stops_list_.at(stop_name));
	}

	return buses_list_ptr;
}

// ����� �������� �� �����. ���������� ��������� �� �������
// ���� ��������� ���, �� ���������� ������� ���������
TransportCatalogue::Bus_* TransportCatalogue::FindBus(string_view bus_name) {
	Bus_* bus = nullptr;

	if (buses_list_.count(bus_name)) {
		bus = buses_list_.at(bus_name);
	}

	return bus;
}



// ������� ��������� ����� �����������
void TransportCatalogue::SetDistance(string_view stop1, string_view stop2, int distance) {
	// ������� ���������� �� ���������
	Stop_* stop1_ptr = stops_list_.at(stop1);
	Stop_* stop2_ptr = stops_list_.at(stop2);

	// �������� �������� � �������
	stop_distance_[{stop1_ptr, stop2_ptr}] = distance;
}

// ��������� ����������� ���������� ����� ����������� �� �������
int TransportCatalogue::GetDistance(string_view stop1, string_view stop2) {
	// ������� ���������� �� ���������
	Stop_* stop1_ptr = stops_list_.at(stop1);
	Stop_* stop2_ptr = stops_list_.at(stop2);

	// ������ ��� ����������
	int distance;

	// ���� ��������. ���� �� �������, �� ������ �������� �������.
	// ���� � ��� �� �������, �� ��� ������ �� ���� � �� �� ���������, ��� �������
	// ��� ���������� ���������� (0)
	if (stop_distance_.count({ stop1_ptr, stop2_ptr })) {
		distance = stop_distance_.at({ stop1_ptr, stop2_ptr });
	}
	else if (stop_distance_.count({ stop2_ptr, stop1_ptr })) {
		distance = stop_distance_.at({ stop2_ptr, stop1_ptr });
	}
	else {
		distance = 0;
	}
	
	return distance;
}

// ������ ���������� ��������� �� �������� � �������������� �����
// ������� (�����, ����������, ����������, ������������)
tuple<int, int, int, double> TransportCatalogue::StopsCount(Bus_* bus_ptr) {

	// ���������, ��� ��������� ������ ��� 1
	if (bus_ptr->stops_.size() < 2) {
		return { 1, 1, 0, 0.0 };
	}

	// ���������� ��������� � ������
	size_t list_size = bus_ptr->stops_.size();

	// ���������� ��������� �� ��������
	int number_of_stops;

	// ������������ �������� � ����������� �� ���� ��������
	bus_ptr->is_circle_ ? number_of_stops = list_size : number_of_stops = 2 * list_size - 1;

	// ������ ���������� ���������
	unordered_set<string_view> single_names;

	// ���������� ���������� ���������
	int unique_stops = 0;

	// ����� ��������������� ��������
	double distance_geo = 0;

	// ����� ����������� ��������
	int distance_fact = 0;

	// ���� ������� ���������, �� �������� �� ������ ��������� 1 ���
	// ���� ������� ��������, �� ��������� ������� ����������
	if (bus_ptr->is_circle_) {
		for (size_t i = 1; i < list_size; ++i) {
			// ������� �������� ��������� � ������
			single_names.insert(bus_ptr->stops_[i - 1]->name_);
			single_names.insert(bus_ptr->stops_[i]->name_);

			// ������ ��������������� ���������� ����� �����������
			distance_geo += geo::ComputeDistance(bus_ptr->stops_[i - 1]->location_, bus_ptr->stops_[i]->location_);

			// ������ ������������ ���������� ����� �����������
			distance_fact += GetDistance(bus_ptr->stops_[i - 1]->name_, bus_ptr->stops_[i]->name_);
		}
	}
	else {
		for (size_t i = 1; i < list_size; ++i) {
			// ������� �������� ��������� � ������
			single_names.insert(bus_ptr->stops_[i - 1]->name_);
			single_names.insert(bus_ptr->stops_[i]->name_);

			// ������ ��������������� ���������� ����� ����������� (���������)
			distance_geo += 2 * geo::ComputeDistance(bus_ptr->stops_[i - 1]->location_, bus_ptr->stops_[i]->location_);

			// ������ ������������ ���������� ����� �����������.
			// ������������ ���������� � ���� � ������ �������
			distance_fact +=
				GetDistance(bus_ptr->stops_[i - 1]->name_, bus_ptr->stops_[i]->name_) +
				GetDistance(bus_ptr->stops_[i]->name_, bus_ptr->stops_[i - 1]->name_);
		}

		// � ������������ ���������� ��������� ����������, ������� ����� ������
		// �������� ��� �������� �� �� �� ��������� (������ � �����)
		distance_fact +=
			GetDistance(bus_ptr->stops_[0]->name_, bus_ptr->stops_[0]->name_) +
			GetDistance(bus_ptr->stops_[list_size - 1]->name_, bus_ptr->stops_[list_size - 1]->name_);

	}

	unique_stops = single_names.size();

	// ������ ������������
	double curvature = distance_fact / distance_geo;

	return { number_of_stops, unique_stops, distance_fact, curvature };
}

// ��������� ���������� � ��������
tuple<string_view, int, int, int, double> TransportCatalogue::GetBusInfo(string_view bus_name) {
	Bus_* bus_ptr = FindBus(bus_name);

	// ��������, ��� ����� ������� ����. ���� ���, ���������� ������� ��������
	if (bus_ptr == nullptr) {
		return { bus_name, 0 , 0, 0, 0.0 };
	}

	// ���������� �� ���������� �� �������� � �������������� ����� ��������
	auto [number_of_stops, unique_stops, distance, curvature] = StopsCount(bus_ptr);

	// ������ ���������� ����� ��������

	return { bus_ptr->name_, number_of_stops, unique_stops, distance, curvature };
}



} // End Of transport_catalog