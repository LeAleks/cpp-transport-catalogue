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
#include <utility>


namespace transport_catalogue {


class TransportCatalogue {
public:
	// Добавление остановки в базу
	void AddStop(Stop&& stop);

	// Добавление маршрута в базу
	void AddBus(std::string_view name, bool is_circle, std::vector<std::string_view>& stops);
	void AddBus(const BusToAdd& bus_to_add);

	// Заполнение базы из набора данных
	void FillCatalogue(const std::deque<StopToAdd>& stops_to_add, const std::deque<BusToAdd>& buses_to_add);

	// Поиск остановки по имени. Возвращает указатель на остановку
	// Если остановки нет, то возвращает нулевой указатель
	const Stop* GetStopByName(std::string_view stop_name) const;

	// Поиск маршрута по имени. Возвращает указатель на маршрут
	// Если маршрута нет, то возвращает нулевой указатель
	const Bus* GetBusByName(std::string_view bus_name) const;

	// Поиск маршрутов через остановку. Возвращает указатель на список маршрутов через остановку
	// Если остановки нет, то возвращает нулевой указатель
	//const std::set<std::string_view>* FindStop(std::string_view stop_name) const;
	const std::set<const Bus*>* GetBusesToStop(std::string_view stop_name) const;

	// Задание дистанции между остановками
	void SetDistance(StopsDistance& distance);

	// Получение физического расстояния между остановками из словаря. Возвращает ноль,
	// если ищется одна и та же остановка, или такой пары остановок нет
	int GetDistance(std::string_view stop1, std::string_view stop2) const;

	// Получение информации о маршруте
	std::optional<BusStat> GetBusInfo(std::string_view bus_name) const;

	// Возврат сортированного списка указателей на все остановки
	std::vector<const Stop*> GetStopList() const;

	// Возврат сортированного списка указателей на все остановки
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
	//std::unordered_map<Stop*, std::set<std::string_view>> buses_to_stop_;
	std::unordered_map<Stop*, std::set<const Bus*>> buses_to_stop_;

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