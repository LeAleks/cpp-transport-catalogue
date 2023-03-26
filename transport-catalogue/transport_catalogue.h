#pragma once

#include "geo.h"
#include "domain.h"


#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <optional>
#include <map>


namespace transport_catalogue {


class TransportCatalogue {
public:
	// Добавление остановки в базу
	void AddStop(Stop&& stop);

	// Добавление маршрута в базу
	void AddBus(std::string_view name, bool is_circle, std::vector<std::string_view>& stops);

	// Поиск остановки по имени. Возвращает указатель на список маршрутов через остановку
	// Если остановки нет, то возвращает нулевой указатель
	//const std::set<Bus*>* FindStop(std::string_view stop_name) const;
	const std::set<std::string_view>* FindStop(std::string_view stop_name) const;

	// Поиск маршрута по имени. Возвращает указатель на маршрут
	// Если остановки нет, то возвращает нулевой указатель
	const Bus* FindBus(std::string_view bus_name) const;

	// Задание дистанции между остановками
	void SetDistance(StopsDistance& distance);

	// Получение физического расстояния между остановками из словаря
	int GetDistance(std::string_view stop1, std::string_view stop2) const;

	// Получение информации о маршруте
	std::optional<BusStat> GetBusInfo(std::string_view bus_name) const;

	// Возврат словаря маршрутов с указателями
	std::vector<const Bus*> GetBusList() const;

private:
	// Массив данных об остановках
	std::deque<Stop> all_stops_;

	// Массив данных о маршрутах
	std::deque<Bus> all_buses_;

	// Словарь указателей на остановки
	std::unordered_map<std::string_view, Stop*> stops_list_;

	// Словарь указателей на маршруты
	std::unordered_map<std::string_view, Bus*> buses_list_;

	// Словарь списков маршрутов по остановкам
	std::unordered_map<Stop*, std::set<std::string_view>> buses_to_stop_;
	//std::unordered_map<Stop*, std::set<Bus*>> buses_to_stop_;

	// Хеш функция для работы с парой указателей
	struct HasherDistance {
		size_t operator()(const std::pair<Stop*, Stop*>& stop_names) const {
			return size_t(stop_names.first) + 101 * size_t(stop_names.second);
		}
	};

	// Словарь расстояний между остановками
	std::unordered_map<std::pair<Stop*, Stop*>, int, HasherDistance> stop_distance_;

	// Расчет количества остановок на маршруте и географическую длину
	// Возврат (общее, уникальное, расстояние, извилистость)
	BusStat StopsCount(const Bus* bus_ptr) const;

};


} // End Of transport_catalog