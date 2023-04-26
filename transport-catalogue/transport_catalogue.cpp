#include "transport_catalogue.h"

#include <cassert>
#include <unordered_set>
#include <algorithm>

using namespace std;

namespace transport_catalogue {

// Добавление остановки в базу
void TransportCatalogue::AddStop(Stop&& stop) {
	// Добавляем остановку в массив
	Stop* ptr = &all_stops_.emplace_back(move(stop));

	// Добавляем информацию об остановке в лист остановок
	stops_list_[ptr->name_] = ptr;

	// Добавляем остановку в словарь остановок со списком маршрутов
	buses_to_stop_[ptr];
}

// Добавление маршрута в базу
void TransportCatalogue::AddBus(string_view name, bool is_circle, vector<string_view>& stops_list_add) {
	// Создаем узел Bus
	Bus bus;

	// Присваиваем имя и тип маршрута
	bus.name_ = string(name);
	bus.is_circle_ = is_circle;

	// Создаем массив указателей на остановки на основании переданных названии
	bus.stops_.reserve(stops_list_add.size());

	// Добавляем маршрут в массив с маршрутами
	Bus* bus_ptr = &all_buses_.emplace_back(move(bus));

	// По указателю на маршрут заполняем список остановок, вместе с этим обновляя
	// словарь остановок со списком маршрутов
	for (string_view stop_name : stops_list_add) {
		// Находим указатель на остановку
		auto& stop_ptr = stops_list_.at(stop_name);

		// Копируем указатель на остановку внутрь маршрута
		bus_ptr->stops_.push_back(stop_ptr);

		// Находим остановку в словаре остановок со списком маршрутов и добавляем
		// в список маршрут
		//buses_to_stop_[stop_ptr].insert(bus_ptr->name_);
		buses_to_stop_[stop_ptr].insert(bus_ptr);
	}

	// Добавляем информацию о маршруте в лист маршрутов
	buses_list_[bus_ptr->name_] = bus_ptr;
}

void TransportCatalogue::AddBus(const BusToAdd& bus_to_add) {
	// Создаем узел Bus
	Bus bus;

	// Присваиваем имя и тип маршрута
	bus.name_ = bus_to_add.name;
	bus.is_circle_ = bus_to_add.is_circle;

	// Создаем массив указателей на остановки на основании переданных названии
	bus.stops_.reserve(bus_to_add.stops.size());

	// Добавляем маршрут в массив с маршрутами
	Bus* bus_ptr = &all_buses_.emplace_back(move(bus));

	// По указателю на маршрут заполняем список остановок, вместе с этим обновляя
	// словарь остановок со списком маршрутов
	for (string_view stop_name : bus_to_add.stops) {
		// Находим указатель на остановку
		auto& stop_ptr = stops_list_.at(stop_name);

		// Копируем указатель на остановку внутрь маршрута
		bus_ptr->stops_.push_back(stop_ptr);

		// Находим остановку в словаре остановок со списком маршрутов и добавляем
		// в список маршрут
		//buses_to_stop_[stop_ptr].insert(bus_ptr->name_);
		buses_to_stop_[stop_ptr].insert(bus_ptr);
	}

	// Добавляем информацию о маршруте в лист маршрутов
	buses_list_[bus_ptr->name_] = bus_ptr;
}

// Заполнение базы из набора данных
void TransportCatalogue::FillCatalogue(const std::deque<StopToAdd>& stops_to_add, const std::deque<BusToAdd>& buses_to_add) {
	// Добваление остановок в базу
	for (auto& stop_to_add : stops_to_add) {
		Stop stop;

		stop.name_ = stop_to_add.name;
		stop.location_ = stop_to_add.location;

		AddStop(move(stop));
	}
	
	// Добавление расстояний между остановками
	for (auto& stop_to_add : stops_to_add) {
		StopsDistance distance;
		distance.stop1_name_ = stop_to_add.name;

		for (auto& to_stop : stop_to_add.distances_to_stops) {
			distance.stop2_name_ = to_stop.first;
			distance.distance_ = to_stop.second;

			SetDistance(distance);
		}
	}

	// Добавление маршрутов
	for (auto& bus_to_add : buses_to_add) {
		AddBus(bus_to_add);
	}
	
}


// Поиск остановки по имени. Возвращает указатель на остановку
// Если остановки нет, то возвращает нулевой указатель
const Stop* TransportCatalogue::GetStopByName(std::string_view stop_name) const {
	Stop* stop = nullptr;

	if (stops_list_.count(stop_name)) {
		stop = stops_list_.at(stop_name);
	}

	return stop;
}

// Поиск маршрута по имени. Возвращает указатель на маршрут
// Если остановки нет, то возвращает нулевой указатель
const Bus* TransportCatalogue::GetBusByName(string_view bus_name) const {
	Bus* bus = nullptr;

	if (buses_list_.count(bus_name)) {
		bus = buses_list_.at(bus_name);
	}

	return bus;
}

// Поиск остановки по имени. Возвращает указатель на список маршрутов через остановку
// Если остановки нет, то возвращает нулевой указатель
//const set<string_view>* TransportCatalogue::FindStop(string_view stop_name) const {
const set<const Bus*>* TransportCatalogue::GetBusesToStop(string_view stop_name) const {
	//const set<Bus*>* TransportCatalogue::FindStop(string_view stop_name) const{
		// Проверка наличия остановки в словаре остановок
	if (stops_list_.count(stop_name)) {
		// Если остановка есть, то берем указать на нее и по нему ищем список маршрутов для
		// остановки. Возвращаем указатель на список
		//return &buses_to_stop_.at(stops_list_.at(stop_name));
		return &buses_to_stop_.at(stops_list_.at(stop_name));
	}

	return nullptr;
}



// Задание дистанции между остановками
void TransportCatalogue::SetDistance(StopsDistance& distance) {
	// Возврат указателей на остановки
	Stop* stop1_ptr = stops_list_.at(distance.stop1_name_);
	Stop* stop2_ptr = stops_list_.at(distance.stop2_name_);

	// Внесение значения в словарь
	stop_distance_[{stop1_ptr, stop2_ptr}] = distance.distance_;
}

// Получение физического расстояния между остановками из словаря. Возвращает ноль,
// если ищется одна и та же остановка, или такой пары остановок нет
int TransportCatalogue::GetDistance(string_view stop1, string_view stop2) const {
	// Возврат указателей на остановки
	Stop* stop1_ptr = stops_list_.at(stop1);
	Stop* stop2_ptr = stops_list_.at(stop2);

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
BusStat TransportCatalogue::StopsCount(const Bus* bus_ptr) const {

	// Проверяем, что остановок больше чем 1
	if (bus_ptr->stops_.size() < 2) {
		return { 0.0, 0, 1, 1 };
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

	//return { number_of_stops, unique_stops, distance_fact, curvature };
	return { curvature, distance_fact, number_of_stops, unique_stops };
}

// Получение информации о маршруте
optional<BusStat> TransportCatalogue::GetBusInfo(string_view bus_name) const {
	const Bus* bus_ptr = GetBusByName(bus_name);

	// Проверка, что такой маршрут есть. Если нет, возвращает nullopt,
	// означающее отсутствие значения
	if (bus_ptr == nullptr) {
		return nullopt;
	}

	// Информация об остановках на маршруте и георгафической длине маршрута
	return StopsCount(bus_ptr);
}

// Возврат списка указателей на все остановки
vector<const Stop*> TransportCatalogue::GetStopList() const {
	vector<const Stop*> result;
	result.reserve(stops_list_.size());

	for (auto& i : stops_list_) {
		result.push_back(i.second);
	}

	sort(result.begin(), result.end(),
		[](const Stop* left, const Stop* right)
		{
			return lexicographical_compare(
				left->name_.begin(), left->name_.end(),
				right->name_.begin(), right->name_.end());
		});

	return result;
}

// Возврат списка указателей на все маршруты
vector<const Bus*> TransportCatalogue::GetBusList() const {
	vector<const Bus*> result;
	result.reserve(buses_list_.size());

	// Перенос данных из unordered_map в vector
	for (auto& i : buses_list_) {
		result.push_back(i.second);
	}

	// Сортировка по названию маршрута
	sort(result.begin(), result.end(),
		[](const Bus* left, const Bus* right)
		{
			return lexicographical_compare(
				left->name_.begin(), left->name_.end(),
				right->name_.begin(), right->name_.end());
		});

	return result;
}

} // End Of transport_catalog