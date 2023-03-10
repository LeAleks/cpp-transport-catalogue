#pragma once

// Чтение запросов на заполнение базы

#include "transport_catalogue.h"

#include <istream>

namespace transport_catalogue {

namespace input_reader{

// Считывание входного потока для добавления инфомарции в базу
void ReadLines(TransportCatalogue& catalog, std::istream& in);

} // End Of input_reader

} // End Of transport_catalog