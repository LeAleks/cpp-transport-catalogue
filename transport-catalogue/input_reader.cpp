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

// ���� �������� ���������� � ���������� ����� �����������
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

// ��������� �������� �� ������ ����
string_view ltrim(string_view s) {
    size_t start = s.find_first_not_of(' ');
    return (start == string_view::npos) ? "" : s.substr(start);
}

// ��������� �������� �� ������� ����
string_view rtrim(string_view s) {
    size_t end = s.find_last_not_of(' ');
    return (end == string_view::npos) ? "" : s.substr(0, end + 1);
}

// ��������� �������� �� ����� ������
string_view TrimLine(string_view s) {
    return rtrim(ltrim(s));
}

// ��������� ��������
string_view GetName(string_view& line_sv) {
    // �������� ��������
    line_sv.remove_prefix(line_sv.find(' ') + 1);

    // ���������� ���������
    size_t name_end = line_sv.find(':');

    // ��������� ��������
    string_view name = line_sv.substr(0, name_end);

    // �������� �������� �� ������
    line_sv.remove_prefix(line_sv.find_first_not_of(':', name_end));

    return name;
}

// ��������� ����� ����������
double GetNumber(string_view& line_sv) {
    double number;

    // ������� ���� ���������� ','
    size_t comma_pos = line_sv.find(',');

    // �������� ������
    string_view lng = line_sv.substr(0, comma_pos);

    // ����������� ������
    number = stod(string(TrimLine(lng)));

    // �������� ������ �� ������
    line_sv.remove_prefix(min(line_sv.find_first_not_of(',', comma_pos), line_sv.size()));

    return number;
}

// ��������� ���������
pair<double, double> GetCoordinates(string_view& line_sv) {
    // ������������ ��������
    double latitude = GetNumber(line_sv);
    double longtitude = GetNumber(line_sv);

    // ���������� �����
    return { latitude, longtitude };
}

// ��������� �������� ������ ��������� � ���������� �� ���
StopDistance GetDistance(string_view stop1_name, string_view& line_sv) {
    // ������ ��� ��������
    StopDistance distance;

    // ����������� ��� ������� ���������
    distance.stop1_name_ = stop1_name;
    
    // ������� ����� ���������� 'm'
    size_t distance_end = line_sv.find('m');

    // �������������� ����� � �����
    distance.distance_ = stoi(string(line_sv.substr(0, distance_end)));

    // ������� ������ � ������� ����� ���������
    line_sv.remove_prefix(distance_end + 4);

    // ���������� ����������� ','
    size_t comma_pos = line_sv.find(',');

    // ��������� �������� ������ ���������
    distance.stop2_name_ = TrimLine(line_sv.substr(0, min(comma_pos, line_sv.size())));

    // ������� ��������
    line_sv.remove_prefix(min(line_sv.find_first_not_of(',', comma_pos), line_sv.size()));

    return distance;
}

// �������� ������ �� ������ �� ���������. ���������� �������� ���������
// � ������� ������ � ����������� � ���������� �������
tuple<string_view, double, double> ParseStopInfo(string_view line_sv, deque<StopDistance>& distances) {
    // ��������� �������� ���������
    string_view stop_name = GetName(line_sv);

    auto [latitude, longitude] = GetCoordinates(line_sv);

    // �������� ���������� �� ���������, ���� ���� � �������
    while (!line_sv.empty()) {
        distances.push_back(move(GetDistance(stop_name, line_sv)));
    }

    // ����� ������� �������� ��� ����������� ������������
    return { stop_name, latitude, longitude };
}


// �������� ������ �� ������ � ��������. ���������� �������� ��������
tuple<string_view, bool, vector<string_view>> ParseBusInfo(string_view line_sv) {
    // ��������� �������� ��������
    string_view bus_name = GetName(line_sv);

    // �������, ��� ������� ���������
    bool is_circle = (line_sv.find('>') != line_sv.npos);

    // ������ �������� ���������
    vector<string_view> stop_names;

    // ����������� �������� ���������
    const string spacers = "->"s;

    // ������ �������� ���������
    size_t stop_begin = 0;

    // ����� �������� ���������
    size_t stop_end;

    // ��������� ������� � ������
    auto pos_end = line_sv.npos;

    while (stop_begin != pos_end) {
        // ������� ������ �����������
        auto spacer_id = line_sv.find_first_of(spacers, stop_begin);

        // ����������� ����� �������� ���������
        spacer_id == pos_end ? stop_end = line_sv.size()  : stop_end = spacer_id;

        // ��������� �������� ���������
        string_view stop_name = TrimLine(line_sv.substr(stop_begin, stop_end - stop_begin));

        // ������ �������� � ������
        stop_names.push_back(stop_name);

        // �������� ����������� ��������
        line_sv.remove_prefix(stop_end);

        // �������������� ������ ������
        stop_begin = line_sv.find_first_not_of(spacers);
    }

    // ����� ������� �������� ��� ����������� ������������
    return {bus_name, is_circle, stop_names};
}

} // End Of detail



// ���������� �������� ������ ��� ���������� ���������� � ����
void ReadLines(TransportCatalogue& catalog, std::istream& in) {
	// ���������� ��������
    size_t number_of_lines = detail::ReadLineWithNumber(in);

    // ������ ��� �������� ����� � ����������� �� ���������� ��� ������
    vector<string> stop_line_s;
    stop_line_s.reserve(number_of_lines);

    // ������ ��� �������� ����� � ����������� �� ��������� ��� ������
    vector<string> bus_line_s;
    bus_line_s.reserve(number_of_lines);

    // ������� ��� �������� ���������� ����� �����������
    deque<detail::StopDistance> stops_distance;

    // ��������� ������ �� �����������
    for (size_t i = 0; i < number_of_lines; ++i) {
        string line;
        getline(in, line);

        // � ������ ���� ��������� ������: ��������� ��� �������
        line[0] == 'S' ? stop_line_s.push_back(move(line)) : bus_line_s.push_back(move(line));
    }

    // ���������� ��������� � ����
    for (string_view line_sv : stop_line_s) {
        // ��������� ���������� �� ���������
        auto [stop_name, latitude, longitude] = detail::ParseStopInfo(line_sv, stops_distance);

        // ������� ���������� � ����
        catalog.AddStop(stop_name, latitude, longitude);
    }

    // ���������� ���������� � ����������� � ����
    for (detail::StopDistance& distance : stops_distance) {
        catalog.SetDistance(distance.stop1_name_, distance.stop2_name_, distance.distance_);
    }

    // ���������� ��������� � ����
    for (string_view line_sv : bus_line_s) {
        auto [bus_name, is_circle, stop_names]  = move(detail::ParseBusInfo(line_sv));

        // ������� ���������� � ����
        catalog.AddBus(bus_name, is_circle, stop_names);
    }
}

} // End Of reader


} // End Of transport_catalog