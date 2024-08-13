#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <map>

#include "CImg.h"
#include "CSVRow.h"
#include "FileMetadata.h"

class DICOM2CIMG
{
private:
    using Img = cimg_library::CImg<unsigned char>;
    using Hist = cimg_library::CImg<float>;

    int m_max_mod_occurs;
    int m_min_slices;
    bool m_pack_histograms;

    std::filesystem::path m_manifest_dir;
    std::vector<FileMetadata> m_metadatas;
    std::map<std::string, unsigned int> m_modality_occurrences;

    Hist get_histogram(const Img &image, int num_bins = 256) const
    {
        Hist histogram = image.get_histogram(num_bins, 0, 255);
        return histogram;
    }

    bool is_sparse_histogram(const Hist &histogram, float threshold = 0.1f) const
    {
        int num_bins = histogram.size();
        int under_threshold_bins = 0;

        // Count the number of bins with values under the threshold
        for (int i = 0; i < num_bins; ++i)
        {
            if (histogram[i] <= threshold)
            {
                ++under_threshold_bins;
            }
        }

        // Define sparsity as having more than half of the bins under the threshold
        return under_threshold_bins > num_bins / 2;
    }

    bool is_sparse_histogram(const FileMetadata &metadata, int num_bins = 256) const
    {
        return metadata.m_active_levels > num_bins / 2;
    }

    Img pack_volumetric_image(const Img &image, const Hist &histogram, float threshold = 0.1f) const
    {
        // Create the mapping from original bins to packed bins
        std::map<int, int> bin_mapping;
        int packed_bin_index = 0;

        // Only add mappings for active levels
        for (int i = 0; i < histogram.size(); ++i)
        {
            if (histogram[i] > threshold)
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
        int num_bins = histogram.size();
        int active_bins = 0;
        float min_active = 1.0f;
        float max_active = 0.0f;

        for (int i = 0; i < num_bins; ++i)
        {
            float bin_val = histogram[i];

            if (bin_val > 0.0f)
            {
                ++active_bins;
                if (bin_val > max_active)
                    max_active = bin_val;
                if (bin_val < min_active)
                    min_active = bin_val;
            }
        }
        metadata.m_active_levels = active_bins;

        float active_normalized = (float)active_bins / (float)num_bins;
        float histogram_usage = active_normalized / (1.0f + max_active - min_active);
        metadata.m_histogram_usage = histogram_usage;
    }

public:
    DICOM2CIMG(int max_mod_occurs, int min_slices, bool pack_histograms)
        : m_max_mod_occurs(max_mod_occurs), m_min_slices(min_slices), m_pack_histograms(pack_histograms)
    {
    }

    DICOM2CIMG(const DICOM2CIMG &) = delete;

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
            // Count occurrences
            m_modality_occurrences[metadata.m_modality] = m_modality_occurrences[metadata.m_modality] + 1;
            if (m_modality_occurrences[metadata.m_modality] > m_max_mod_occurs)
            {
                // We reached the quota for this modality
                continue;
            }

            // Count slices
            if (std::stoi(metadata.m_slices) < m_min_slices)
            {
                // Not enough slices
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
                std::to_string(m_modality_occurrences[metadata.m_modality]) +
                ".cimg";
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
                        std::cout << "Slice dimensions: "
                                  << image.width() << " x; "
                                  << image.height() << " y; "
                                  << std::endl;

                        volumetric_image = Img(image.width(), image.height(), 1, image.spectrum(), 0);
                    }

                    volumetric_image.append(image, 'z');
                }

                // The volumetric image now contains all slices
                std::cout << "Volumetric image dimensions: "
                          << volumetric_image.width() << "x; "
                          << volumetric_image.height() << "y; "
                          << volumetric_image.depth() << "z; "
                          << std::endl;

                volumetric_image.save_cimg(destination_file.c_str());

                // Calculate histogram usage and update metadata with it
                Hist histogram = get_histogram(volumetric_image);
                calculate_histogram_usage(histogram, metadata);

                // Pack the image if necessary
                int did_pack = 0;
                if (m_pack_histograms)
                {
                    // Doing it two-way because the methods are equivocal
                    if (is_sparse_histogram(metadata) || is_sparse_histogram(histogram))
                    {
                        Img packed_image = pack_volumetric_image(volumetric_image, histogram);
                        did_pack = 1;
                        packed_image.save_cimg(destination_file_packed.c_str());
                    }
                }

                // Update metadata with the remaining parameters
                metadata.set_image_params(
                    file_name,
                    volumetric_image.width(),
                    volumetric_image.height(),
                    volumetric_image.depth(),
                    did_pack);
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
