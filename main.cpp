#include "DICOM2CIMG.h"

int main() {
    std::cout << "Hello from NTComp!\n";

    DICOM2CIMG a(10, 50, true);

    a.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1677266397124");
    a.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-LUAD");
}
