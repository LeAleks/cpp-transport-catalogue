#include "transport_catalogue.h"
#include "transport_router.h"

#include "json_reader.h"
#include "map_renderer.h"
#include "json_builder.h"

#include "request_handler.h"

#include <iostream>
#include <fstream>


using namespace std;

int main() {
	/*
	* Примерная структура программы:
	* - Считать JSON из stdin
	* - Построить на его основе JSON базу данных транспортного справочника
	* - Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
	*   с ответами.
	* - Вывести в stdout ответы в виде JSON
	*/

	// Чтение файла json и формирование базы запросов
	json_reader::InputQueries queries = json_reader::ReadTransportJson(cin);

	// База с маршрутами
	transport_catalogue::TransportCatalogue catalogue;
	catalogue.FillCatalogue(queries.stops_to_add, queries.buses_to_add);

	// Граф с маршрутами
	graph::DirectedWeightedGraph<double> graph(catalogue.GetStopList().size() + 1);

	// Обработчик маршрутов со встроенным графом маршрутов
	transport_router::TransportRouter transport_router(catalogue, queries.routing_settings);
	transport_router.FillGraph();

	// Обработчик графа маршрутов
	graph::Router<double> router(transport_router.GetGraph());

	// Отрисовщик карты маршрутов в формате SVG
	map_renderer::MapRenderer renderer(queries.render_settings);

	// Обработчик запросов
	request_handler::RequestHandler request_handler(catalogue, renderer, transport_router, router);

	// Вывод запросов в формате json
	json::Document json_responce = request_handler.GetJsonResponce(queries.requests);
	json::Print(json_responce, cout);


}