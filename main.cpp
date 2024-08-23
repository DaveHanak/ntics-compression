#define CONVERT_DICOM
//#define CREATE_CONFIGS
// #define SANDBOX

#ifdef CONVERT_DICOM
#include "DicomConverter.h"
#endif
#ifdef CREATE_CONFIGS
#include "CodecConfigCreator.h"
#endif
#ifdef SANDBOX
#include "CImg.h"
#endif

int main()
{
#ifdef SANDBOX
    using namespace cimg_library;
    // experiments here
#endif

#ifdef CONVERT_DICOM
    using F = ImageFormat;
    // {
    //     // QIN-BREAST -> manifest-1542731172463
    //     DicomConverter QINBREAST(8, 50, 3, false, false, F::RAW);
    //     QINBREAST.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1542731172463");
    //     QINBREAST.convert("/media/hamster/Hamster Old/NTWI/OurSet", "QIN-BREAST");
    // }
    // {
    //     // CPTAC-LUAD -> manifest-1677266397124
    //     DicomConverter CPTACLUAD(8, 50, 3, false, false, F::RAW);
    //     CPTACLUAD.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1677266397124");
    //     CPTACLUAD.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-LUAD");
    // }
    // {
    //     // CPTAC-SAR  -> manifest-1677267704131
    //     DicomConverter CPTACSAR(8, 50, 3, false, false, F::RAW);
    //     CPTACSAR.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1677267704131");
    //     CPTACSAR.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-SAR");
    // }
    // {
    //     // CPTAC-PDA  -> manifest-1692386697723
    //     DicomConverter CPTACPDA(8, 50, 3, false, false, F::RAW);
    //     CPTACPDA.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1692386697723");
    //     CPTACPDA.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-PDA");
    // }
    // {
    //     // CPTAC-UCEC -> manifest-1712342731330
    //     DicomConverter CPTACUCEC(8, 50, 3, false, false, F::RAW);
    //     CPTACUCEC.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1712342731330");
    //     CPTACUCEC.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CPTAC-UCEC");
    // }
    // {
    //     // CMB-CRC    -> manifest-1722776407088
    //     DicomConverter CMBCRC(8, 50, 3, false, false, F::RAW);
    //     CMBCRC.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1722776407088");
    //     CMBCRC.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CMB-CRC");
    // }
    // {
    //     // CMB-LCA    -> manifest-1722777127284
    //     DicomConverter CMBLCA(8, 50, 3, false, false, F::RAW);
    //     CMBLCA.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1722777127284");
    //     CMBLCA.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CMB-LCA");
    // }
    {
        // CMB-MEL    -> manifest-1722777127284
        DicomConverter CMBMEL(8, 50, 3, false, false, F::RAW);
        CMBMEL.load_metadatas("/media/hamster/Hamster Old/NTWI/Data/manifest-1722777380915");
        CMBMEL.convert("/media/hamster/Hamster Old/NTWI/OurSet", "CMB-MEL");
    }
#endif

#ifdef CREATE_CONFIGS
    CodecConfigCreator ccc(false, true, false, false);
    ccc.run("/media/hamster/Hamster Old/NTWI/OurSet");
#endif
}
