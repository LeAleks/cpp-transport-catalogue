#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "serialization.h"

#include <optional>
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    //std::cout << "main.cpp " << "(" << __LINE__ << ") " << std::endl;
    const std::string_view mode(argv[1]);

    // Считывание JSON
    // Чтение файла json и формирование базы запросов
    json_reader::InputQueries queries = json_reader::ReadTransportJson(std::cin);

    if (mode == "make_base"sv) {
        // make base here
        serialization::Serialize(
            queries.stops_to_add,
            queries.buses_to_add,
            queries.render_settings,
            queries.routing_settings,
            queries.ser_settings);

    } else if (mode == "process_requests"sv) {
        // process requests here
        std::optional<serialization::DeserializedParameters> input = serialization::Deserialize(queries.ser_settings);

        if (!input) {
            std::cout << "Can't read database" << std::endl;
        }
        else {
            // База данных для работы
            transport_catalogue::TransportCatalogue catalogue;
            catalogue.FillCatalogue(input.value().stops_to_add, input.value().buses_to_add);

            // Обработчик маршрутов со встроенным графом маршрутов
            transport_router::TransportRouter transport_router(catalogue, input.value().routing_settings);

            // Обработчик графа маршрутов
            graph::Router<double> router(transport_router.GetGraph());

            // Отрисовщик карты маршрутов в формате SVG
            map_renderer::MapRenderer renderer(input.value().render_settings);

            // Обработчик запросов
            request_handler::RequestHandler request_handler(catalogue, renderer, transport_router, router);

            // Вывод запросов в формате json
            json::Document json_responce = request_handler.GetJsonResponce(queries.requests);
            json::Print(json_responce, std::cout);

        }
    } else {
        PrintUsage();
        return 1;
    }
}
