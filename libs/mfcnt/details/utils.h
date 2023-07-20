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

    mmap_buffer()
        : open_flags(-1)
        , p_cur_buf(nullptr)
        , cur_buf_num(0)
    {}

    mmap_buffer(const std::string& path, const mode m)
        : open_flags(-1)
        , p_cur_buf(nullptr)
        , cur_buf_num(0)
    {
        open(path, m);
    }

    mmap_buffer(const mmap_buffer& orig)
        : open_flags(-1)
        , p_cur_buf(nullptr)
    {
        opts.offset = orig.opts.offset;
        open(orig.file_path, orig.open_flags, orig.opts.prot, orig.opts.flags);
        map(orig.cur_buf_num);
    }

    mmap_buffer(mmap_buffer&& orig)
        : opts(std::move(orig.opts))
        , file_path(std::move(orig.file_path))
        , open_flags(std::move(orig.open_flags))
        , p_cur_buf(std::move(orig.p_cur_buf))
        , cur_buf_num(std::move(orig.cur_buf_num))
    {
        orig.opts.fd = -1;
        orig.p_cur_buf = nullptr;
    }

    /// @brief  Unmap buffer and close the file.
    void close()
    {
        if (! is_open()) {
            return;
        }
        unmap();
        ::close(opts.fd);
        opts.fd = -1;
    }

    /// @brief  File size calculation.
    /// @return File size.
    /// @throw  std::runtime_error if can not get file stat. Before throwing an
    ///         exception, the file will be closed.
    size_t file_size()
    {
        assert(is_open());

        struct ::stat st;
        if (::fstat(opts.fd, &st) == -1) {
            throw std::runtime_error("file_size: error file status: " + str_error_r(errno));
        }

        return st.st_size;
    }

    bool is_open() const { return (opts.fd != -1); }

    /// @brief  Mapping file to buffer.
    /// @param  buf_num - the number of the "segment" of the file to be mapping in memory.
    /// @return Pointer to mapping in memory.
    pointer map(const size_t buf_num) const
    {
        assert(is_open());

        if (buf_num == cur_buf_num && p_cur_buf) {
            return p_cur_buf;
        }

        p_cur_buf = (pointer)::mmap64(p_cur_buf, TBufSize, opts.prot, opts.flags,
                                      opts.fd, opts.offset + buf_num * TBufSize);
        if (p_cur_buf == MAP_FAILED) {
            throw std::runtime_error("map: error map file to memory: " + str_error_r(errno));
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
        int open_fls = O_CLOEXEC | O_LARGEFILE;
        open_fls = open_flags | (m == mode::R_ONLY) ? O_RDONLY : O_RDWR;

        int prot_fls;
        int mmap_fls;
        if (m == mode::R_ONLY) {
            prot_fls = PROT_READ;
            mmap_fls = MAP_SHARED | MAP_FILE;
        } else if (m == mode::RW_PRIVATE) {
            prot_fls = PROT_READ | PROT_WRITE;
            mmap_fls = MAP_PRIVATE | MAP_FILE;
        } else { // if (m == mode::RW_SHARED) {
            prot_fls = PROT_READ | PROT_WRITE;
            mmap_fls = MAP_SHARED | MAP_FILE;
        }

        open(path, open_fls, prot_fls, mmap_fls);
    }

    /// @brief  Open the file.
    /// @param  path  - path to file.
    /// @param  m - open file mode.
    /// @throw  std::runtime_error if can not open file.
    void open(const std::string& path, int open_fls, int prot_fls, int mmap_fls)
    {
        file_path = path;
        open_flags = open_fls;

        opts.prot = prot_fls;
        opts.flags = mmap_fls;

        opts.fd = ::open(file_path.data(), open_flags);
        if (opts.fd == -1) {
            throw std::runtime_error("open: error open file: " + str_error_r(errno));
        }
    }

    void swap(mmap_buffer& orig)
    {
        std::swap(opts, orig.opts);

        std::swap(file_path, orig.file_path);
        std::swap(open_flags, orig.open_flags);

        std::swap(p_cur_buf, orig.p_cur_buf);
        std::swap(cur_buf_num, orig.cur_buf_num);
    }

    void unmap() const
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
            return std::string("invalid errno code '" + std::to_string(error_code) + "'");
        }

        return std::string(str_err) + " (" + std::to_string(error_code) + ")";
    }

    mmap_options opts;

    std::string file_path;
    int open_flags;

    mutable pointer p_cur_buf;
    mutable size_t cur_buf_num;
};

/// @brief  Memory page size calculation.
/// @return Memory page size.
inline long memory_page_size()
{
    return ::sysconf(_SC_PAGE_SIZE);
}

inline void* mmap_buf(void* p_addr, const size_t length, const int prot, const int flags, const int fd, const off_t offset)
{
    assert(fd != -1);
    assert(length);

    p_addr = ::mmap64(p_addr, length, prot, flags, fd, offset);
    if (p_addr == MAP_FAILED) {
        throw std::runtime_error("mmap_buf: error map file to memory: " + std::to_string(errno));
    }

    return p_addr;
}

inline void* mmap_buf(void* p_addr, const size_t length, const mmap_options& opts, const off_t offset)
{
    return mmap_buf(p_addr, length, opts.prot, opts.flags, opts.fd, opts.offset + offset);
}

inline int munmap_buf(void* p_addr, const size_t length)
{
    assert(length);

    if (! p_addr) {
        return 0;
    }

    return ::munmap(p_addr, length);
}

} // namespace utils
} // namespace details
} // namespace mfcnt

#endif /* _MMAP_CONTAINERS_MFCNT_UTILS_H */

