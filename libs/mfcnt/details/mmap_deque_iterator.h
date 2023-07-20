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

#ifndef _MMAP_CONTAINERS_MFCNT_MMAP_DEQUE_ITERATOR_H
#define _MMAP_CONTAINERS_MFCNT_MMAP_DEQUE_ITERATOR_H

#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

#include "mfcnt/details/utils.h"

namespace mfcnt {
namespace details {

template<typename TTp, size_t TBufSize>
class mmap_deque_iterator
{
    typedef typename std::conditional<std::is_const<TTp>::value, typename std::remove_cv<TTp>::type, TTp>::type _type;
    typedef _type* _raw_ptr;

public:
    typedef std::random_access_iterator_tag         iterator_category;
    typedef TTp                                     value_type;
    typedef TTp*                                    pointer;
    typedef TTp&                                    reference;
    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;

    mmap_deque_iterator()
        : m_p_opts(NULL)
        , m_p_first(NULL)
        , m_p_last(NULL)
        , m_p_cur(NULL)
        , m_buf_num(0)
        , m_pos(0)
    {}

    mmap_deque_iterator(const utils::mmap_buffer<_raw_ptr, TBufSize>& buf_mapper, size_t buf_num, size_t pos)
        : m_p_opts(&buf_mapper.opts)
        , m_buf_num(buf_num)
        , m_pos(pos)
    {
        if (m_p_opts->fd != -1) {
            change_buf(m_buf_num, pos % TBufSize);
        }
    }

    mmap_deque_iterator(const mmap_deque_iterator& it)
        : m_p_opts(it.m_p_opts)
        , m_p_buf(it.m_p_buf)
        , m_p_first(it.m_p_first)
        , m_p_last(it.m_p_last)
        , m_p_cur(it.m_p_cur)
        , m_buf_num(it.m_buf_num)
        , m_pos(it.m_pos)
    {
        assert(m_p_opts->fd != -1);
        assert(m_p_opts != nullptr);
    }

    template<class T, typename = typename std::enable_if<! std::is_const<T>::value && std::is_same<const T, TTp>::value>::type>
    mmap_deque_iterator(const mmap_deque_iterator<T, TBufSize>& it)
        : m_p_opts(it.m_p_opts)
        , m_p_buf(it.m_p_buf)
        , m_p_first(it.m_p_first)
        , m_p_last(it.m_p_last)
        , m_p_cur(it.m_p_cur)
        , m_buf_num(it.m_buf_num)
        , m_pos(it.m_pos)
    {
        assert(m_p_opts->fd != -1);
        assert(m_p_opts != nullptr);
    }

    reference operator*() const
    {
        assert(m_p_opts != nullptr);
        assert(m_p_opts->fd != -1);
        assert(m_p_cur && m_p_cur != m_p_last);
        return *m_p_cur;
    }

    pointer operator->() const
    {
        assert(m_p_opts->fd != -1);
        assert(m_p_cur && m_p_cur != m_p_last);
        return m_p_cur;
    }

    mmap_deque_iterator& operator++()
    {
        assert(m_p_opts->fd != -1);
        assert(m_p_cur && m_p_cur != m_p_last);

        ++m_p_cur;
        ++m_pos;
        if (m_p_cur == m_p_last) {
            change_buf(++m_buf_num, 0);
        }
        return *this;
    }

    mmap_deque_iterator operator++(int)
    {
        assert(m_p_opts->fd != -1);

        mmap_deque_iterator tmp = *this;
        this->operator++();
        return tmp;
    }

    mmap_deque_iterator& operator+=(difference_type n)
    {
        assert(m_p_opts->fd != -1);

        m_pos += n;
        const difference_type offset = n + (m_p_cur - m_p_first);
        if ((offset >= 0) && (offset < difference_type(TBufSize))) {
            m_p_cur += n;
        } else {
            const difference_type node_offset = (offset > 0) ? (offset / difference_type(TBufSize)) : (-difference_type((-offset - 1) / TBufSize) - 1);
            m_buf_num += node_offset;
            change_buf(m_buf_num, offset - node_offset * difference_type(TBufSize));
        }
        return *this;
    }

    mmap_deque_iterator operator+(difference_type n)
    {
        assert(m_p_opts->fd != -1);

        mmap_deque_iterator tmp = *this;
        tmp += n;
        return tmp;
    }

    mmap_deque_iterator& operator--()
    {
        assert(m_p_opts->fd != -1);
        assert(m_pos != 0);

        if (m_p_cur == m_p_first) {
            change_buf(--m_buf_num, TBufSize);
        }
        --m_p_cur;
        --m_pos;
        return *this;
    }

    mmap_deque_iterator operator--(int)
    {
        assert(m_p_opts->fd != -1);

        mmap_deque_iterator tmp = *this;
        this->operator--();
        return tmp;
    }

    mmap_deque_iterator& operator-=(difference_type n)
    {
        assert(m_p_opts->fd != -1);

        return *this += -n;
    }

    mmap_deque_iterator operator-(difference_type n)
    {
        assert(m_p_opts->fd != -1);

        mmap_deque_iterator tmp = *this;
        tmp -= n;
        return tmp;
    }

    mmap_deque_iterator& operator=(const mmap_deque_iterator& it)
    {
        //assert(it.m_p_opts->fd != -1);

        m_p_opts = it.m_p_opts;
        m_p_buf = it.m_p_buf;
        m_p_first = it.m_p_first;
        m_p_last = it.m_p_last;
        m_p_cur = it.m_p_cur;
        m_buf_num = it.m_buf_num;
        m_pos = it.m_pos;

        return *this;
    }

    template<class T, typename = typename std::enable_if<! std::is_const<T>::value && std::is_same<const T, TTp>::value>::type>
    mmap_deque_iterator& operator=(const mmap_deque_iterator<T, TBufSize>& it)
    {
        //assert(it.m_p_opts->fd != -1);

        m_p_opts = it.m_p_opts;
        m_p_buf = it.m_p_buf;
        m_p_first = it.m_p_first;
        m_p_last = it.m_p_last;
        m_p_cur = it.m_p_cur;
        m_buf_num = it.m_buf_num;
        m_pos = it.m_pos;

        return *this;
    }

private:
    inline void change_buf(const size_t buf_num, const size_t cur_pos)
    {
        assert(m_p_opts->fd != -1);

        m_p_buf.reset((_raw_ptr)utils::mmap_buf(nullptr, TBufSize, *m_p_opts, buf_num * TBufSize), buf_deleter);

        assert(m_p_opts->fd != -1);

        m_p_first = m_p_buf.get();
        m_p_last = m_p_first + TBufSize;
        m_p_cur = m_p_first + cur_pos;
    }
    
    static void buf_deleter(_raw_ptr p_buf)
    {
        utils::munmap_buf(p_buf, TBufSize);
    }

public:
    const utils::mmap_options* m_p_opts;
    std::shared_ptr<_type> m_p_buf;
    _raw_ptr m_p_first;
    _raw_ptr m_p_last;
    _raw_ptr m_p_cur;
    size_t   m_buf_num;
    size_t   m_pos;
};

template<typename TTp, size_t TBufSize>
inline bool operator==(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    assert(lhl.m_p_opts == rhl.m_p_opts);
    return (lhl.m_pos == rhl.m_pos) && (lhl.m_p_opts == rhl.m_p_opts);
}

template<typename TTp, size_t TBufSize>
inline bool operator==(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<const TTp, TBufSize>& rhl)
{
    assert(lhl.m_p_opts == rhl.m_p_opts);
    return (lhl.m_pos == rhl.m_pos) && (lhl.m_p_opts == rhl.m_p_opts);
}

template<typename TTp, size_t TBufSize>
inline bool operator==(const mmap_deque_iterator<const TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    assert(lhl.m_p_opts == rhl.m_p_opts);
    return (lhl.m_pos == rhl.m_pos) && (lhl.m_p_opts == rhl.m_p_opts);
}

template<typename TTp, size_t TBufSize>
inline bool operator!=(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return (lhl.m_pos != rhl.m_pos) || (lhl.m_p_opts != rhl.m_p_opts);
}

template<typename TTp, size_t TBufSize>
inline bool operator!=(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<const TTp, TBufSize>& rhl)
{
    return (lhl.m_pos != rhl.m_pos) || (lhl.m_p_opts != rhl.m_p_opts);
}

template<typename TTp, size_t TBufSize>
inline bool operator!=(const mmap_deque_iterator<const TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return (lhl.m_pos != rhl.m_pos) || (lhl.m_p_opts != rhl.m_p_opts);
}

template<typename TTp, size_t TBufSize>
inline bool operator<(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return (lhl.m_pos < rhl.m_pos);
}

template<typename TTp, size_t TBufSize>
inline bool operator<(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<const TTp, TBufSize>& rhl)
{
    return (lhl.m_pos < rhl.m_pos);
}

template<typename TTp, size_t TBufSize>
inline bool operator<(const mmap_deque_iterator<const TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return (lhl.m_pos < rhl.m_pos);
}

template<typename TTp, size_t TBufSize>
inline bool operator>(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return rhl < lhl;
}

template<typename TTp, size_t TBufSize>
inline bool operator>(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<const TTp, TBufSize>& rhl)
{
    return rhl < lhl;
}

template<typename TTp, size_t TBufSize>
inline bool operator>(const mmap_deque_iterator<const TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return rhl < lhl;
}

template<typename TTp, size_t TBufSize>
inline bool operator<=(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return ! (rhl < lhl);
}

template<typename TTp, size_t TBufSize>
inline bool operator<=(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<const TTp, TBufSize>& rhl)
{
    return ! (rhl < lhl);
}

template<typename TTp, size_t TBufSize>
inline bool operator<=(const mmap_deque_iterator<const TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return ! (rhl < lhl);
}

template<typename TTp, size_t TBufSize>
inline bool operator>=(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return ! (lhl < rhl);
}

template<typename TTp, size_t TBufSize>
inline bool operator>=(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<const TTp, TBufSize>& rhl)
{
    return ! (lhl < rhl);
}

template<typename TTp, size_t TBufSize>
inline bool operator>=(const mmap_deque_iterator<const TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return ! (lhl < rhl);
}

template<typename TTp, size_t TBufSize>
inline typename mmap_deque_iterator<TTp, TBufSize>::difference_type operator-(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<TTp, TBufSize>& rhl)
{
    return typename mmap_deque_iterator<TTp, TBufSize>::difference_type(lhl.m_pos - rhl.m_pos);
}

template<typename TTp, size_t TBufSize>
inline typename mmap_deque_iterator<TTp, TBufSize>::difference_type operator-(const mmap_deque_iterator<TTp, TBufSize>& lhl, const mmap_deque_iterator<const TTp, TBufSize>& rhl)
{
    return typename mmap_deque_iterator<TTp, TBufSize>::difference_type(lhl.m_pos - rhl.m_pos);
}

template<typename TTp, size_t TBufSize>
inline mmap_deque_iterator<TTp, TBufSize> operator+(ptrdiff_t n, const mmap_deque_iterator<TTp, TBufSize>& it)
{
    return it + n;
}

} // namespace details
} // namespace mfcnt

#endif /* _MMAP_CONTAINERS_MFCNT_MMAP_DEQUE_ITERATOR_H */

