#include "DICOM2CIMG.h"

int main() {
    std::cout << "Hello from NTComp!\n";

    DICOM2CIMG a(10, 50, true);

    a.load_metadatas("mAanifest");
    a.convert("/media/hamster/Hamster Old/NTWI/OurSet", "Collection1");
}
