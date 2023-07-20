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

#ifndef _MMAP_CONTAINERS_MFCNT_MMAP_LIST_VIEW_H
#define _MMAP_CONTAINERS_MFCNT_MMAP_LIST_VIEW_H

#include "mfcnt/types.h"
#include "mfcnt/details/mmap_base_container.h"
#include "mfcnt/details/mmap_list_iterator.h"

namespace mfcnt {

template<typename TTp, size_t TCount = 4*1024*1024>
class mmap_list_view : protected details::mmap_base_container<TTp, sizeof(TTp)*TCount, details::mmap_list_iterator>
{
    typedef details::mmap_base_container<TTp, sizeof(TTp)*TCount, details::mmap_list_iterator> base;

public:
    typedef TTp                                     value_type;
    typedef typename base::const_pointer            pointer;
    typedef typename base::const_pointer            const_pointer;
    typedef typename base::const_reference          reference;
    typedef typename base::const_reference          const_reference;
    typedef typename base::iterator                 iterator;
    typedef typename base::const_iterator           const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;

    mmap_list_view()
        : base()
    {}

    mmap_list_view(const char* file_path, size_t size, off64_t offset, mode m = mode::R_ONLY)
        : base(file_path, size, offset, m)
    {}

    mmap_list_view(const std::string& file_path, size_t size, off64_t offset, mode m = mode::R_ONLY)
        : base(file_path, size, offset, m)
    {}

    mmap_list_view(const char* file_path, off64_t offset = 0, mode m = mode::R_ONLY)
        : base(std::string(file_path), offset, m)
    {}

    mmap_list_view(const std::string& file_path, off64_t offset = 0, mode m = mode::R_ONLY)
        : base(file_path, offset, m)
    {}

    mmap_list_view(const mmap_list_view& orig)
        : base(orig)
    {}

    mmap_list_view(mmap_list_view&& orig)
        : base(std::move(orig))
    {}

    virtual ~mmap_list_view() {}

    const_reference at(size_type pos) const
    {
        base::check_range(pos);
        return (*this)[pos];
    }

    const_reference back() const { return (*this)[size() - 1]; }

    iterator begin() { return iterator(base::m_buffer, 0, 0); }

    const_iterator begin() const { return const_iterator(base::m_buffer, 0, 0); }

    const_iterator cbegin() const { return const_iterator(base::m_buffer, 0, 0); }

    const_iterator cend() const { return const_iterator(base::m_buffer, base::m_size / (sizeof(TTp)*TCount), base::m_size); }

    bool empty() const { return (size() == 0); }

    iterator end() { return iterator(base::m_buffer, base::m_size / (sizeof(TTp)*TCount), base::m_size); }

    const_iterator end() const { return const_iterator(base::m_buffer, base::m_size / (sizeof(TTp)*TCount), base::m_size); }

    size_type size() const { return base::m_size; }

    void swap(mmap_list_view& orig) { base::swap(orig); }

    mmap_list_view& operator=(const mmap_list_view& orig)
    {
        if (this != &orig) {
            mmap_list_view(orig).swap(*this);
        }
        return *this;
    }

    mmap_list_view& operator=(mmap_list_view&& orig)
    {
        if (this != &orig) {
            mmap_list_view(std::move(orig)).swap(*this);
        }
        return *this;
    }

    const_reference operator[](size_type pos) const
    {
        assert(pos < size());
        return base::get_value(pos);
    }
};

} // namespace mfcnt

#endif /* _MMAP_CONTAINERS_MFCNT_MMAP_LIST_VIEW_H */

