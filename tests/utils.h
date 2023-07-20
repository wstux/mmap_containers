#ifndef _CONTAINERS_TESTS_UTILS_H
#define _CONTAINERS_TESTS_UTILS_H

#include <filesystem>
#include <fstream>
#include <regex>

namespace tests {

const std::string kTestData = "\
We don't need no education.\n\
We don't need no thought control.\n\
No dark sarcasm in the classroom.\n\
Teacher, leave those kids alone.\n\
Hey, Teacher, leave those kids alone!\n\
All in all it's just another brick in the wall.\n\
All in all you're just another brick in the wall.\n\
\n";

namespace details {

struct utils final
{
    template<typename TCnt>
    static TCnt create_stl_cnt(const std::filesystem::path& file)
    {
        size_t size = std::filesystem::file_size(file);
        TCnt cnt;
        std::ifstream fin(file);
        if (! fin.is_open()) {
            return cnt;
        }

        std::vector<char> buffer(1024);
        while (size != 0) {
            if (size < buffer.size()) {
                fin.read(buffer.data(), size);
                cnt.insert(cnt.end(), buffer.begin(), buffer.begin() + size);
                size = 0;
            } else {
                fin.read(buffer.data(), buffer.size());
                cnt.insert(cnt.end(), buffer.begin(), buffer.end());
                size -= buffer.size();
            }
        }
        return cnt;
    }

    static bool create_test_file(const std::filesystem::path& file,
                                 const std::string& td, size_t size)
    {
        std::ofstream fout(file);
        if (! fout.is_open()) {
            return false;
        }

        while (size != 0) {
            if (size < td.size()) {
                fout << td.substr(0, size);
                size = 0;
            } else {
                fout << td;
                size -= td.size();
            }
        }

        return true;
    }

    static bool create_test_file(const std::filesystem::path& file, size_t size)
    {
        return create_test_file(file, kTestData, size);
    }

    static size_t fd_count()
    {
        namespace fs = std::filesystem;

        static const fs::path fd_path = "/proc/self/fd";

        size_t fd_count = 0;
        std::regex filter_re("[0-9]{1,5}");
        for (const fs::path& dir_entry : fs::directory_iterator(fd_path)) {
            if (! std::regex_search(dir_entry.filename().string(), filter_re)) {
                continue;
            }
            ++fd_count;
        }
        return fd_count;
    }

    static int mem_usage()
    {
        static const std::string statm_path = "/proc/self/statm";
        unsigned long dummu;
        unsigned long resident;

        FILE *f = fopen(statm_path.c_str(), "r");
        if (!f) {
            return -1;
        }

        if (fscanf(f,"%lu %lu", &dummu, &resident) != 2) {
            resident = -1;
        }
        fclose(f);
        return resident;
    }
};

} // namespace details
} // namespace tests

#endif /* _CONTAINERS_TESTS_UTILS_H */

