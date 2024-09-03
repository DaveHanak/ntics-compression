// #define CONVERT_DICOM
 //#define CREATE_CONFIGS
//#define SANDBOX
 #define CREATE_RESULTS_FOR_CODEC

#ifdef CONVERT_DICOM
#include "DicomConverter.h"
#endif
#ifdef CREATE_CONFIGS
#include "CodecConfigCreator.h"
#endif
#ifdef SANDBOX
#include "CImg.h"
#include "FileMetadata.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#endif
#ifdef CREATE_RESULTS_FOR_CODEC
#include "ResultSheetCreator.h"
#endif

int main()
{
#ifdef SANDBOX
    using Img = cimg_library::CImg<unsigned char>;
    namespace fs = std::filesystem;
    fs::path path = "/media/hamster/Hamster Old/NTWI/OurSet/Bruylants";
    std::string bruy = "Bruylants";
    std::vector<FileMetadata> metas;
    for (const auto &entry : fs::directory_iterator(path))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".cimg")
        {
            fs::path parent = entry.path().parent_path();
            std::string newname = entry.path().stem().string() + ".raw";
            fs::path newpath = parent / newname;

            Img img = Img::get_load_cimg(entry.path().c_str());
            img.save_raw(newpath.c_str());

            FileMetadata meta = FileMetadata(bruy, bruy, bruy, bruy);

            std::string fname = newpath.filename();
            meta.set_image_params(
                fname,
                img.width(),
                img.height(),
                img.depth(),
                1);

            meta.m_converted = true;
            metas.push_back(meta);
        }
    }
    fs::path converted_metadatas = path / "conv_metadata.csv";
    std::ofstream conv_metadata(converted_metadatas);
    if (conv_metadata)
    {
        conv_metadata << FileMetadata::get_info_header();
        for (const auto &m : metas)
        {
            if (m.m_converted)
                conv_metadata << m.get_info() << "\n";
        }
    }
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
                        // JP3D   AVC    HEVC    VVC
    CodecConfigCreator ccc(true, true, true, true);
    //ccc.run("/media/hamster/Hamster Old/NTWI/OurSet");
    ccc.run("/media/hamster/Hamster Old/NTWI/OurSet/Bruylants");
#endif

#ifdef CREATE_RESULTS_FOR_CODEC
    ResultSheetCreator rsc;
    rsc.run("/media/hamster/Hamster Old/NTWI/OurSet", JP3D);
    rsc.run("/media/hamster/Hamster Old/NTWI/OurSet", AVC);
    rsc.run("/media/hamster/Hamster Old/NTWI/OurSet", HEVC);
    rsc.run("/media/hamster/Hamster Old/NTWI/OurSet", VVC);
#endif
}
