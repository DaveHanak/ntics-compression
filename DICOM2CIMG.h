#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <map>

#include "CImg.h"
#include "CSVRow.h"
#include "FileMetadata.h"

enum ImageFormat
{
    CIMG,
    RAW
};

class DicomConverter
{
private:
    using Img = cimg_library::CImg<unsigned char>;
    using Hist = cimg_library::CImg<float>;

    int m_max_mod_occurs;
    int m_min_slices, m_min_slices_us;
    bool m_pack_histograms, m_copy_originals;
    ImageFormat m_format;

public:
    DicomConverter(
        int max_mod_occurs,
        int min_slices,
        int min_slices_us,
        bool pack_histograms,
        bool copy_originals,
        ImageFormat format) : m_max_mod_occurs(max_mod_occurs),
                              m_min_slices(min_slices),
                              m_min_slices_us(min_slices_us),
                              m_pack_histograms(pack_histograms),
                              m_copy_originals(copy_originals),
                              m_format(format)
    {
    }

    DicomConverter(const DicomConverter &) = delete;

private:
    std::filesystem::path m_manifest_dir;
    std::vector<FileMetadata> m_metadatas;
    std::map<std::string, unsigned int> m_modality_occurrences;

    Hist get_histogram(const Img &image, int num_bins = 256) const
    {
        Hist histogram = image.get_histogram(num_bins, 0, 255);
        return histogram;
    }

    bool is_sparse_histogram(const FileMetadata &metadata, int num_bins = 256) const
    {
        return metadata.m_active_levels < num_bins;
    }

    Img pack_volumetric_image(const Img &image, const Hist &histogram) const
    {
        // Create the mapping from original bins to packed bins
        std::map<int, int> bin_mapping;
        int packed_bin_index = 0;

        // Only add mappings for active levels
        for (int i = 0; i < histogram.size(); ++i)
        {
            if (histogram[i] > 0)
            {
                bin_mapping[i] = packed_bin_index++;
            }
        }

        // Create a new image for the packed image
        Img packed_image(image.width(), image.height(), image.depth(), image.spectrum());

        // Apply the mapping to the image
        cimg_forXYZC(image, x, y, z, c)
        {
            packed_image(x, y, z, c) = bin_mapping[image(x, y, z, c)];
        }

        return packed_image;
    }

    void calculate_histogram_usage(const Hist &histogram, FileMetadata &metadata)
    {
        int active_bins = 0;
        int min_active_bin = -1;
        int max_active_bin;

        for (int i = 0; i < histogram.size(); ++i)
        {
            float bin_val = histogram[i];

            if (bin_val > 0)
            {
                if (min_active_bin == -1)
                    min_active_bin = i;
                ++active_bins;
                max_active_bin = i;
            }
        }
        metadata.m_active_levels = active_bins;
        metadata.m_histogram_usage = (float)active_bins / (float)(1 + max_active_bin - min_active_bin);
    }

public:
    bool load_metadatas(const std::filesystem::path &manifest_dir)
    {
        namespace fs = std::filesystem;
        try
        {
            for (const auto &entry : fs::directory_iterator(manifest_dir))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".csv")
                {
                    std::ifstream csvFile(entry.path().string());
                    if (csvFile)
                    {
                        CSVRow row;
                        row.readNextRow(csvFile); // Skip the header
                        while (row.readNextRow(csvFile))
                        {
                            using FMF = FileMetadata::Field;
                            m_metadatas.push_back(
                                FileMetadata(
                                    row[FMF::Collection],
                                    row[FMF::Modality],
                                    row[FMF::Slices],
                                    row[FMF::Folder]));
                            m_modality_occurrences.insert(
                                std::pair<std::string, unsigned int>(
                                    row[FMF::Modality],
                                    0));
                        }
                        break;
                    }
                    else
                    {
                        // Weird ifstream error reporting part
                        std::cerr << "Error: " << strerror(errno);
                        return false;
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

        m_manifest_dir = manifest_dir;
        return true;
    }

    bool convert(const std::filesystem::path &collections_dir, const std::string &collection_name)
    {
        namespace fs = std::filesystem;

        // Prepare the directory for our collection
        fs::path collection_dir = collections_dir / collection_name;
        if (!fs::exists(collection_dir))
        {
            if (!fs::create_directory(collection_dir))
            {
                std::cerr << "Unable to create collection directory" << std::endl;
                return false;
            }
        }

        if (m_metadatas.empty())
        {
            std::cerr << "No metadatas loaded" << std::endl;
            return false;
        }

        // Update the metadata at the very end so no const
        for (auto &metadata : m_metadatas)
        {
            // Check if enough slices
            int meta_slices = std::stoi(metadata.m_slices);
            if (metadata.m_modality != "US" && meta_slices < m_min_slices ||
                metadata.m_modality == "US" && meta_slices < m_min_slices_us)
            {
                // Not enough slices
                continue;
            }

            // Count occurrences
            if (m_modality_occurrences[metadata.m_modality] > m_max_mod_occurs - 1)
            {
                // We reached the quota for this modality
                continue;
            }

            // Fix the path
            fs::path file_dir = m_manifest_dir / metadata.m_folder.substr(2);
            if (!fs::is_directory(file_dir))
            {
                std::cerr << file_dir << " is not a directory" << std::endl;
                return false;
            }

            // Iterate over all the image slice entries in the directory and put them in a vector
            std::vector<fs::directory_entry> entries;
            for (const auto &entry : fs::directory_iterator(file_dir))
            {
                if (entry.is_regular_file())
                {
                    entries.push_back(entry);
                }
            }

            // We have to get the slices in order so let's sort the entries vector
            std::sort(entries.begin(), entries.end(), [](const fs::directory_entry &a, const fs::directory_entry &b)
                      { return a.path().filename() < b.path().filename(); });

            // The result file name
            std::string file_name =
                metadata.m_collection + "_" +
                metadata.m_modality + "_" +
                std::to_string(m_modality_occurrences[metadata.m_modality] + 1);
            fs::path destination_file = collection_dir / file_name;

            // Prepare the directory for packed images
            fs::path destination_packed = collection_dir / "packed";
            if (m_pack_histograms && !fs::exists(destination_packed))
            {
                if (!fs::create_directory(destination_packed))
                {
                    std::cerr << "Unable to create packed directory" << std::endl;
                    return false;
                }
            }
            fs::path destination_file_packed = destination_packed / file_name;

            // Iterate over the sorted slices, append them to the result image
            try
            {
                Img volumetric_image;
                for (const auto &entry : entries)
                {
                    Img image = Img::get_load_medcon_external(entry.path().c_str());

                    if (image.is_empty())
                    {
                        std::cerr << "Unexpected empty image; skipping" << std::endl;
                        continue;
                    }

                    if (image.depth() > 1)
                    {
                        std::cerr << "Unexpected 3D layers in 2D image; skipping" << std::endl;
                        continue;
                    }

                    if (image.spectrum() > 1)
                    {
                        image = image.get_RGBtoYCbCr().get_channel(0);
                    }

                    if (volumetric_image.is_empty())
                    {
                        volumetric_image = Img(image.width(), image.height(), 1, image.spectrum(), 0);
                    }

                    volumetric_image.append(image, 'z');
                }

                // The volumetric image now contains all slices
                switch (m_format)
                {
                case CIMG:
                    volumetric_image.save_cimg(destination_file.append(".cimg").c_str());
                    break;
                case RAW:
                    volumetric_image.save_raw(destination_file.append(".raw").c_str());
                    break;
                default:
                    std::cerr << "Unsupported format" << std::endl;
                    return false;
                }

                // Calculate histogram usage and update metadata with it
                Hist histogram = get_histogram(volumetric_image);
                calculate_histogram_usage(histogram, metadata);

                // Pack the image if necessary
                int did_pack = 0;
                if (m_pack_histograms)
                {
                    // Doing it two-way because the methods are equivocal
                    if (is_sparse_histogram(metadata))
                    {
                        Img packed_image = pack_volumetric_image(volumetric_image, histogram);
                        did_pack = 1;
                        switch (m_format)
                        {
                        case CIMG:
                            packed_image.save_cimg(destination_file_packed.append(".cimg").c_str());
                            break;
                        case RAW:
                            packed_image.save_raw(destination_file_packed.append(".raw").c_str());
                            break;
                        default:
                            std::cerr << "Unsupported format" << std::endl;
                            return false;
                        }
                    }
                }

                // Update metadata with the remaining parameters
                metadata.set_image_params(
                    file_name,
                    volumetric_image.width(),
                    volumetric_image.height(),
                    volumetric_image.depth(),
                    did_pack);

                // Grand finish
                metadata.m_converted = true;
                m_modality_occurrences[metadata.m_modality] = m_modality_occurrences[metadata.m_modality] + 1;
                if (m_copy_originals) std::filesystem::copy(file_dir, collection_dir / file_name, std::filesystem::copy_options::recursive);
            }
            catch (...) // Skip images with any kinds of problems
            {
                std::exception_ptr p = std::current_exception();
                std::cerr << (p ? p.__cxa_exception_type()->name() : "null") << std::endl;
            }
        }

        // Create our own metadata csv so we know what's what
        fs::path converted_metadatas = collection_dir / "conv_metadata.csv";
        std::ofstream conv_metadata(converted_metadatas);
        if (conv_metadata)
        {
            conv_metadata << FileMetadata::get_info_header();
            for (const auto &m : m_metadatas)
            {
                if (m.m_converted)
                    conv_metadata << m.get_info() << "\n";
            }
        }
        else
        {
            std::cerr << "Could not create final csv" << std::endl;
        }

        return true;
    }
};
