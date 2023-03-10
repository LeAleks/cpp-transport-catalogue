#include "input_reader.h"

#include <deque>
#include <string>
#include <vector>
#include <iostream>
#include <string_view>
#include <unordered_map>


namespace transport_catalogue {


namespace input_reader {

using namespace std;


namespace detail{

// Узел хранения инфомарции о расстоянии между остановками
struct StopDistance {
    int distance_;
    string_view stop1_name_;
    string_view stop2_name_;
};

string ReadLine(istream& in) {
    string s;
    getline(in, s);
    return s;
}

int ReadLineWithNumber(istream& in) {
    int result = 0;
    in >> result;
    ReadLine(in);
    return result;
}

// Обрезание пробелов по левому краю
string_view ltrim(string_view s) {
    size_t start = s.find_first_not_of(' ');
    return (start == string_view::npos) ? "" : s.substr(start);
}

// Обрезание пробелов по правому краю
string_view rtrim(string_view s) {
    size_t end = s.find_last_not_of(' ');
    return (end == string_view::npos) ? "" : s.substr(0, end + 1);
}

// Обрезание пробелов по краям строки
string_view TrimLine(string_view s) {
    return rtrim(ltrim(s));
}

// Выделение названия
string_view GetName(string_view& line_sv) {
    // Удаление префикса
    line_sv.remove_prefix(line_sv.find(' ') + 1);

    // Нахождение двоеточия
    size_t name_end = line_sv.find(':');

    // Выделение названия
    string_view name = line_sv.substr(0, name_end);

    // Удаление названия из строки
    line_sv.remove_prefix(line_sv.find_first_not_of(':', name_end));

    return name;
}

// Выделение одной координаты
double GetNumber(string_view& line_sv) {
    double number;

    // Находим знак разделения ','
    size_t comma_pos = line_sv.find(',');

    // Выделяем данные
    string_view lng = line_sv.substr(0, comma_pos);

    // Преобразуем данные
    number = stod(string(TrimLine(lng)));

    // Удаление данных из строки
    line_sv.remove_prefix(min(line_sv.find_first_not_of(',', comma_pos), line_sv.size()));

    return number;
}

// Выделение координат
pair<double, double> GetCoordinates(string_view& line_sv) {
    // Возвращаемые значения
    double latitude = GetNumber(line_sv);
    double longtitude = GetNumber(line_sv);

    // Возвращаем даные
    return { latitude, longtitude };
}

// Выделение названия второй остановки и расстояния до нее
StopDistance GetDistance(string_view stop1_name, string_view& line_sv) {
    // Буффер для возврата
    StopDistance distance;

    // Присваиваем имя текущей остановки
    distance.stop1_name_ = stop1_name;
    
    // Находим конец расстояния 'm'
    size_t distance_end = line_sv.find('m');

    // Преобразование сроки в число
    distance.distance_ = stoi(string(line_sv.substr(0, distance_end)));

    // Убираем данные и предлог перед названием
    line_sv.remove_prefix(distance_end + 4);

    // Нахождение разделителя ','
    size_t comma_pos = line_sv.find(',');

    // Выделение названия второй остановки
    distance.stop2_name_ = TrimLine(line_sv.substr(0, min(comma_pos, line_sv.size())));

    // Удаляем название
    line_sv.remove_prefix(min(line_sv.find_first_not_of(',', comma_pos), line_sv.size()));

    return distance;
}

// Разбитие строки на данные об остановке. Возвращает свойства остановки
// и пеносит данные о расстояниях в переданный словарь
tuple<string_view, double, double> ParseStopInfo(string_view line_sv, deque<StopDistance>& distances) {
    // Выделение название остановки
    string_view stop_name = GetName(line_sv);

    auto [latitude, longitude] = GetCoordinates(line_sv);

    // Указание расстояния до остановок, если есть в запросе
    while (!line_sv.empty()) {
        distances.push_back(move(GetDistance(stop_name, line_sv)));
    }

    // Явный возврат значений для возможности тестирования
    return { stop_name, latitude, longitude };
}


// Разбитие строки на данные о маршруте. Возвращает свойства маршрута
tuple<string_view, bool, vector<string_view>> ParseBusInfo(string_view line_sv) {
    // Выделение название маршрута
    string_view bus_name = GetName(line_sv);

    // Провека, что маршрут кольцевой
    bool is_circle = (line_sv.find('>') != line_sv.npos);

    // Массив названий остановок
    vector<string_view> stop_names;

    // Разделители названий остановок
    const string spacers = "->"s;

    // Начало названия остановки
    size_t stop_begin = 0;

    // Конец названия остановки
    size_t stop_end;

    // Последняя позиция в строке
    auto pos_end = line_sv.npos;

    while (stop_begin != pos_end) {
        // Находим индекс разделителя
        auto spacer_id = line_sv.find_first_of(spacers, stop_begin);

        // Определение конца названия остановки
        spacer_id == pos_end ? stop_end = line_sv.size()  : stop_end = spacer_id;

        // Выделение название остановки
        string_view stop_name = TrimLine(line_sv.substr(stop_begin, stop_end - stop_begin));

        // Вносим название в список
        stop_names.push_back(stop_name);

        // Удаление выделенного названия
        line_sv.remove_prefix(stop_end);

        // Переопределяем начало поиска
        stop_begin = line_sv.find_first_not_of(spacers);
    }

    // Явный возврат значений для возможности тестирования
    return {bus_name, is_circle, stop_names};
}

} // End Of detail



// Считывание входного потока для добавления инфомарции в базу
void ReadLines(TransportCatalogue& catalog, std::istream& in) {
	// Количество запросов
    size_t number_of_lines = detail::ReadLineWithNumber(in);

    // Массив для хранения строк с информацией по остановкам при чтении
    vector<string> stop_line_s;
    stop_line_s.reserve(number_of_lines);

    // Массив для хранения строк с информацией по маршрутам при чтении
    vector<string> bus_line_s;
    bus_line_s.reserve(number_of_lines);

    // Словарь для хранения расстояний между остановками
    deque<detail::StopDistance> stops_distance;

    // Считываем строки по отдельности
    for (size_t i = 0; i < number_of_lines; ++i) {
        string line;
        getline(in, line);

        // К какому типу относится строка: остановка или маршрут
        line[0] == 'S' ? stop_line_s.push_back(move(line)) : bus_line_s.push_back(move(line));
    }

    // Добавление остановок в базу
    for (string_view line_sv : stop_line_s) {
        // Выделение информации об остановке
        auto [stop_name, latitude, longitude] = detail::ParseStopInfo(line_sv, stops_distance);

        // Перенос инфомарции в базу
        catalog.AddStop(stop_name, latitude, longitude);
    }

    // Добавление информации о расстояниях в базу
    for (detail::StopDistance& distance : stops_distance) {
        catalog.SetDistance(distance.stop1_name_, distance.stop2_name_, distance.distance_);
    }

    // Добавление маршрутов в базу
    for (string_view line_sv : bus_line_s) {
        auto [bus_name, is_circle, stop_names]  = move(detail::ParseBusInfo(line_sv));

        // Перенос инфомарции в базу
        catalog.AddBus(bus_name, is_circle, stop_names);
    }
}

} // End Of reader


} // End Of transport_catalog