#pragma once

// Чтение запросов на вывод и сам вывод

#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue {

namespace stat_reader{

// Считывание входного потока для возврата запрашиваемой информации
void ReadQuery(TransportCatalogue& catalog, std::istream& in);

} // Конец stat_reader

} // End Of transport_catalog