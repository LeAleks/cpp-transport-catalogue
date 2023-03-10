#include "transport_catalogue.h"

#include <cassert>
#include <unordered_set>

#include <iostream>

using namespace std;


namespace transport_catalogue {


// Добавление остановки в базу
void TransportCatalogue::AddStop(string_view name, double latitude, double longitude) {
	// Создаем узел остановки
	Stop_ stop;

	// Заполняем узел
	stop.name_ = string(name);
	stop.location_.lat = latitude;
	stop.location_.lng = longitude;

	// Добавляем остановку в массив
	Stop_* ptr = &all_stops_.emplace_back(move(stop));

	// Добавляем информацию об остановке в лист остановок
	stops_list_[ptr->name_] = ptr;

	// Добавляем остановку в словарь остановок со списком маршрутов
	buses_to_stop_[ptr];

}

// Добавление маршрута в базу
void TransportCatalogue::AddBus(string_view name, bool is_circle, vector<string_view>& stops_list) {
	// Создаем узел Bus
	Bus_ bus;

	// Присваиваем имя и тип маршрута
	bus.name_ = string(name);
	bus.is_circle_ = is_circle;

	// Создаем массив указателей на остановки на основании переданных названии
	bus.stops_.reserve(stops_list.size());

	// Добавляем маршрут в массив с маршрутами
	Bus_* bus_ptr = &all_buses_.emplace_back(move(bus));

	// По указателю на маршрут заполняем список остановок, вместе с этим обновляя
	// словарь остановок со списком маршрутов
	for (string_view stop_name : stops_list) {
		// Находим указатель на остановку
		auto& stop_ptr = stops_list_.at(stop_name);

		// Копируем указатель на остановку внутрь маршрута
		bus_ptr->stops_.push_back(stop_ptr);

		// Находим остановку в словаре остановок со списком маршрутов и добавляем
		// в список маршрут
		buses_to_stop_[stop_ptr].insert(bus_ptr->name_);
	}

	// Добавляем информацию о маршруте в лист маршрутов
	buses_list_[bus_ptr->name_] = bus_ptr;
}



// Поиск остановки по имени. Возвращает указатель на список маршрутов через остановку
// Если остановки нет, то возвращает нулевой указатель
set<string_view>* TransportCatalogue::FindStop(string_view stop_name) {
	// Указатель на словарь маршрутов
	set<string_view>* buses_list_ptr = nullptr;

	// Проверка наличия остановки в словаре остановок
	if (stops_list_.count(stop_name)) {
		// Если остановка есть, то берем указать на нее и по нему ищем список маршрутов для
		// остановки. Возвращаем указатель на список
		buses_list_ptr = &buses_to_stop_.at(stops_list_.at(stop_name));
	}

	return buses_list_ptr;
}

// Поиск маршрута по имени. Возвращает указатель на маршрут
// Если остановки нет, то возвращает нулевой указатель
TransportCatalogue::Bus_* TransportCatalogue::FindBus(string_view bus_name) {
	Bus_* bus = nullptr;

	if (buses_list_.count(bus_name)) {
		bus = buses_list_.at(bus_name);
	}

	return bus;
}



// Задание дистанции между остановками
void TransportCatalogue::SetDistance(string_view stop1, string_view stop2, int distance) {
	// Возврат указателей на остановки
	Stop_* stop1_ptr = stops_list_.at(stop1);
	Stop_* stop2_ptr = stops_list_.at(stop2);

	// Внесение значения в словарь
	stop_distance_[{stop1_ptr, stop2_ptr}] = distance;
}

// Получение физического расстояния между остановками из словаря
int TransportCatalogue::GetDistance(string_view stop1, string_view stop2) {
	// Возврат указателей на остановки
	Stop_* stop1_ptr = stops_list_.at(stop1);
	Stop_* stop2_ptr = stops_list_.at(stop2);

	// Буффер для расстояния
	int distance;

	// Ищем значение. Если не находит, то меняем названия местами.
	// Если и так не находит, то это запрос на одну и ту же остановку, для которой
	// нет указанного расстояния (0)
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

// Расчет количества остановок на маршруте и географическую длину
// Возврат (общее, уникальное, расстояние, извилистость)
tuple<int, int, int, double> TransportCatalogue::StopsCount(Bus_* bus_ptr) {

	// Проверяем, что остановок больше чем 1
	if (bus_ptr->stops_.size() < 2) {
		return { 1, 1, 0, 0.0 };
	}

	// Количество остановок в списке
	size_t list_size = bus_ptr->stops_.size();

	// Количество остановок на маршруте
	int number_of_stops;

	// Корректируем значения в зависимости от типа маршрута
	bus_ptr->is_circle_ ? number_of_stops = list_size : number_of_stops = 2 * list_size - 1;

	// Список уникальных остановок
	unordered_set<string_view> single_names;

	// Количество уникальных остановок
	int unique_stops = 0;

	// Длина географического маршрута
	double distance_geo = 0;

	// Длина физического маршрута
	int distance_fact = 0;

	// Если маршрут кольцевой, то проходим по списку остановок 1 раз
	// Если маршрут линейный, то учитываем двойное расстояние
	if (bus_ptr->is_circle_) {
		for (size_t i = 1; i < list_size; ++i) {
			// Перенос названий остановок в список
			single_names.insert(bus_ptr->stops_[i - 1]->name_);
			single_names.insert(bus_ptr->stops_[i]->name_);

			// Расчет географического расстояния между остановками
			distance_geo += geo::ComputeDistance(bus_ptr->stops_[i - 1]->location_, bus_ptr->stops_[i]->location_);

			// Расчет фактического расстояния между остановками
			distance_fact += GetDistance(bus_ptr->stops_[i - 1]->name_, bus_ptr->stops_[i]->name_);
		}
	}
	else {
		for (size_t i = 1; i < list_size; ++i) {
			// Перенос названий остановок в список
			single_names.insert(bus_ptr->stops_[i - 1]->name_);
			single_names.insert(bus_ptr->stops_[i]->name_);

			// Расчет географического расстояния между остановками (удвоенное)
			distance_geo += 2 * geo::ComputeDistance(bus_ptr->stops_[i - 1]->location_, bus_ptr->stops_[i]->location_);

			// Расчет фактического расстояния между остановками.
			// Прибавляется расстояние в одну и другую сторону
			distance_fact +=
				GetDistance(bus_ptr->stops_[i - 1]->name_, bus_ptr->stops_[i]->name_) +
				GetDistance(bus_ptr->stops_[i]->name_, bus_ptr->stops_[i - 1]->name_);
		}

		// К фактическому расстоянию добавляем расстояние, которое нужно пройти
		// автобусу для возврата на ту же остановку (начало и конец)
		distance_fact +=
			GetDistance(bus_ptr->stops_[0]->name_, bus_ptr->stops_[0]->name_) +
			GetDistance(bus_ptr->stops_[list_size - 1]->name_, bus_ptr->stops_[list_size - 1]->name_);

	}

	unique_stops = single_names.size();

	// Расчет извилистости
	double curvature = distance_fact / distance_geo;

	return { number_of_stops, unique_stops, distance_fact, curvature };
}

// Получение информации о маршруте
tuple<string_view, int, int, int, double> TransportCatalogue::GetBusInfo(string_view bus_name) {
	Bus_* bus_ptr = FindBus(bus_name);

	// Проверка, что такой маршрут есть. Если нет, возвращает нудевые значения
	if (bus_ptr == nullptr) {
		return { bus_name, 0 , 0, 0, 0.0 };
	}

	// Информация об остановках на маршруте и георгафической длине маршрута
	auto [number_of_stops, unique_stops, distance, curvature] = StopsCount(bus_ptr);

	// Расчет физической длины маршрута

	return { bus_ptr->name_, number_of_stops, unique_stops, distance, curvature };
}



} // End Of transport_catalog