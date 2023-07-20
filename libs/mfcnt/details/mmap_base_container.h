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

#ifndef _MMAP_CONTAINERS_MFCNT_MMAP_BASE_CONTAINER_H
#define _MMAP_CONTAINERS_MFCNT_MMAP_BASE_CONTAINER_H

#include "mfcnt/types.h"
#include "mfcnt/details/utils.h"

namespace mfcnt {
namespace details {

template<typename TType, size_t TBufSize, template<typename TTp, size_t TBs> class TIterator>
class mmap_base_container
{
protected:
    typedef TType                                   value_type;
    typedef value_type*                             pointer;
    typedef const value_type*                       const_pointer;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef TIterator<value_type, TBufSize>         iterator;
    typedef TIterator<const value_type, TBufSize>   const_iterator;

    /// @brief  Constructor.
    mmap_base_container()
        : m_size(0)
        , m_begin_delta(0)
        , m_mmap_size(0)
    {}

    /// @brief  Constructor.
    /// @param  file_path  - path to file.
    /// @param  fd         - file descriptor.
    /// @param  offset     - offset to start mapping the file.
    /// @param  open_flags - flags with which you need to open the file.
    /// @param  mmap_prot  - desired memory protection of the mapping.
    /// @param  mmap_flags - determines whether updates to the mapping are visible to other
    ///                      processes mapping the same region, and whether updates are carried
    ///                      through to the underlying file.
    mmap_base_container(const std::string& file_path, off64_t offset, mode m)
        : m_buffer(file_path, m)
        , m_size(m_buffer.file_size())
        , m_begin_delta(offset % utils::memory_page_size())
        , m_mmap_size(m_size + m_begin_delta)
    {
        assert(m_buffer.is_open());
        assert(! (TBufSize % utils::memory_page_size()));
        assert(! (m_size % sizeof(value_type)));

        m_buffer.opts.offset = offset - m_begin_delta;
    }

    /// @brief  Constructor.
    /// @param  file_path  - path to file.
    /// @param  fd         - file descriptor.
    /// @param  size       - the size of the file to be mapped to memory in the container.
    /// @param  offset     - offset to start mapping the file.
    /// @param  open_flags - flags with which you need to open the file.
    /// @param  mmap_prot  - desired memory protection of the mapping.
    /// @param  mmap_flags - determines whether updates to the mapping are visible to other
    ///                      processes mapping the same region, and whether updates are carried
    ///                      through to the underlying file.
    mmap_base_container(const std::string& file_path, size_t size, off64_t offset, mode m)
        : m_buffer(file_path, m)
        , m_size(size)
        , m_begin_delta(offset % utils::memory_page_size())
        , m_mmap_size(m_size + m_begin_delta)
    {
        assert(m_buffer.is_open());
        assert(! (TBufSize % utils::memory_page_size()));
        assert(! (m_size % sizeof(value_type)));

        m_buffer.opts.offset = offset - m_begin_delta;
    }

    /// @brief  Copy constructor.
    mmap_base_container(const mmap_base_container& orig)
        : m_buffer(orig.m_buffer)
        , m_size(orig.m_size)
        , m_begin_delta(orig.m_begin_delta)
        , m_mmap_size(orig.m_mmap_size)
    {
        assert(m_buffer.is_open());
        assert(! (TBufSize % utils::memory_page_size()));
        assert(! (m_size % sizeof(value_type)));
    }

    /// @brief  Move constructor.
    mmap_base_container(mmap_base_container&& orig)
        : m_buffer(std::move(orig.m_buffer))
        , m_size(orig.m_size)
        , m_begin_delta(orig.m_begin_delta)
        , m_mmap_size(orig.m_mmap_size)
    {
        orig.m_size = 0;
    }

    /// @brief  Destructor.
    virtual ~mmap_base_container()
    {
        if (! m_buffer.is_open()) {
            return;
        }

        m_buffer.close();
        m_size = 0;
    }

    /// @brief  Сheck for exceeding the permissible range.
    /// @throw  std::runtime_error if the value exceeds the permissible limits.
    inline void check_range(size_t pos) const
    {
        assert(m_buffer.is_open() && "check_range: file is not open");
        assert(pos < m_size && "position is out of range");

        if (pos >= m_size) {
            throw std::runtime_error("mmap_base_container::check_range: pos (which is "
                                     + std::to_string(pos) + ") >= this->size() (which is "
                                     + std::to_string(m_size) + ")");
        }
    }

    /// @brief  Get value by position.
    /// @param  pos - position.
    /// @return Element reference.
    inline reference get_value(size_t pos) const
    {
        assert(m_buffer.is_open() && "get_value: file is not open");

        pos += m_begin_delta;
        pointer p_page = m_buffer.map(pos / TBufSize);
        return *(p_page + (pos % TBufSize));
    }

    void swap(mmap_base_container& orig)
    {
        if (this == &orig) {
            return;
        }
        m_buffer.swap(orig.m_buffer);
        std::swap(m_size, orig.m_size);
        std::swap(m_begin_delta, orig.m_begin_delta);
        std::swap(m_mmap_size, orig.m_mmap_size);
    }

protected:
    utils::mmap_buffer<pointer, TBufSize> m_buffer;
    size_t m_size;
    size_t m_begin_delta;
    size_t m_mmap_size;
};

} // namespace details
} // namespace mfcnt

#endif /* _MMAP_CONTAINERS_MFCNT_MMAP_BASE_CONTAINER_H */

