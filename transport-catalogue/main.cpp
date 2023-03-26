// Точка входа в справочник

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"


#include <fstream>



int main() {
    using namespace std;

    {
        /*
        * Примерная структура программы:
        *
        * Считать JSON из stdin
        * Построить на его основе JSON базу данных транспортного справочника
        * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
        * с ответами.
        * Вывести в stdout ответы в виде JSON
        */

        // обработка запросов из json
        transport_catalogue::TransportCatalogue db;

        // Отрисовка карты маршрутов
        map_renderer::MapRenderer renderer;

        // Открытие файла для чтения
        //ifstream fin("input.json"s);

        // Открытие файла для записи
        //ofstream fout("output.json"s);

        json_reader::ReadJson(db, renderer, cin, cout);

        // Закрыли файл
        //fin.close();

        // Закрыли файл
        //fout.close();

    }
}

