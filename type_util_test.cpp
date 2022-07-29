#include "type_util.h"

#include <string.h>

#include <assert.h>
#include <algorithm>
#include <type_traits>
#include <tuple>
#include <string>
#include <string_view>
#include <iostream>

// Minimum sizeof(T) in tuple - manual

template <class _T> struct min_sz;

template <class T>
struct min_sz<std::tuple<T>> {
    static constexpr int value = sizeof(T);
    static constexpr int index = 0;
};

template <class T, class... Ts>
struct min_sz<std::tuple<T, Ts...>> {
    using second_type = min_sz<std::tuple<Ts...>>;
    static constexpr int value = sizeof(T) <= second_type::value ? sizeof(T) : second_type::value;
    static constexpr int index = sizeof(T) <= second_type::value ? 0 : second_type::index + 1;
};

template <class _T>
static constexpr int min_sz_i = min_sz<_T>::index;
template <class _T>
static constexpr int min_sz_v = min_sz<_T>::value;

// Minimum sizeof(T) in tuple - simpler

template <class _T> struct min_sz2;

template <class... Ts>
struct min_sz2<std::tuple<Ts...>> {
    static constexpr size_t index = t::min<t::seq_t<size_t, sizeof(Ts)...>>::index;
};

template <class _T>
using sz_sorted_t = typename t::selection_sort<min_sz2, _T>::type;


template <class T, class>
struct bigger_than_1 {
    static constexpr bool value = sizeof(T) > 1;
};

////////////////

class TestManager {
    static TestManager*& _inst() {
        static TestManager* inst = nullptr;
        return inst;
    }

public:
    TestManager() {
        auto*& inst = _inst();
        assert(inst == nullptr);
        inst = this;
    }
    ~TestManager() {
        auto*& inst = _inst();
        inst = nullptr;
    }

    static TestManager& instance() {
        return *_inst();
    }

    template <class EXPR, class EXPECTED>
    void expect_same(const char* filename, int line_num, const char* expression_str) {
        test_count++;
        std::cout << format_filename(filename) << ":" << line_num << " ";
        if (!std::is_same_v<EXPR, EXPECTED>) {
            fail_count++;
            std::cout << fail_str() << "\n  " << expression_str        << "\n"
                      << "Is:\n  "            << type_name<EXPR>()     << "\n"
                      << "Expected:\n  "      << type_name<EXPECTED>() << "\n\n";
        } else if (print_ok) {
            std::cout << pass_str() << "    "
                      << (print_ok_exp ? expression_str : "") << "\n";
        }
    }

    template <class T1, class T2> // EXPRESSION, class EXPECTED>
    void expect_eq(const char* filename, int line_num, const char* result_str,
                   const char* expected_str, const T1& result, const T2& expected) {
        test_count++;
        std::cout << format_filename(filename) << ":" << line_num << " ";
        if (result != T1(expected)) {
            fail_count++;
            std::cout << fail_str() << "\n  " << result_str   << "\n"
                      << "Is:\n  "            << result       << "\n"
                      << "Expected:\n  "      << expected_str
                      << "  Which is "        << expected     << "\n\n";
        } else if (print_ok) {
            std::cout << pass_str() << "    "
                      << (print_ok_exp ? result_str : "") << "\n";
        }
    }

    int report() {
        std::cout << "------\n"
                  << "Total   " << test_count << " tests\n"
                  << "\033[32mPASSED\033[0m  " << test_count - fail_count << " tests\n";
        if (fail_count) {
            std::cout << "\033[31mFAILED\033[0m  " << fail_count << " tests\n";
        }
        return fail_count ? 1 : 0;
    }

    bool print_ok        = true;
    bool print_ok_exp    = true;
    bool print_full_path = false;

private:
    int test_count = 0;
    int fail_count = 0;

    template<class T>
    std::string_view type_name() {
        const char* s     = __PRETTY_FUNCTION__;
        const char* start = strstr(s, "[with T = ") + strlen("[with T = ");
        const char* end   = strchr(start, ';');
        return std::string_view(start, end ? end - start : strlen(start) - 1);
    }

    std::string_view format_filename(const char* filename) {
        if (print_full_path) return filename;
        const char* start = strrchr(filename, '/');
        return start ? start + 1 : filename;
    }

    static const char* fail_str() { return "\033[31mFAIL\033[0m"; }
    static const char* pass_str() { return "\033[32mOK\033[0m"; }
};

#define _UNPAREN_(...)  __VA_ARGS__

#define EXPECT_SAME(EXPR, EXPECTED)                                         \
    do {                                                                    \
        TestManager::instance().expect_same<                                \
            _UNPAREN_ EXPR, _UNPAREN_ EXPECTED>(__FILE__, __LINE__, #EXPR); \
    } while (false)

#define EXPECT_EQ(EXPR, EXPECTED)                                               \
    do {                                                                        \
        TestManager::instance().expect_eq(__FILE__, __LINE__, #EXPR, #EXPECTED, \
            _UNPAREN_ EXPR, _UNPAREN_ EXPECTED);                                \
    } while (false)

int main() {
    TestManager test_mgr;

    using namespace t;

    EXPECT_EQ((find_if_v<std::tuple<char, int, long>, std::is_same, char>),  (0));
    EXPECT_EQ((find_if_v<std::tuple<char, int, long>, std::is_same, int>),   (1));
    EXPECT_EQ((find_if_v<std::tuple<char, int, long>, std::is_same, long>),  (2));
    EXPECT_EQ((find_if_v<std::tuple<char, int, long>, std::is_same, short>), (3));

    EXPECT_EQ((find_v<std::tuple<char, int, long>, char>),  (0));
    EXPECT_EQ((find_v<std::tuple<char, int, long>, int>),   (1));
    EXPECT_EQ((find_v<std::tuple<char, int, long>, long>),  (2));
    EXPECT_EQ((find_v<std::tuple<char, int, long>, short>), (end_v<std::tuple<char, int, long>>));

    EXPECT_EQ((find_one_of_v<std::tuple<char, int, long>, std::tuple<short>>),       (3));
    EXPECT_EQ((find_one_of_v<std::tuple<char, int, long>, std::tuple<short, int>>),  (1));
    EXPECT_EQ((find_one_of_v<std::tuple<char, int, long>, std::tuple<short, long>>), (2));

    EXPECT_SAME((filter_t<std::tuple<char, int, long>, bigger_than_1>),
                (std::tuple<int, long>));

    EXPECT_SAME((filter_t<seq_t<int, 1, 2, 3, 5, 6, 7>, is_not_one_of, seq_t<int, 2, 3, 4>>),
                (seq_t<int, 1, 5, 6, 7>));

    //
    // tuple
    //

    EXPECT_SAME((concat_t<
                    int,
                    bool, std::tuple<std::string>, std::tuple<>, char,
                    std::tuple<double>
                >),
                (std::tuple<int, bool, std::string, char, double>));


    EXPECT_SAME((reverse_t<std::tuple<int, bool, std::string, double>>), (std::tuple<double, std::string, bool, int>));

//    EXPECT_SAME((reverse2_t<std::tuple<int, bool, std::string, double>>,
//                                            std::tuple<double, std::string, bool, int>>);

    EXPECT_SAME((head_t<2, std::tuple<int, bool, long, double>>), (std::tuple<int, bool>));
    EXPECT_SAME((head_t<3, std::tuple<int, bool, long, double>>), (std::tuple<int, bool, long>));

    EXPECT_SAME((tail_t<2, std::tuple<int, bool, long, double>>), (std::tuple<           long, double>));
    EXPECT_SAME((tail_t<3, std::tuple<int, bool, long, double>>), (std::tuple<     bool, long, double>));

    EXPECT_SAME((skip_t<2, std::tuple<int, bool, long, double>>), (std::tuple<           long, double>));
    EXPECT_SAME((skip_t<3, std::tuple<int, bool, long, double>>), (std::tuple<                 double>));

    EXPECT_SAME((select_t<3, std::tuple<int, bool, long, double>>), (double));
    EXPECT_SAME((select_t<2, std::tuple<int, bool, long, double>>), (long));

    EXPECT_EQ((min_sz_i<std::tuple<bool, int, long, double>>), (0));
    EXPECT_EQ((min_sz_i<std::tuple<int, bool, long, double>>), (1));
    EXPECT_EQ((min_sz_i<std::tuple<int, long, bool, double>>), (2));
    EXPECT_EQ((min_sz_i<std::tuple<int, long, double, bool>>), (3));
    EXPECT_EQ((min_sz_v<std::tuple<bool, int, long, double>>), (1));
    EXPECT_EQ((min_sz_v<std::tuple<int, bool, long, double>>), (1));
    EXPECT_EQ((min_sz_v<std::tuple<int, long, bool, double>>), (1));
    EXPECT_EQ((min_sz_v<std::tuple<int, long, double, bool>>), (1));


    EXPECT_SAME((sz_sorted_t<std::tuple<int, bool, short, double>>),
                (std::tuple<bool, short, int, double>));
    EXPECT_SAME((sz_sorted_t<std::tuple<long, int, short, char, bool, double>>),
                (std::tuple<char, bool, short, int, long, double>));

    //
    // seq
    //

    EXPECT_SAME((concat_t<seq_t<int, 1,2,3>, seq_t<int, 4,5,6>,seq_t<int, 7,8,9>>),
                (seq_t<int, 1,2,3, 4,5,6, 7,8,9>));


    EXPECT_SAME((head_t<2, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 4, 1>));
    EXPECT_SAME((head_t<3, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 4, 1, 8>));

    EXPECT_SAME((tail_t<2, seq_t<int, 4, 1, 8, 8>>), (seq_t<int,       8, 8>));
    EXPECT_SAME((tail_t<3, seq_t<int, 4, 1, 8, 8>>), (seq_t<int,    1, 8, 8>));

    EXPECT_SAME((skip_t<2, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 8, 8>));
    EXPECT_SAME((skip_t<3, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 8>));

    EXPECT_SAME((select_t<0, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 4>));
    EXPECT_SAME((select_t<1, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 1>));
    EXPECT_SAME((select_t<2, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 8>));
    EXPECT_SAME((select_t<3, seq_t<int, 4, 1, 8, 8>>), (seq_t<int, 8>));

//    EXPECT_SAME((reverse_t<seq_t<int, 4, 1, 24, 8>>),  (seq_t<int, 8, 24, 1, 4>));
//    EXPECT_SAME((reverse2_t<seq_t<int, 4, 1, 24, 8>>), (seq_t<int, 8, 24, 1, 4>));

    EXPECT_SAME((sorted_t<seq_t<int, 4, 1, 2, 8>>), (seq_t<int, 1, 2, 4, 8>));
    EXPECT_SAME((sorted_t<seq_t<int, 8, 4, 2, 1, 1, 8>>), (seq_t<int, 1, 1, 2, 4, 8, 8>));

    return test_mgr.report();
}
