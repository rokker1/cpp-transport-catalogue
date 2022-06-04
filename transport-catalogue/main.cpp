#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include "geo.h"

int main () {
    using namespace std;
    using namespace transport::catalogue;
    using namespace transport::input_reader;
    using namespace transport::stat_reader;
    Transport_catalogue transport_catalogue;

    ReadInput(std::cin, transport_catalogue);
    ProcessRequests(std::cin, std::cout, transport_catalogue);

}