#include <string>

class FileMetadata
{
private:
    using s = std::string;
    using sv = std::string_view;

public:
    s m_collection, m_modality, m_folder, m_result_name;
    int m_width, m_height, m_depth, m_is_packed;

    FileMetadata(s &c, s &m, s &f)
        : m_collection(c), m_modality(m), m_folder(f)
    {
    }

    FileMetadata(sv c, sv m, sv f)
        : m_collection(s(c)), m_modality(s(m)), m_folder(s(f))
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

    s get_info() const
    {
        return m_result_name
        + "," + m_modality
        + "," + std::to_string(m_width)
        + "," + std::to_string(m_height)
        + "," + std::to_string(m_depth)
        + "," + std::to_string(m_is_packed);
    }

    enum Field
    {
        Collection = 1,
        Modality = 10,
        Folder = 15,
    };
};