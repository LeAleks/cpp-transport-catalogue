#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

#include "json_builder.h"
#include <iostream>

#include <fstream>


using namespace std;

int main() {

    // Вход в проверку json_builder
    /*
    {
        try {
            json::Print(
                json::Document{
                    json::Builder{}
                    .StartDict()
                        .Key("key1"s).Value(123)
                        .Key("key2"s).Value("value2"s)
                        .Key("key3"s).StartArray()
                            .Value(456)
                            .StartDict().EndDict()
                            .StartDict()
                                .Key(""s)
                                .Value(nullptr)
                            .EndDict()
                            .Value(""s)
                        .EndArray()
                    .EndDict()
                    .Build()
                },
                std::cout
            );
            std::cout << endl;

        }
        catch (std::exception const& ex) {
            std::cout << ex.what();
        }

        json::Print(
            json::Document{
                json::Builder{}
                .Value("just a string"s)
                .Build()
            },
            std::cout
        );
        std::cout << "\n"s << endl;


        //json::Builder{}.StartDict().Build();  // правило 3
        //json::Builder{}.StartDict().Key("1"s).Value(1).Value(1);  // правило 2
        //json::Builder{}.StartDict().Key("1"s).Key(""s);  // правило 1
        //json::Builder{}.StartArray().Key("1"s);  // правило 4
        //json::Builder{}.StartArray().EndDict();  // правило 4
        //json::Builder{}.StartArray().Value(1).Value(2).EndDict();  // правило 5 
    }
    */

    // Точка входа в справочник
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