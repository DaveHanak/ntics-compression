#include <string>

class FileMetadata
{
private:
    using s = std::string;
    using sv = std::string_view;

public:
    s m_collection, m_modality, m_slices, m_folder, m_result_name;
    int m_width, m_height, m_depth, m_active_levels, m_is_packed;
    float m_histogram_usage;

    FileMetadata(s &c, s &m, s &sl, s &f)
        : m_collection(c), m_modality(m), m_slices(sl), m_folder(f)
    {
    }

    FileMetadata(sv c, sv m, sv sl, sv f)
        : m_collection(s(c)), m_modality(s(m)), m_slices(s(sl)), m_folder(s(f))
    {
    }

    void set_image_params(s &name, int width, int height, int depth, int is_packed)
    {
        m_result_name = name;
        m_width = width;
        m_height = height;
        m_depth = depth;
        m_is_packed = is_packed;
    }

    static inline s get_info_header() {
        return "Name,OriginFolder,Modality,Width,Height,Depth,ActiveLevels,HistogramUsage,HasPackedVersion\n";
    }

    s get_info() const
    {
        return m_result_name
        + "," + m_folder
        + "," + m_modality
        + "," + std::to_string(m_width)
        + "," + std::to_string(m_height)
        + "," + std::to_string(m_depth)
        + "," + std::to_string(m_active_levels)
        + "," + std::to_string(m_histogram_usage)
        + "," + std::to_string(m_is_packed);
    }

    enum Field
    {
        Collection = 1,
        Modality = 10,
        Slices = 13,
        Folder = 15,
    };
};