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

#include <deque>
#include <filesystem>
#include <list>
#include <vector>

#include <testing/perfdefs.h>
#include <testing/utils.h>

#include "utils.h"
#include "mfcnt/mmap_deque_view.h"
#include "mfcnt/mmap_list_view.h"

namespace {

class mfcnt_env : public ::testing::utils::base_env
{
    using base = ::testing::utils::base_env;

public:
    mfcnt_env()
        : base("perf_mfcnt")
    {}

    virtual void SetUp() override
    {
        using ut = ::tests::details::utils;

        base::SetUp();

        m_file_10_Mb = work_dir() / "tmp_file_10_Mb";
        PERF_ASSERT_TRUE(ut::create_test_file(m_file_10_Mb, tests::kTestData, 10 * 1024 * 1024));

        m_file_25_Mb = work_dir() / "tmp_file_25_Mb";
        PERF_ASSERT_TRUE(ut::create_test_file(m_file_25_Mb, tests::kTestData, 25 * 1024 * 1024));

        m_file_50_Mb = work_dir() / "tmp_file_50_Mb";
        PERF_ASSERT_TRUE(ut::create_test_file(m_file_50_Mb, tests::kTestData, 50 * 1024 * 1024));

//        m_file_100_Mb = work_dir() / "tmp_file_100_Mb";
//        PERF_ASSERT_TRUE(ut::create_test_file(m_file_100_Mb, tests::kTestData, 100 * 1024 * 1024));

//        m_file_250_Mb = work_dir() / "tmp_file_250_Mb";
//        PERF_ASSERT_TRUE(ut::create_test_file(m_file_250_Mb, tests::kTestData, 250 * 1024 * 1024));
    }

    static const std::filesystem::path& file_10_Mb() { return m_file_10_Mb; }
    static const std::filesystem::path& file_25_Mb() { return m_file_25_Mb; }
    static const std::filesystem::path& file_50_Mb() { return m_file_50_Mb; }
    static const std::filesystem::path& file_100_Mb() { return m_file_100_Mb; }
    static const std::filesystem::path& file_250_Mb() { return m_file_250_Mb; }

    template<typename TCnt>
    static TCnt cnt_from_file(const std::filesystem::path& file)
    {
        using cnt_type = TCnt;
        if constexpr (std::is_same<cnt_type, std::deque<char>>::value ||
                      std::is_same<cnt_type, std::list<char>>::value ||
                      std::is_same<cnt_type, std::vector<char>>::value) {
            return ::tests::details::utils::create_stl_cnt<cnt_type>(file);
        } else {
            return cnt_type(file);
        }
    }

private:
    std::filesystem::path work_dir() const { return base::work_dir(); }

private:
    static std::filesystem::path m_file_10_Mb;
    static std::filesystem::path m_file_25_Mb;
    static std::filesystem::path m_file_50_Mb;
    static std::filesystem::path m_file_100_Mb;
    static std::filesystem::path m_file_250_Mb;
};

std::filesystem::path mfcnt_env::m_file_10_Mb = {};
std::filesystem::path mfcnt_env::m_file_25_Mb = {};
std::filesystem::path mfcnt_env::m_file_50_Mb = {};
std::filesystem::path mfcnt_env::m_file_100_Mb = {};
std::filesystem::path mfcnt_env::m_file_250_Mb = {};

template<typename TType>
class mfcnt_common : public ::testing::Test
{
    using cnt_t = TType;

public:
    virtual void SetUp() override
    {
        m_cnt_10_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_10_Mb()));
        m_cnt_25_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_25_Mb()));
        m_cnt_50_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_50_Mb()));
        m_cnt_100_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_100_Mb()));
        m_cnt_250_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_250_Mb()));
    }

protected:
    cnt_t m_cnt_10_Mb;
    cnt_t m_cnt_25_Mb;
    cnt_t m_cnt_50_Mb;
    cnt_t m_cnt_100_Mb;
    cnt_t m_cnt_250_Mb;
};

template<typename TType>
class mfcnt_line : public ::testing::Test
{
    using cnt_t = TType;

public:
    virtual void SetUp() override
    {
        m_cnt_10_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_10_Mb()));
        m_cnt_25_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_25_Mb()));
        m_cnt_50_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_50_Mb()));
        m_cnt_100_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_100_Mb()));
        m_cnt_250_Mb = std::move(mfcnt_env::cnt_from_file<cnt_t>(mfcnt_env::file_250_Mb()));
    }

protected:
    cnt_t m_cnt_10_Mb;
    cnt_t m_cnt_25_Mb;
    cnt_t m_cnt_50_Mb;
    cnt_t m_cnt_100_Mb;
    cnt_t m_cnt_250_Mb;
};

using types_common = testing::Types<mfcnt::mmap_deque_view<char>,
                                    mfcnt::mmap_list_view<char>,
                                    std::deque<char>,
                                    //std::list<char>,
                                    std::vector<char>>;
TYPED_PERF_TEST_SUITE(mfcnt_common, types_common);

using types_line = testing::Types<mfcnt::mmap_deque_view<char>,
                                  std::deque<char>,
                                  std::vector<char>>;
TYPED_PERF_TEST_SUITE(mfcnt_line, types_line);

} // <anonymous> namespace

#define TYPED_PERF_TEST_COPY_END_IT(file_size)                              \
    TYPED_PERF_TEST(mfcnt_common, copy_end_it_##file_size##MB)              \
    {                                                                       \
        using cnt_type = TypeParam;                                         \
        PERF_INIT_TIMER(copy_end_it);                                       \
        size_t dummy = 0;                                                   \
        PERF_START_TIMER(copy_end_it);                                      \
        for (typename cnt_type::const_iterator it =                         \
                this->m_cnt_##file_size##_Mb.begin();                       \
             it != this->m_cnt_##file_size##_Mb.end(); ++it) {              \
            dummy += *it;                                                   \
        }                                                                   \
        PERF_PAUSE_TIMER(copy_end_it);                                      \
    }

#define TYPED_PERF_TEST_NO_COPY_END_IT(file_size)                           \
    TYPED_PERF_TEST(mfcnt_common, no_copy_end_it_##file_size##MB)           \
    {                                                                       \
        using cnt_type = TypeParam;                                         \
        PERF_INIT_TIMER(no_copy_end_it);                                    \
        size_t dummy = 0;                                                   \
        typename cnt_type::const_iterator end_it =                          \
            this->m_cnt_##file_size##_Mb.end();                             \
        PERF_START_TIMER(no_copy_end_it);                                   \
        for (typename cnt_type::const_iterator it =                         \
                this->m_cnt_##file_size##_Mb.begin(); it != end_it; ++it) { \
            dummy += *it;                                                   \
        }                                                                   \
        PERF_PAUSE_TIMER(no_copy_end_it);                                   \
    }

#define TYPED_PERF_TEST_OPERATOR(file_size)                                 \
    TYPED_PERF_TEST(mfcnt_line, operator_##file_size##MB)                   \
    {                                                                       \
        PERF_INIT_TIMER(operator[]);                                        \
        size_t dummy = 0;                                                   \
        PERF_START_TIMER(operator[]);                                       \
        for (size_t i = 0; i < this->m_cnt_##file_size##_Mb.size(); ++i) {  \
            dummy += this->m_cnt_##file_size##_Mb[i];                       \
        }                                                                   \
        PERF_PAUSE_TIMER(operator[]);                                       \
    }

#define TYPED_PERF_TEST_AT_FUNC(file_size)                                  \
    TYPED_PERF_TEST(mfcnt_line, at_function_##file_size##MB)                \
    {                                                                       \
        PERF_INIT_TIMER(at_function);                                       \
        size_t dummy = 0;                                                   \
        PERF_START_TIMER(at_function);                                      \
        for (size_t i = 0; i < this->m_cnt_##file_size##_Mb.size(); ++i) {  \
            dummy += this->m_cnt_##file_size##_Mb.at(i);                    \
        }                                                                   \
        PERF_PAUSE_TIMER(at_function);                                      \
    }

#define DECLARE_TESTS_GROUP(group_name)     \
    TYPED_PERF_TEST_##group_name(10)        \
    TYPED_PERF_TEST_##group_name(25)        \
    TYPED_PERF_TEST_##group_name(50)
//    TYPED_PERF_TEST_##group_name(100)
//    TYPED_PERF_TEST_##group_name(250)

DECLARE_TESTS_GROUP(COPY_END_IT)
DECLARE_TESTS_GROUP(NO_COPY_END_IT)
DECLARE_TESTS_GROUP(OPERATOR)
DECLARE_TESTS_GROUP(AT_FUNC)

int main(int /*argc*/, char** /*argv*/)
{
    ::testing::AddGlobalTestEnvironment(new mfcnt_env());
    return RUN_ALL_PERF_TESTS();
}

