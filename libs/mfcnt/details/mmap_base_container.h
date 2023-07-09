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

template<typename TType, size_t TBufSize>
class mmap_base_container
{
protected:
    typedef TType                   value_type;
    typedef value_type*             pointer;
    typedef const value_type*       const_pointer;
    typedef value_type&             reference;
    typedef const value_type&       const_reference;

    /// @brief  Constructor.
    mmap_base_container()
        : m_begin_delta(0)
        , m_mmap_size(0)
        , m_size(0)
    {}

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
        , m_begin_delta(0)
        , m_mmap_size(0)
        , m_size(0)
    {
        assert(m_impl.buf_mapper.opts.fd != -1);

        const long page_size = utils::memory_page_size();
        assert(! (TBufSize % page_size));
        assert(! (size % sizeof(value_type)));

        m_begin_delta = offset % page_size;
        m_mmap_size = size + m_begin_delta;
        m_size = size;

        m_buffer.opts.offset = offset - m_begin_delta;
    }

    /// @brief  Copy constructor.
    mmap_base_container(const mmap_base_container& orig) = delete;

    /// @brief  Move constructor.
    mmap_base_container(mmap_base_container&& orig) = delete;

    /// @brief  Destructor.
    virtual ~mmap_base_container()
    {
        
        if (! m_buffer.is_open()) {
            return;
        }

        m_buffer.close();
        m_size = 0;
    }

private:
    mmap_buffer<pointer, TBufSize> m_buffer;

    size_t m_begin_delta;
    size_t m_mmap_size;
    size_t m_size;
};

} // namespace details
} // namespace mfcnt

#endif /* _MMAP_CONTAINERS_MFCNT_MMAP_BASE_CONTAINER_H */

