#include "stat_reader.h"
#include "input_reader.h"

#include <string>
#include <vector>
#include <iostream>
#include <string_view>

using namespace std;


namespace transport_catalogue {

namespace stat_reader{

namespace detail{

void RequestStop(TransportCatalogue& catalog, string_view query, std::ostream& out) {
    // Удаление префикса "Stop "
    query.remove_prefix(5);

    // Возврат информации об остановке
    auto info_ptr = catalog.FindStop(query);

    // Если возвращен пустой указатель, то остановка не найдена
    if (info_ptr == nullptr) {
        out << "Stop "s << query << ": not found\n";
    }
    // Если возвращеннй массив пустой, то маршрутов нет
    else if ((*info_ptr).empty()) {
        out << "Stop "s << query << ": no buses\n";
    }
    // Вывод маршрутов
    else {
        out << "Stop "s << query << ": buses ";
        bool is_first = true;

        for (auto& i : *info_ptr) {
            if (is_first) {
                is_first = false;
                out << i;
                //out << i->name_;
            }
            else
            {
                out << " " << i;
                //out << " " << i->name_;
            }
        }
        out << endl;
    }
}

void RequestBus(TransportCatalogue& catalog, string_view query, std::ostream& out) {
    // Удаление префикса "Bus "
    query.remove_prefix(4);

    // Возврат информации о маршруте
    auto route = catalog.GetBusInfo(query);

    // Если пришли нулевые значения, то маршрут не найден
    if (route.has_value()) {
        out << "Bus "s << query << ": "s <<
            route->stop_count << " stops on route, "s <<
            route->unique_stop_count << " unique stops, "s <<
            route->route_length << " route length, "s <<
            route->curvature << " curvature\n"s;  
    }
    else {
        out << "Bus "s << query << ": not found\n"s;
    }

}


} // Конец detail

void ReadQuery(TransportCatalogue& catalog, std::istream& in, std::ostream& out){
    // Количество запросов
    size_t number_of_lines = input_reader::detail::ReadLineWithNumber(in);

    // Считываем строки по отдельности
    for (size_t i = 0; i < number_of_lines; ++i) {
        string line;
        getline(in, line);

        // Перевод стоки в string_view
        string_view line_sv(line);

        // Определение типа запроса
        if (line_sv[0] == 'S') {
            detail::RequestStop(catalog, line_sv, out);
        }
        else {
            detail::RequestBus(catalog, line_sv, out);
        }
    }
}

} // End Of stat_reader

} // End Of transport_catalog