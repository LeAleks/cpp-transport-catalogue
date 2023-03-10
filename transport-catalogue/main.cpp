// Точка входа в справочник

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

int main() {
	using namespace std;

	transport_catalogue::TransportCatalogue cat;
	transport_catalogue::input_reader::ReadLines(cat, cin);
	transport_catalogue::stat_reader::ReadQuery(cat, cin);

}