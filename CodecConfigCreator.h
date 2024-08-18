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

        std::string create_config_avc(const ConfigData &configData) const
    {
        std::string config(
            R"(
InputFile: XnameX.raw
BitstreamFile: XnameX.265
SourceWidth: XwidthX
SourceHeight: XheightX
FramesToBeEncoded: XdepthX
FrameRate: 1
Profile: monochrome
InputChromaFormat: 400
CostMode: lossless
QP: 0
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

    std::string create_config_hevc(const ConfigData &configData) const
    {
        std::string config(
            R"(
InputFile: XnameX.raw
BitstreamFile: XnameX.265
SourceWidth: XwidthX
SourceHeight: XheightX
FramesToBeEncoded: XdepthX
FrameRate: 1
Profile: monochrome
InputChromaFormat: 400
CostMode: lossless
QP: 0
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
                        for (const auto &configData : configDatas)
                        {
                            std::ofstream config(entry.path().parent_path() / (configData.m_name + ".cfg264"));
                            if (config)
                            {
                                config << create_config_avc(configData);
                            }
                            else
                            {
                                std::cerr << "Error: " << strerror(errno);
                                return false;
                            }
                        }
                    }
                    if (m_hevc)
                    {
                        for (const auto &configData : configDatas)
                        {
                            std::ofstream config(entry.path().parent_path() / (configData.m_name + ".cfg265"));
                            if (config)
                            {
                                config << create_config_hevc(configData);
                            }
                            else
                            {
                                std::cerr << "Error: " << strerror(errno);
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