/*
 * The MIT License
 *
 * Copyright 2023 Chistyakov Alexander.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _MMAP_CONTAINERS_MFCNT_UTILS_H
#define _MMAP_CONTAINERS_MFCNT_UTILS_H

extern "C" {
    #include <fcntl.h>
    #include <string.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
}

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>

#include "mfcnt/types.h"

namespace mfcnt {
namespace details {
namespace utils {

struct mmap_options
{
    mmap_options()
        : fd(-1)
    {}

    /// File descriptor.
    int fd;

    /// The offset to start mapping the file to align with the size
    /// of the memory page.
    size_t offset;

    /// Desired memory protection of the mapping.
    int prot;

    /// Determines whether updates to the mapping are visible to other
    /// processes mapping the same region, and whether updates are carried
    /// through to the underlying file.
    int flags;
};

template<typename TPtr, size_t TBufSize>
struct mmap_buffer
{
    typedef TPtr    pointer;

    BufMapper()
        : open_flags(-1)
        , p_cur_buf(NULL)
        , cur_buf_num(0)
    {}

    /// @brief  Mapping file to buffer.
    /// @param  buf_num - the number of the "segment" of the file to be mapping in memory.
    /// @return Pointer to mapping in memory.
    pointer mmap(const size_t buf_num) const
    {
        assert(opts.fd != -1);

        if (buf_num == cur_buf_num && p_cur_buf) {
            return p_cur_buf;
        }

        p_cur_buf = (pointer)::mmap64(p_cur_buf, TBufSize, opts.prot, opts.flags,
                                      opts.fd, opts.offset + buf_num * TBufSize);
        if (p_addr == MAP_FAILED) {
            throw std::runtime_error("mmap: error map file to memory: " + str_error_r(errno));
        }
        cur_buf_num = buf_num;
        return p_cur_buf;
    }

    /// @brief  Open the file.
    /// @param  path  - path to file.
    /// @param  m - open file mode.
    /// @throw  std::runtime_error if can not open file.
    void open(const std::string& path, const mode m)
    {
        file_path = path;
        open_flags = O_CLOEXEC | O_LARGEFILE;
        open_flags = open_flags | (m == mode::R_ONLY) ? O_RDONLY : O_RDWR;

        opts.fd = ::open(file_path.data(), open_flags);
        if (opts.fd == -1) {
            throw std::runtime_error("open: error open file: " + str_error_r(errno));
        }

        if (m == mode::R_ONLY) {
            opts.prot = PROT_READ;
            opts.flags = MAP_SHARED | MAP_FILE;
        } else if (m == mode::RW_PRIVATE) {
            opts.prot = PROT_READ | PROT_WRITE;
            opts.flags = MAP_PRIVATE | MAP_FILE;
        } else { // if (m == mode::RW_SHARED) {
            opts.prot = PROT_READ | PROT_WRITE;
            opts.flags = MAP_SHARED | MAP_FILE;
        }
    }

    void munmap() const
    {
        if (p_cur_buf != nullptr) {
            ::munmap(p_cur_buf, TBufSize);
        }

        p_cur_buf = nullptr;
        cur_buf_num = 0;
    }

    static std::string str_error_r(int error_code)
    {
        const static size_t buff_size = 1024;
        char err_buffer[buff_size];
        char *str_err = nullptr;

    #if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
        if (::strerror_r(error_code, err_buffer, buff_size) == 0) {
            str_err = err_buffer;
        }
    #else
        str_err = ::strerror_r(error_code, err_buffer, buff_size);
    #endif

        if (! str_err) {
            return std::string("Invalid errno code '" + std::to_string(error_code) + "'");
        }

        return std::string(str_err) + " (" + std::to_string(error_code) + ")";
    }

    mmap_options opts;

    std::string file_path;
    int open_flags;

    mutable pointer p_cur_buf;
    mutable size_t cur_buf_num;
};

} // namespace utils
} // namespace details
} // namespace mfcnt

#endif /* _MMAP_CONTAINERS_MFCNT_UTILS_H */

