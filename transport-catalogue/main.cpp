#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

#include "json_builder.h"
#include <iostream>

#include <fstream>


using namespace std;

int main() {

    // ���� � �������� json_builder
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


        //json::Builder{}.StartDict().Build();  // ������� 3
        //json::Builder{}.StartDict().Key("1"s).Value(1).Value(1);  // ������� 2
        //json::Builder{}.StartDict().Key("1"s).Key(""s);  // ������� 1
        //json::Builder{}.StartArray().Key("1"s);  // ������� 4
        //json::Builder{}.StartArray().EndDict();  // ������� 4
        //json::Builder{}.StartArray().Value(1).Value(2).EndDict();  // ������� 5 
    }
    */

    // ����� ����� � ����������
    {
        /*
        * ��������� ��������� ���������:
        *
        * ������� JSON �� stdin
        * ��������� �� ��� ������ JSON ���� ������ ������������� �����������
        * ��������� ������� � �����������, ����������� � ������� "stat_requests", �������� JSON-������
        * � ��������.
        * ������� � stdout ������ � ���� JSON
        */

        // ��������� �������� �� json
        transport_catalogue::TransportCatalogue db;

        // ��������� ����� ���������
        map_renderer::MapRenderer renderer;

        // �������� ����� ��� ������
        //ifstream fin("input.json"s);

        // �������� ����� ��� ������
        //ofstream fout("output.json"s);

        json_reader::ReadJson(db, renderer, cin, cout);

        // ������� ����
        //fin.close();

        // ������� ����
        //fout.close();

    }









}