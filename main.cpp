#include "DICOM2CIMG.h"

int main()
{
    std::cout << "Hello from NTComp!\n";
    using F = ImageFormat;
    {
        // QIN-BREAST -> manifest-1542731172463
        DicomConverter QINBREAST(8, 50, 3, false, true, F::RAW);
        QINBREAST.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1542731172463");
        QINBREAST.convert("/media/hamster/Hamster Old/NTWI/OurSet", "QIN-BREAST");
    }
    {
        // CPTAC-LUAD -> manifest-1677266397124
        DicomConverter CPTACLUAD(8, 50, 3, false, true, F::RAW);
        CPTACLUAD.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1677266397124");
        CPTACLUAD.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-LUAD");
    }
    {
        // CPTAC-SAR  -> manifest-1677267704131
        DicomConverter CPTACSAR(8, 50, 3, false, true, F::RAW);
        CPTACSAR.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1677267704131");
        CPTACSAR.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-SAR");
    }
    {
        // CPTAC-PDA  -> manifest-1692386697723
        DicomConverter CPTACPDA(8, 50, 3, false, true, F::RAW);
        CPTACPDA.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1692386697723");
        CPTACPDA.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-PDA");
    }
    {
        // CPTAC-UCEC -> manifest-1712342731330
        DicomConverter CPTACUCEC(8, 50, 3, false, true, F::RAW);
        CPTACUCEC.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1712342731330");
        CPTACUCEC.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-UCEC");
    }
    {
        // CMB-CRC    -> manifest-1722776407088
        DicomConverter CMBCRC(8, 50, 3, false, true, F::RAW);
        CMBCRC.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1722776407088");
        CMBCRC.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CMB-CRC");
    }
    {
        // CMB-LCA    -> manifest-1722777127284
        DicomConverter CMBLCA(8, 50, 3, false, true, F::RAW);
        CMBLCA.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1722777127284");
        CMBLCA.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CMB-LCA");
    }

    std::cout << "Goodbye from NTComp!\n";
    
    // cheatsheet
    // QIN-BREAST -> manifest-1542731172463
    // CPTAC-LUAD -> manifest-1677266397124
    // CPTAC-SAR  -> manifest-1677267704131
    // CPTAC-PDA  -> manifest-1692386697723
    // CPTAC-UCEC -> manifest-1712342731330
    // CMB-CRC    -> manifest-1722776407088
    // CMB-LCA    -> manifest-1722777127284
}
