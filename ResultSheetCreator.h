#include "CSVRow.h"
#include <filesystem>
#include <string.h>
#include <format>
#include <regex>

enum Codec
{
    AVC,
    HEVC,
    VVC,
    JP3D,
}

class Result
{
private:
    using s = std::string;
    using sv = std::string_view;

public:
    s m_name, m_width, m_height, m_depth, m_encoding_time, m_decoding_time;
    double m_encoding_rate, m_decoding_rate, m_bits_per_pixel;

    Result(const s &name, const s &width, const s &height, const s &depth)
        : m_name(name), m_width(width), m_height(height), m_depth(depth) {}

    Result(sv name, sv width, sv height, sv depth)
        : m_name(s(name)), m_width(s(width)), m_height(s(height)), m_depth(s(depth)) {}

    enum FieldConv
    {
        Name = 0,
        Width = 3,
        Height = 4,
        Depth = 5,
    };
    enum FieldTime
    {
        Name = 0,
        Time = 1,
    };

    static inline s get_info_header()
    {
        return "Name,Width,Height,Depth,EncodingTime,EncodingRate,DecodingTime,DecodingRate,BPP\n";
    }

    s get_info() const
    {
        return m_name
        + "," + m_width
        + "," + m_height
        + "," + m_depth
        + "," + m_encoding_time
        + "," + std::to_string(m_encoding_rate)
        + "," + m_decoding_time
        + "," + std::to_string(m_decoding_rate)
        + "," + std::to_string(m_bits_per_pixel);
    }
};

class ResultSheetCreator
{
public:
    bool run(const std::filesystem::path &collection_dir, Codec codec)
    {
        namespace fs = std::filesystem;
        try
        {
            for (const auto &entry : fs::recursive_directory_iterator(collection_dir))
            {
                if (entry.is_regular_file() && entry.path().filename() == "conv_metadata.csv")
                {
                    std::vector<Result> results;
                    { // 1. Open conv_metadata.csv
                        std::ifstream csvFile(entry.path().string());
                        if (csvFile)
                        {
                            CSVRow row;
                            row.readNextRow(csvFile); // Skip the header
                            while (row.readNextRow(csvFile))
                            {
                                using RFC = Result::FieldConv;
                                results.push_back(
                                    Result(
                                        row[RFC::Name],
                                        row[RFC::Width],
                                        row[RFC::Height],
                                        row[RFC::Depth]));
                            }
                        }
                        else
                        {
                            // Weird ifstream error reporting part
                            std::cerr << "Error while opening converted metadata: " << strerror(errno);
                            return false;
                        }
                    }
                    // We assume that we have all the necessary artifacts for producing results
                    fs::path parent_path = entry.path().parent_path();
                    std::string log_enc_name, log_dec_name, enc_ext, result_file_name;
                    // Set codec specific names
                    switch (codec)
                    {
                    case (AVC):
                        log_enc_name = "AVC-enc.log";
                        log_dec_name = "AVC-dec.log";
                        enc_ext = ".264e";
                        result_file_name = "AVC-results.csv";
                        break;
                    case (HEVC):
                        log_enc_name = "HEVC-enc.log";
                        log_dec_name = "HEVC-dec.log";
                        enc_ext = ".265e";
                        result_file_name = "HEVC-results.csv";
                        break;
                    case (VVC):
                        log_enc_name = "VVC-enc.log";
                        log_dec_name = "VVC-dec.log";
                        enc_ext = ".266e";
                        result_file_name = "VVC-results.csv";
                        break;
                    case (JP3D):
                        log_enc_name = "JP3D-enc.log";
                        log_dec_name = "JP3D-dec.log";
                        enc_ext = ".jp3de";
                        result_file_name = "JP3D-results.csv";
                        break;
                    }
                    { // 2. Open encoding log
                        fs::path log_path = parent_path / log_enc_name;
                        std::ifstream csvFile(log_path.string());
                        if (csvFile)
                        {
                            CSVRow row;
                            row.readNextRow(csvFile); // Skip the header
                            while (row.readNextRow(csvFile))
                            {
                                using RFT = Result::FieldTime;
                                using s = std::string;
                                s name = s(row[RFT::Name]);
                                size_t dot_index = name.find_last_of("."); // Remove .cfg*
                                if (dot_index != s::npos)
                                    name = name.substr(0, dot_index);
                                for (auto &result : results)
                                {
                                    if (result.m_name == name)
                                    {
                                        result.m_encoding_time = s(row[RFT::Time]);
                                    }
                                }
                            }
                        }
                        else
                        {
                            // Weird ifstream error reporting part
                            std::cerr << "Error while opening encoding log: " << strerror(errno);
                            return false;
                        }
                    }
                    { // 3. Open decoding log
                        fs::path log_path = parent_path / log_dec_name;
                        std::ifstream csvFile(log_path.string());
                        if (csvFile)
                        {
                            CSVRow row;
                            row.readNextRow(csvFile); // Skip the header
                            while (row.readNextRow(csvFile))
                            {
                                using RFT = Result::FieldTime;
                                using s = std::string;
                                s name = s(row[RFT::Name]);
                                size_t dot_index = name.find_last_of("."); // Remove .cfg*
                                if (dot_index != s::npos)
                                    name = name.substr(0, dot_index);
                                for (auto &result : results)
                                {
                                    if (result.m_name == name)
                                    {
                                        result.m_decoding_time = s(row[RFT::Time]);
                                    }
                                }
                            }
                        }
                        else
                        {
                            // Weird ifstream error reporting part
                            std::cerr << "Error while opening decoding log: " << strerror(errno);
                            return false;
                        }
                    }
                    { // 4. Calculate the rest of the parameters
                        for (auto &result : results)
                        {
                            // Pixels
                            int w = std::stoi(result.m_width);
                            int h = std::stoi(result.m_height);
                            int d = std::stoi(result.m_depth);
                            uintmax_t pixels = w * h * d;

                            // Bits per pixel [bits/pixels]
                            uintmax_t enc_size_in_bits = fs::file_size(result.m_name + enc_ext) * 8;
                            double bpp = (double)enc_size_in_bits / (double)pixels;
                            result.m_bits_per_pixel = bpp;

                            // Encoding rate [10^6 * pixels/seconds]
                            double enc_time = std::stod(result.m_encoding_time);
                            double encoding_rate = ((double)pixels / (double)enc_time) / 1000000.0;
                            result.m_encoding_rate = encoding_rate;

                            // Decoding rate [10^6 * pixels/seconds]
                            double dec_time = std::stod(result.m_decoding_time);
                            double decoding_rate = ((double)pixels / (double)dec_time) / 1000000.0;
                            result.m_decoding_rate = decoding_rate;
                        }
                    }
                    { // 5. Save results in csv
                        fs::path result_file_path = parent_path / result_file_name;
                        std::ofstream result_fstream(result_file_path);
                        if (result_fstream)
                        {
                            result_fstream << Result::get_info_header();
                            for (const auto &r : results)
                            {
                                result_fstream << r.get_info() << "\n";
                            }
                        }
                        else
                        {
                            std::cerr << "Could not create final result csv" << std::endl;
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