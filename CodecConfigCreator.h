#include "CSVRow.h"
#include <filesystem>
#include <string.h>
#include <format>
#include <regex>

class ConfigData
{
private:
    using s = std::string;
    using sv = std::string_view;

public:
    const s m_name, m_width, m_height, m_depth;

    ConfigData(const s &name, const s &width, const s &height, const s &depth)
        : m_name(name), m_width(width), m_height(height), m_depth(depth) {}

    ConfigData(sv name, sv width, sv height, sv depth)
        : m_name(s(name)), m_width(s(width)), m_height(s(height)), m_depth(s(depth)) {}

    enum Field
    {
        Name = 0,
        Width = 3,
        Height = 4,
        Depth = 5,
    };
};

class CodecConfigCreator
{
private:
    bool m_jp3d, m_avc, m_hevc, m_vvc;

    std::string create_config_hevc(const ConfigData &configData) const
    {
        std::string config(
            R"(
InputFile: XnameX.raw
BitstreamFile: XnameX.265e
SourceWidth: XwidthX
SourceHeight: XheightX
FramesToBeEncoded: XdepthX
FrameRate: 1
Profile: monochrome
InputChromaFormat: 400
CostMode: lossless
TransquantBypassEnable: 1
CUTransquantBypassFlagForce: 1
GOPSize: 1
IntraPeriod: 1
LoopFilterDisable: 1
ConstrainedIntraPred: 1
SAO: 0
QuadtreeTULog2MaxSize: 5
            )");
        config = std::regex_replace(config, std::regex("XnameX"), configData.m_name);
        config = std::regex_replace(config, std::regex("XwidthX"), configData.m_width);
        config = std::regex_replace(config, std::regex("XheightX"), configData.m_height);
        config = std::regex_replace(config, std::regex("XdepthX"), configData.m_depth);
        return config;
    }

    std::string create_config_avc_enc(const ConfigData &configData) const
    {
        std::string config(
            R"(
InputFile = "XnameX.raw"
ReconFile = "XnameX_rec.raw"
OutputFile = "XnameX.264e"
SourceWidth = XwidthX
SourceHeight = XheightX
OutputWidth = XwidthX
OutputHeight = XheightX
FramesToBeEncoded = XdepthX
            )");
        config = std::regex_replace(config, std::regex("XnameX"), configData.m_name);
        config = std::regex_replace(config, std::regex("XwidthX"), configData.m_width);
        config = std::regex_replace(config, std::regex("XheightX"), configData.m_height);
        config = std::regex_replace(config, std::regex("XdepthX"), configData.m_depth);
        return config;
    }

    std::string create_config_avc_dec(const ConfigData &configData) const
    {
        std::string config(
            R"(
InputFile = "XnameX.264e" 
OutputFile = "XnameX.264d" 
RefFile = "XnameX_rec.raw" 
WriteUV = 0 
FileFormat = 0 
RefOffset = 0 
POCScale = 2 
DisplayDecParams = 0 
ConcealMode = 0 
RefPOCGap = 2 
POCGap = 2 
Silent = 0 
IntraProfileDeblocking = 1 
DecFrmNum = 0 
DecodeAllLayers = 0 
            )");
        config = std::regex_replace(config, std::regex("XnameX"), configData.m_name);
        return config;
    }

    std::string create_config_vvc_enc(const ConfigData &configData) const
    {
        std::string config(
            R"(
InputFile: XnameX.raw
BitstreamFile: XnameX.266e
SourceWidth: XwidthX
SourceHeight: XheightX
FramesToBeEncoded: XdepthX
FrameRate: 1
Profile: auto
InputBitDepth: 8
InputChromaFormat: 400
TransformSkip: 1
TransformSkipFast: 1
TransformSkipLog2MaxSize: 5
GOPSize: 1
IntraPeriod: 1
DecodingRefreshType: 1
DisableLoopFilterAcrossTiles: 1
DisableLoopFilterAcrossSlices: 1
CostMode : lossless
BDPCM : 0
ChromaTS : 1
DepQuant : 0
RDOQ : 0
RDOQTS : 0
SBT : 0
LMCSEnable : 0
ISP : 0
MTS : 0
LFNST : 0
JointCbCr : 0
DeblockingFilterDisable : 1
SAO : 0
ALF : 0
CCALF : 0
DMVR : 0
BIO : 0
PROF : 0
Log2MaxTbSize : 5
InternalBitDepth : 0
TSRCdisableLL : 1
            )");
        config = std::regex_replace(config, std::regex("XnameX"), configData.m_name);
        config = std::regex_replace(config, std::regex("XwidthX"), configData.m_width);
        config = std::regex_replace(config, std::regex("XheightX"), configData.m_height);
        config = std::regex_replace(config, std::regex("XdepthX"), configData.m_depth);
        return config;
    }

    std::string create_config_jp3d_enc(const ConfigData &configData) const
    {
        std::string config("./jp3d -c --size=XwidthX,XheightX,XdepthX --levels=4,4,2 --bitrates=- XnameX.raw XnameX.jp3de");
        config = std::regex_replace(config, std::regex("XnameX"), configData.m_name);
        config = std::regex_replace(config, std::regex("XwidthX"), configData.m_width);
        config = std::regex_replace(config, std::regex("XheightX"), configData.m_height);
        config = std::regex_replace(config, std::regex("XdepthX"), configData.m_depth);
        return config;
    }

public:
    CodecConfigCreator(bool jp3d, bool avc, bool hevc, bool vvc)
        : m_jp3d(jp3d), m_avc(avc), m_hevc(hevc), m_vvc(vvc) {}

    bool run(const std::filesystem::path &collection_dir)
    {
        namespace fs = std::filesystem;
        try
        {
            for (const auto &entry : fs::recursive_directory_iterator(collection_dir))
            {
                if (entry.is_regular_file() && entry.path().filename() == "conv_metadata.csv")
                {
                    std::vector<ConfigData> configDatas;
                    std::ifstream csvFile(entry.path().string());
                    if (csvFile)
                    {
                        CSVRow row;
                        row.readNextRow(csvFile); // Skip the header
                        while (row.readNextRow(csvFile))
                        {
                            using CF = ConfigData::Field;
                            configDatas.push_back(
                                ConfigData(
                                    row[CF::Name],
                                    row[CF::Width],
                                    row[CF::Height],
                                    row[CF::Depth]));
                        }
                    }
                    else
                    {
                        // Weird ifstream error reporting part
                        std::cerr << "Error: " << strerror(errno);
                        return false;
                    }
                    // Now we have all the data from the csv; let us create configs
                    if (m_avc)
                    {
                        // AVC reference software JM encoder and decoder configs
                        // ---
                        // Note: the default config is the supplied lossless.cfg264e
                        // The created configs are only meant for resetting the defaults
                        // ---
                        for (const auto &configData : configDatas)
                        {
                            { // Encoder config
                                std::ofstream config(entry.path().parent_path() / (configData.m_name + ".cfg264e"));
                                if (config)
                                {
                                    config << create_config_avc_enc(configData);
                                }
                                else
                                {
                                    std::cerr << "Error creating AVC encoder config: " << strerror(errno);
                                    return false;
                                }
                            }
                            { // Decoder config
                                std::ofstream config(entry.path().parent_path() / (configData.m_name + ".cfg264d"));
                                if (config)
                                {
                                    config << create_config_avc_dec(configData);
                                }
                                else
                                {
                                    std::cerr << "Error creating AVC decoder config: " << strerror(errno);
                                    return false;
                                }
                            }
                        }
                    }
                    if (m_hevc)
                    {
                        // HEVC reference software HM encoder configs
                        for (const auto &configData : configDatas)
                        {
                            std::ofstream config(entry.path().parent_path() / (configData.m_name + ".cfg265e"));
                            if (config)
                            {
                                config << create_config_hevc(configData);
                            }
                            else
                            {
                                std::cerr << "Error creating HEVC encoder config: " << strerror(errno);
                                return false;
                            }
                        }
                    }
                    if (m_vvc)
                    {
                        // VVC reference software VTM encoder configs
                        for (const auto &configData : configDatas)
                        {
                            std::ofstream config(entry.path().parent_path() / (configData.m_name + ".cfg266e"));
                            if (config)
                            {
                                config << create_config_vvc_enc(configData);
                            }
                            else
                            {
                                std::cerr << "Error creating VVC encoder config: " << strerror(errno);
                                return false;
                            }
                        }
                    }
                    if (m_jp3d)
                    {
                        // JP3D compression scripts
                        for (const auto &configData : configDatas)
                        {
                            std::ofstream config(entry.path().parent_path() / (configData.m_name + ".sh"));
                            if (config)
                            {
                                config << create_config_jp3d_enc(configData);
                            }
                            else
                            {
                                std::cerr << "Error creating JP3D config: " << strerror(errno);
                                return false;
                            }
                        }
                    }
                }
            }
        }
        catch (...) // Too lazy to check what it can throw, just give me the error code
        {
            std::exception_ptr p = std::current_exception();
            std::cerr << (p ? p.__cxa_exception_type()->name() : "null") << std::endl;
            return false;
        }
        return true;
    }
};