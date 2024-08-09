#include "DICOM2CIMG.h"

#include <memory>
#include <iostream>
#include <fstream>

#include <map>



int main() {
    std::cout << "Hello from NTComp!\n";

    DICOM2CIMG a(true);

    a.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1589996189771");
    a.convert("/media/hamster/Hamster Old/NTWI/OurSet");

}
