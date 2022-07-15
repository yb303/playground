#include <assert.h>
#include <stdint.h>
#include <type_traits>
#include <tuple>
#include <string>

namespace t {

/////////////

template <class T1, class T2>
struct concat;

template <class T2, class... T1s>
struct concat<std::tuple<T1s...>, T2> {
    using type = std::tuple<T1s..., T2>;
};

template <class T1, class... T2s>
struct concat<T1, std::tuple<T2s...>> {
    using type = std::tuple<T1, T2s...>;
};

template <class T1, class T2>
using concat_t = typename concat<T1, T2>::type;

/////////////

template <class T1>
struct concat2_impl;

template <class... T1s>
struct concat2_impl<std::tuple<T1s...>> {
    template <class T2> struct add;
};

template <class... T1s>
template <class... T2s>
struct concat2_impl<std::tuple<T1s...>>::add<std::tuple<T2s...>> {
    using type = std::tuple<T1s..., T2s...>;
};

template <class T1, class T2>
struct concat2 {
    using type = typename concat2_impl<T1>::add<T2>::type;
};

template <class T1, class T2>
using concat2_t = typename concat2<T1, T2>::type;

/////////////

template <int N, class _T> struct head;

template <int N, class T, class... Ts>
struct head<N, std::tuple<T, Ts...>> {
    using type = typename concat<
                    T,
                    typename head<
                        N - 1,
                        std::tuple<Ts...>
                    >::type
                 >::type;
};

template <class T, class... Ts>
struct head<0, std::tuple<T, Ts...>> {
    using type = std::tuple<>;
};

template <int N, class _T>
using head_t = typename head<N, _T>::type;

/////////////

template <int N, class _T, bool Stop = (N == 0)> struct skip;

template <class... Ts>
struct skip<0, std::tuple<Ts...>, true> {
    using type = std::tuple<Ts...>;
};

template <int N, class T, class... Ts>
struct skip<N, std::tuple<T, Ts...>, false> {
    using type = typename skip<
                    N - 1,
                    std::tuple<Ts...>
                >::type;
};

template <int N, class _T>
using skip_t = typename skip<N, _T>::type;

/////////////

template <int N, class _T, bool Stop = (N == 0)> struct select;

template <class T, class... Ts>
struct select<0, std::tuple<T, Ts...>, true> {
    using type = T;
};

template <int N, class T, class... Ts>
struct select<N, std::tuple<T, Ts...>, false> {
    using type = typename select<
                    N - 1,
                    std::tuple<Ts...>
                >::type;
};

template <int N, class _T>
using select_t = typename select<N, _T>::type;

/////////////

template <class _T> struct reverse;

template <class T>
struct reverse<std::tuple<T>> {
    using type = std::tuple<T>;
};

template <class T0, class... Ts>
struct reverse<std::tuple<T0, Ts...>> {
    using type = typename concat<
                    typename reverse<std::tuple<Ts...>>::type,
                    T0
                >::type;
};

template <class _T>
using reverse_t = typename reverse<_T>::type;

/////////////

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

//////////////

template <template <class> class pick_first, class _T> struct pick_sort;

template <template <class> class pick_first, class T>
struct pick_sort<pick_first, std::tuple<T>> {
    using type = std::tuple<T>;
};
template <template <class> class pick_first, class... Ts>
struct pick_sort<pick_first, std::tuple<Ts...>> {
    using tup_t = std::tuple<Ts...>;
    static constexpr int first_i = pick_first<tup_t>::index;
    using type = concat_t<
                    select_t<first_i, tup_t>,
                    typename pick_sort<
                        pick_first,
                        concat2_t<
                            head_t<first_i, tup_t>,
                            skip_t<first_i + 1, tup_t>
                        >
                    >::type
                >;
};

template <class _T>
using sort_sz_t = typename pick_sort<min_sz, _T>::type;

//////////////

template<class T>
void ppp() {
    printf("%s\n", __PRETTY_FUNCTION__);
}

} // namespace t

int main() {
    using namespace t;

    static_assert(std::is_same_v<reverse_t<std::tuple<int, bool, std::string, double>>,
                                           std::tuple<double, std::string, bool, int>>);


    static_assert(std::is_same_v<head_t<3, std::tuple<int, bool, long, double>>,
                                           std::tuple<int, bool, long>>);
    static_assert(std::is_same_v<head_t<2, std::tuple<int, bool, long, double>>,
                                           std::tuple<int, bool>>);

    static_assert(std::is_same_v<skip_t<3, std::tuple<int, bool, long, double>>,
                                           std::tuple<double>>);
    static_assert(std::is_same_v<skip_t<2, std::tuple<int, bool, long, double>>,
                                           std::tuple<long, double>>);

    static_assert(std::is_same_v<select_t<3, std::tuple<int, bool, long, double>>, double>);
    static_assert(std::is_same_v<select_t<2, std::tuple<int, bool, long, double>>, long>);

    static_assert(min_sz_i<std::tuple<int, bool, long, double>> == 1);
    static_assert(min_sz_i<std::tuple<short, int, bool, long, double>> == 2);

    ppp<sort_sz_t<std::tuple<long, int, double, short, bool, char>>>();

    static_assert(std::is_same_v<sort_sz_t<std::tuple<int, bool, short, double>>,
                                           std::tuple<bool, short, int, double>>);
    static_assert(std::is_same_v<sort_sz_t<std::tuple<long, int, short, char, bool, double>>,
                                           std::tuple<char, bool, short, int, long, double>>);

    return 0;
}
