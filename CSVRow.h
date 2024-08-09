// stolen from https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class CSVRow
{
private:
    std::string         m_line;
    std::vector<int>    m_data;
public:
    std::string_view operator[](std::size_t index) const
    {
        return std::string_view(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
    }
    std::size_t size() const
    {
        return m_data.size() - 1;
    }
    bool readNextRow(std::istream& str)
    {
        std::getline(str, m_line);

        if (str.eof()) return false;

        m_data.clear();
        m_data.emplace_back(-1);
        std::string::size_type pos = 0;
        while((pos = m_line.find(',', pos)) != std::string::npos)
        {
            m_data.emplace_back(pos);
            ++pos;
        }
        // This checks for a trailing comma with no data after it.
        pos   = m_line.size();
        m_data.emplace_back(pos);

        return true;
    }
};