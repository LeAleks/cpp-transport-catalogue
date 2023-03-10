#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <set>


namespace transport_catalogue {


class TransportCatalogue {
private:
	// Информация об остановке
	struct Stop_ {
		std::string name_;
		geo::Coordinates location_;
	};

	// Информация о маршруте
	struct Bus_ {
		bool is_circle_;
		std::string name_;
		std::vector<const Stop_*> stops_;
	};

public:
	// Добавление остановки в базу
	void AddStop(std::string_view name, double latitude, double longitude);

	// Добавление маршрута в базу
	void AddBus(std::string_view name, bool is_circle, std::vector<std::string_view>& stops);

	// Поиск остановки по имени. Возвращает указатель на список маршрутов через остановку
	// Если остановки нет, то возвращает нулевой указатель
	std::set<std::string_view>* FindStop(std::string_view stop_name);

	// Поиск маршрута по имени. Возвращает указатель на маршрут
	// Если остановки нет, то возвращает нулевой указатель
	Bus_* FindBus(std::string_view bus_name);

	// Задание дистанции между остановками
	void SetDistance(std::string_view stop1, std::string_view stop2, int distance);

	// Получение физического расстояния между остановками из словаря
	int GetDistance(std::string_view stop1, std::string_view stop2);

	// Получение информации о маршруте
	std::tuple<std::string_view, int, int, int, double> GetBusInfo(std::string_view bus_name);

private:
	// Массив данных об остановках
	std::deque<Stop_> all_stops_;

	// Массив данных о маршрутах
	std::deque<Bus_> all_buses_;

	// Словарь указателей на остановки
	std::unordered_map<std::string_view, Stop_*> stops_list_;

	// Словарь указателей на маршруты
	std::unordered_map<std::string_view, Bus_*> buses_list_;

	// Словарь списков маршрутов по остановкам
	std::unordered_map<Stop_*, std::set<std::string_view>> buses_to_stop_;

	// Хеш функция для работы с парой указателей
	struct HasherDistance {
		size_t operator()(const std::pair<Stop_*, Stop_*>& stop_names) const {
			return size_t(stop_names.first) + 101 * size_t(stop_names.second);
		}
	};

	// Словарь расстояний между остановками
	std::unordered_map<std::pair<Stop_*, Stop_*>, int, HasherDistance> stop_distance_;

	// Расчет количества остановок на маршруте и географическую длину
	// Возврат (общее, уникальное, расстояние, извилистость)
	std::tuple<int, int, int, double> StopsCount(Bus_* bus_ptr);

};


} // End Of transport_catalog