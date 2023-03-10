#include "stat_reader.h"

#include <string>
#include <vector>
#include <iostream>
#include <string_view>

using namespace std;


namespace transport_catalogue {

namespace stat_reader{

namespace detail{
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


void RequestStop(TransportCatalogue& catalog, string_view query) {
    // �������� �������� "Stop "
    query.remove_prefix(5);

    // ������� ���������� �� ���������
    auto info_ptr = catalog.FindStop(query);

    // ���� ��������� ������ ���������, �� ��������� �� �������
    if (info_ptr == nullptr) {
        cout << "Stop "s << query << ": not found\n";
    }
    // ���� ����������� ������ ������, �� ��������� ���
    else if ((*info_ptr).empty()) {
        cout << "Stop "s << query << ": no buses\n";
    }
    // ����� ���������
    else {
        cout << "Stop "s << query << ": buses ";
        bool is_first = true;

        for (auto& i : *info_ptr) {
            if (is_first) {
                is_first = false;
                cout << i;
            }
            else
            {
                cout << " " << i;
            }
        }
        cout << endl;
    }
}

void RequestBus(TransportCatalogue& catalog, string_view query) {
    // �������� �������� "Bus "
    query.remove_prefix(4);

    // ������� ���������� � ��������
    auto [bus_name, number_of_stops_on_route, number_unique, lenght, curvature] = catalog.GetBusInfo(query);

    // ���� ������ ������� ��������, �� ������� �� ������
    if (number_of_stops_on_route == 0) {
        cout << "Bus "s << bus_name << ": not found\n"s;
    }
    else {
        cout << "Bus "s << bus_name << ": "s <<
            number_of_stops_on_route << " stops on route, "s <<
            number_unique << " unique stops, "s << 
            lenght << " route length, "s <<
            curvature << " curvature\n"s;
    }

}


} // ����� detail

void ReadQuery(TransportCatalogue& catalog, std::istream& in){
    // ���������� ��������
    size_t number_of_lines = detail::ReadLineWithNumber(in);

    // ��������� ������ �� �����������
    for (size_t i = 0; i < number_of_lines; ++i) {
        string line;
        getline(in, line);

        // ������� ����� � string_view
        string_view line_sv(line);

        // ����������� ���� �������
        if (line_sv[0] == 'S') {
            detail::RequestStop(catalog, line_sv);
        }
        else {
            detail::RequestBus(catalog, line_sv);
        }
    }
}

} // End Of stat_reader

} // End Of transport_catalog