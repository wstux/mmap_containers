#include <filesystem>

#include <testing/testdefs.h>
#include <testing/utils.h>

#include "mfcnt/mmap_deque_view.h"

#include "utils.h"

namespace {

class mfcnt_env : public ::testing::utils::base_env
{
    using base = ::testing::utils::base_env;

public:
    mfcnt_env()
        : base("ut_mfcnt")
    {}

    virtual void SetUp() override
    {
        namespace ut = ::tests::details;

        base::SetUp();

        m_test_file = base::work_dir() + "/tmp_file";
        ASSERT_TRUE(ut::utils::create_test_file(m_test_file, m_file_1_Mb))
            << "failed to create test file '" << m_test_file << "'";
    }

    static std::string test_data()
    {
        namespace ut = ::tests;

        std::string test_data;
        size_t size = m_file_1_Mb;
        while (size != 0) {
            if (size < ut::kTestData.size()) {
                test_data += ut::kTestData.substr(0, size);
                size = 0;
            } else {
                test_data += ut::kTestData;
                size -= ut::kTestData.size();
            }
        }
        return test_data;
    }

    static const std::filesystem::path& test_file() { return m_test_file; }

    static size_t test_file_size() { return m_file_1_Mb; }
private:
    static const size_t m_file_1_Mb;
    static std::filesystem::path m_test_file;
};

const size_t mfcnt_env::m_file_1_Mb = 1 * 1024 * 1024;
std::filesystem::path mfcnt_env::m_test_file = {};

template<typename TType>
class mfcnt_fixture : public ::testing::Test
{
    using base = ::testing::Test;

public:
    using cnt_type_t = TType;

    virtual void SetUp() override
    {
        base::SetUp();

        m_base_fd_count = tests::details::utils::fd_count();
//        m_base_mem = tests::details::utils::mem_usage();
    }

    virtual void TearDown() override
    {
        base::TearDown();

        const size_t fd_count = tests::details::utils::fd_count();
        EXPECT_TRUE(m_base_fd_count == fd_count)
            << m_base_fd_count << " != " << fd_count;

//        const size_t mem_usage = tests::details::utils::mem_usage();
//        EXPECT_TRUE(m_base_mem == mem_usage)
//            << m_base_mem << " != " << mem_usage;
    }

    std::string test_data() const { return mfcnt_env::test_data(); }

    std::string test_file() const { return mfcnt_env::test_file().string(); }

    size_t test_file_size() const { return mfcnt_env::test_file_size(); }

private:
    size_t m_base_fd_count = 0;
//    size_t m_base_mem = 0;
};

using cnt_types = testing::Types<mfcnt::mmap_deque_view<char, 4096>>;
TYPED_TEST_SUITE(mfcnt_fixture, cnt_types);

} // <anonumous> namespace

TYPED_TEST(mfcnt_fixture, at)
{
    TypeParam cnt(this->test_file());
    EXPECT_TRUE(! cnt.empty());

    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
    EXPECT_TRUE(cnt.at(4097) == 'a') << "cnt.at(" << cnt.at(4097) << ") == 'a'";
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
}

TYPED_TEST(mfcnt_fixture, copy_constructor)
{
    TypeParam cnt_orig(this->test_file());
    EXPECT_TRUE(cnt_orig.size() == this->test_file_size());

    TypeParam cnt(cnt_orig);
    EXPECT_TRUE(cnt.size() == this->test_file_size());
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
    EXPECT_TRUE(cnt.at(4097) == 'a') << "cnt.at(" << cnt.at(4097) << ") == 'a'";
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";

    EXPECT_TRUE(cnt.at(0) == cnt_orig.at(0)) << "cnt.at(" << cnt.at(0) << ") == cnt_orig.at(" << cnt_orig.at(0) << ")";
    EXPECT_TRUE(cnt.at(4097) == cnt_orig.at(4097)) << "cnt.at(" << cnt.at(4097) << ") == cnt_orig.at(" << cnt_orig.at(4097) << ")";
    EXPECT_TRUE(cnt.at(0) == cnt_orig.at(0)) << "cnt.at(" << cnt.at(0) << ") == cnt_orig.at(" << cnt_orig.at(0) << ")";
}

TYPED_TEST(mfcnt_fixture, copy_operator)
{
    TypeParam cnt_orig(this->test_file());
    EXPECT_TRUE(cnt_orig.size() == this->test_file_size());

    TypeParam cnt;
    EXPECT_TRUE(cnt.size() == 0);

    cnt = cnt_orig;
    EXPECT_TRUE(cnt.size() == this->test_file_size());
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
    EXPECT_TRUE(cnt.at(4097) == 'a') << "cnt.at(" << cnt.at(4097) << ") == 'a'";
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";

    EXPECT_TRUE(cnt.at(0) == cnt_orig.at(0)) << "cnt.at(" << cnt.at(0) << ") == cnt_orig.at(" << cnt_orig.at(0) << ")";
    EXPECT_TRUE(cnt.at(4097) == cnt_orig.at(4097)) << "cnt.at(" << cnt.at(4097) << ") == cnt_orig.at(" << cnt_orig.at(4097) << ")";
    EXPECT_TRUE(cnt.at(0) == cnt_orig.at(0)) << "cnt.at(" << cnt.at(0) << ") == cnt_orig.at(" << cnt_orig.at(0) << ")";
}


TYPED_TEST(mfcnt_fixture, move_constructor)
{
    TypeParam cnt_orig(this->test_file());
    EXPECT_TRUE(cnt_orig.size() == this->test_file_size());

    TypeParam cnt(std::move(cnt_orig));
    EXPECT_TRUE(cnt.size() == this->test_file_size());
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
    EXPECT_TRUE(cnt.at(4097) == 'a') << "cnt.at(" << cnt.at(4097) << ") == 'a'";
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
}

TYPED_TEST(mfcnt_fixture, move_operator)
{
    TypeParam cnt;
    EXPECT_TRUE(cnt.size() == 0);
    {
        TypeParam cnt_orig(this->test_file());
        EXPECT_TRUE(cnt_orig.size() == this->test_file_size());

        cnt = std::move(cnt_orig);
    }

    EXPECT_TRUE(cnt.size() == this->test_file_size());
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
    EXPECT_TRUE(cnt.at(4097) == 'a') << "cnt.at(" << cnt.at(4097) << ") == 'a'";
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
}


TYPED_TEST(mfcnt_fixture, operator_brackets)
{
    TypeParam cnt(this->test_file());
    EXPECT_TRUE(! cnt.empty());

    EXPECT_TRUE(cnt[0] == 'W') << "cnt[" << cnt[0] << "] == 'W'";
    EXPECT_TRUE(cnt[4097] == 'a') << "cnt[" << cnt[4097] << "] == 'a'";
    EXPECT_TRUE(cnt[0] == 'W') << "cnt[" << cnt[0] << "] == 'W'";
}

TYPED_TEST(mfcnt_fixture, swap)
{
    TypeParam cnt_orig(this->test_file());
    EXPECT_TRUE(cnt_orig.size() == this->test_file_size());
    EXPECT_TRUE(cnt_orig.at(0) == 'W') << "cnt_orig.at(" << cnt_orig.at(0) << ") == 'W'";
    EXPECT_TRUE(cnt_orig.at(4097) == 'a') << "cnt_orig.at(" << cnt_orig.at(4097) << ") == 'a'";
    EXPECT_TRUE(cnt_orig.at(0) == 'W') << "cnt_orig.at(" << cnt_orig.at(0) << ") == 'W'";

    TypeParam cnt;
    EXPECT_TRUE(cnt.size() == 0);

    cnt.swap(cnt_orig);
    EXPECT_TRUE(cnt_orig.size() == 0);
    EXPECT_TRUE(cnt.size() == this->test_file_size());
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
    EXPECT_TRUE(cnt.at(4097) == 'a') << "cnt.at(" << cnt.at(4097) << ") == 'a'";
    EXPECT_TRUE(cnt.at(0) == 'W') << "cnt.at(" << cnt.at(0) << ") == 'W'";
}

TYPED_TEST(mfcnt_fixture, test_1)
{
    TypeParam cnt(this->test_file());
    EXPECT_TRUE(! cnt.empty());

    typename TypeParam::iterator it = cnt.begin();
    for (const char ch : tests::kTestData) {
        EXPECT_TRUE(ch == *it) << ch << " != " << *it;
        ++it;
    }
}

TYPED_TEST(mfcnt_fixture, test_2)
{
    TypeParam cnt(this->test_file());
    const std::string test_data = this->test_data();

    ASSERT_TRUE(test_data.size() == cnt.size());

    typename TypeParam::iterator it = cnt.begin();
    for (const char ch : test_data) {
        EXPECT_TRUE(ch == *it) << ch << " != " << *it;
        ++it;
    }
}

TYPED_TEST(mfcnt_fixture, move)
{
    TypeParam cnt(this->test_file());
    EXPECT_TRUE(cnt.size() == this->test_file_size());

    {
        TypeParam cnt_m = std::move(cnt);
        EXPECT_TRUE(cnt_m.size() == this->test_file_size());
        EXPECT_TRUE(cnt_m.begin() != cnt_m.end());
        EXPECT_TRUE(cnt_m.cbegin() != cnt_m.cend());
    }
    //EXPECT_TRUE(cnt.empty()) << "container size = " << cnt.size();
    //EXPECT_TRUE(cnt.begin() == cnt.end());
    //EXPECT_TRUE(cnt.cbegin() == cnt.cend());
}

TYPED_TEST(mfcnt_fixture, move_and_iterate)
{
    const std::string test_data = this->test_data();

    TypeParam cnt;
    {
        TypeParam cnt_m(this->test_file());
        cnt = std::move(cnt_m);
    }
    
    EXPECT_TRUE(cnt.size() == this->test_file_size());
    EXPECT_TRUE(cnt.begin() != cnt.end());
    EXPECT_TRUE(cnt.cbegin() != cnt.cend());
    typename TypeParam::iterator it = cnt.begin();
    for (size_t i = 0; i < test_data.size(); ++i) {
        const char ch = test_data[i];
        EXPECT_TRUE(ch == *it) << ch << " != " << *it;
        ++it;
    }
}

TYPED_TEST(mfcnt_fixture, it_plus)
{
    using container_t = TypeParam;

    container_t cnt(this->test_file());
    typename container_t::iterator it_1 = cnt.begin();
    typename container_t::iterator it_2 = cnt.begin();
    typename container_t::const_iterator c_it_1 = cnt.cbegin();
    typename container_t::const_iterator c_it_2 = cnt.cbegin();
    size_t i = 0;

    EXPECT_TRUE(*it_1 == this->test_data()[i]);
    EXPECT_TRUE(it_1 == it_2);
    EXPECT_TRUE(it_1 == c_it_1);
    EXPECT_TRUE(c_it_2 == it_2);
    EXPECT_TRUE(c_it_1 == c_it_2);

    ++i;
    ++it_1;                 // check operator++()
    it_2++;                 // check operator++(int)
    c_it_1 += 1;            // check operator+=(int)
    c_it_2 = c_it_2 + 1;    // check operator+(int)
    // check operator==()
    EXPECT_TRUE(*it_1 == this->test_data()[i]);
    EXPECT_TRUE(it_1 == it_2);
    EXPECT_TRUE(it_1 == c_it_1);
    EXPECT_TRUE(c_it_2 == it_2);
    EXPECT_TRUE(c_it_1 == c_it_2);

    i = i + 3;
    it_1 = it_1 + 3;
    it_2 = it_2 + 1;
    c_it_1 = c_it_1 + 5;
    // check operator!=()
    EXPECT_TRUE(it_1 != it_2);
    EXPECT_TRUE(it_1 != c_it_1);
    EXPECT_TRUE(c_it_2 != it_2);
    EXPECT_TRUE(c_it_1 != c_it_2);

    // check operator=(iterator)
    it_2 = it_1;
    c_it_1 = it_1;
    c_it_2 = it_1;
    EXPECT_TRUE(it_1 == it_2);
    EXPECT_TRUE(it_1 == c_it_1);
    EXPECT_TRUE(c_it_2 == it_2);
    EXPECT_TRUE(c_it_1 == c_it_2);

    // check value near end of "window"
    it_1 = it_1 + (4090 - i);
    i = 4090;
    EXPECT_TRUE(*it_1 == this->test_data()[i]);

    // check value in next "window"
    i += 10;
    it_1 += 10;
    EXPECT_TRUE(*it_1 == this->test_data()[i]);
}

TYPED_TEST(mfcnt_fixture, it_minus)
{
    using container_t = TypeParam;

    container_t cnt(this->test_file());
    typename container_t::iterator it_1 = cnt.end();
    typename container_t::iterator it_2 = cnt.end();
    typename container_t::const_iterator c_it_1 = cnt.cend();
    typename container_t::const_iterator c_it_2 = cnt.cend();
    size_t i = this->test_data().size();

    EXPECT_TRUE(it_1 == it_2);
    EXPECT_TRUE(it_1 == c_it_1);
    EXPECT_TRUE(c_it_2 == it_2);
    ASSERT_TRUE(c_it_1 == c_it_2);

    --i;
    --it_1;                 // check operator--()
    it_2--;                 // check operator--(int)
    c_it_1 -= 1;            // check operator-=(int)
    c_it_2 = c_it_2 - 1;    // check operator-(int)
    // check operator==()
    EXPECT_TRUE(*it_1 == this->test_data()[i]);
    EXPECT_TRUE(it_1 == it_2);
    EXPECT_TRUE(it_1 == c_it_1);
    EXPECT_TRUE(c_it_2 == it_2);
    EXPECT_TRUE(c_it_1 == c_it_2);

    i = i - 3;
    it_1 = it_1 - 3;
    it_2 = it_2 - 1;
    c_it_1 = c_it_1 - 5;
    // check operator!=()
    EXPECT_TRUE(it_1 != it_2);
    EXPECT_TRUE(it_1 != c_it_1);
    EXPECT_TRUE(c_it_2 != it_2);
    EXPECT_TRUE(c_it_1 != c_it_2);

    // check operator=(iterator)
    it_2 = it_1;
    c_it_1 = it_1;
    c_it_2 = it_1;
    EXPECT_TRUE(it_1 == it_2);
    EXPECT_TRUE(it_1 == c_it_1);
    EXPECT_TRUE(c_it_2 == it_2);
    EXPECT_TRUE(c_it_1 == c_it_2);

    c_it_1 += 3;
    EXPECT_TRUE((c_it_1 - c_it_2) == 3);

    // check value near begin of "window"
    it_1 = it_1 - (i - 4100);
    i = 4100;
    EXPECT_TRUE(*it_1 == this->test_data()[i]);

    // check value in prev "window"
    i -= 10;
    it_1 -= 10;
    EXPECT_TRUE(*it_1 == this->test_data()[i]);
}

TYPED_TEST(mfcnt_fixture, DISABLED_iterators)
{
    TypeParam cnt(this->test_file());
    const std::string test_data = this->test_data();

    ASSERT_TRUE(test_data.size() == cnt.size());

    typename TypeParam::iterator it1 = cnt.begin();
    typename TypeParam::iterator it2 = --cnt.end();

    for (size_t i1 = 0, i2 = test_data.size() - 1; i1 < i2; ++i1, ++it1, --i2, --it2) {
        const char ch1 = test_data[i1];
        const char ch2 = test_data[i2];
        EXPECT_TRUE(ch1 == *it1) << ch1 << " != " << *it1;
        EXPECT_TRUE(ch2 == *it2) << ch2 << " != " << *it2;
    }
}

int main(int /*argc*/, char** /*argv*/)
{
    ::testing::AddGlobalTestEnvironment(new mfcnt_env());
    return RUN_ALL_TESTS();
}

