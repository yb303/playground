#include <assert.h>
#include <string.h> // for memcpy, memset
#include <stdint.h>
#include <type_traits>
#include <tuple>
#include <string>
#include <new>
#include <array>
#include <algorithm>

namespace t {

///////////////

template <class... T>
struct concat;

// 1 type
template <class T>
struct concat<T> {
    using type = std::tuple<T>;
};

template <class... Ts>
struct concat<std::tuple<Ts...>> {
    using type = std::tuple<Ts...>;
};

// 2 types
template <class T1, class T2>
struct concat<T1, T2> {
    using type = std::tuple<T1, T2>;
};

template <class T1, class... T2s>
struct concat<T1, std::tuple<T2s...>> {
    using type = std::tuple<T1, T2s...>;
};

template <class... T1s, class T2>
struct concat<std::tuple<T1s...>, T2> {
    using type = std::tuple<T1s..., T2>;
};

template <class... T1s, class... T2s>
struct concat<std::tuple<T1s...>, std::tuple<T2s...>> {
    using type = std::tuple<T1s..., T2s...>;
};

// 2+ types = 1 type + 1+ types
template <class T1, class T2, class... Ts>
struct concat<T1, T2, Ts...> {
    using type = typename concat<
                            T1,
                            typename concat<T2, Ts...>::type
                        >::type;
};

template <class... Ts>
using concat_t = typename concat<Ts...>::type;

///////////////

template <int N, class _T> struct head;

template <int N, class T, class... Ts>
struct head<N, std::tuple<T, Ts...>> {
    using type = concat_t<
                    T,
                    typename head<
                        N - 1,
                        std::tuple<Ts...>
                    >::type
                 >;
};

template <class T, class... Ts>
struct head<0, std::tuple<T, Ts...>> {
    using type = std::tuple<>;
};

template <int N, class _T>
using head_t = typename head<N, _T>::type;

/////////////

template <int N, class _T, bool Stop = (N == std::tuple_size_v<_T>)> struct tail;

template <int N, class... Ts>
struct tail<N, std::tuple<Ts...>, true> {
    using type = std::tuple<Ts...>;
};

template <int N, class T, class... Ts>
struct tail<N, std::tuple<T, Ts...>, false> {
    using type = typename tail<
                        N,
                        std::tuple<Ts...>
                    >::type;
};

template <int N, class _T>
using tail_t = typename tail<N, _T>::type;

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

template <int I, class _T> struct erase {
    using type = concat_t<
                    head_t<I, _T>,
                    skip_t<I + 1, _T>
                >;
};

template <int I, class _T>
using erase_t = typename erase<I, _T>::type;

///////////////

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

template <class T, class... Ts>
struct reverse<std::tuple<T, Ts...>> {
    using type = concat_t<
                    typename reverse<std::tuple<Ts...>>::type,
                    T
                >;
};

template <class _T>
using reverse_t = typename reverse<_T>::type;

///////////////////

template <class _T> struct reverse2;

template <class T>
struct reverse2<std::tuple<T>> {
    using type = std::tuple<T>;
};

template <class... Ts>
struct reverse2<std::tuple<Ts...>> {
    static constexpr size_t n = sizeof...(Ts) / 2;
    using type = concat_t<
                    typename reverse2<skip_t<n, std::tuple<Ts...>>>::type,
                    typename reverse2<head_t<n, std::tuple<Ts...>>>::type
                >;
};

template <class _T>
using reverse2_t = typename reverse2<_T>::type;

/////////////

template <template <class> class F, class _T> struct less;

template <template <class> class F, class T>
struct less<F, std::tuple<T>> {
    static constexpr auto   value = F<T>::value;
    static constexpr size_t index = 0;
};

template <template <class> class F, class T, class... Ts>
struct less<F, std::tuple<T, Ts...>> {
    using second_type = less<F, std::tuple<Ts...>>;
    static constexpr auto lhs = F<T>::value;
    static constexpr auto rhs = second_type::value;
    static constexpr auto value = lhs <= rhs ? lhs : rhs;
    static constexpr auto index = lhs <= rhs ? 0 : second_type::index + 1;
};

/////////////

template <class L, class R> struct less_sz {
    static constexpr bool value = sizeof(L) < sizeof(R);
};

template <class _T> struct min_sz2 {
};
template <class... Ts> struct min_sz2<std::tuple<Ts...>> {
    static constexpr auto a1 = std::make_integer_sequence<size_t, sizeof...(Ts)>();
    static constexpr std::array<size_t, sizeof...(Ts)> a  = { sizeof(Ts)... };
    static constexpr size_t index = std::min_element(a.begin(), a.end()) - a.begin();
};

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
                        erase_t<first_i, tup_t>
                    >::type
                >;
};

template <class _T>
using sort_sz_t = typename pick_sort<min_sz2, _T>::type;

//////////////

template<class T>
void ppp() {
    printf("%s\n", __PRETTY_FUNCTION__);
}

}

int main() {
    using namespace t;

    static_assert(std::is_same_v<
                    concat_t<
                        int,
                        bool, std::tuple<std::string>, std::tuple<>,
                        std::tuple<double>
                    >,
                    std::tuple<int, bool, std::string, double>
                >);


    static_assert(std::is_same_v<reverse_t<std::tuple<int, bool, std::string, double>>,
                                           std::tuple<double, std::string, bool, int>>);

    static_assert(std::is_same_v<reverse2_t<std::tuple<int, bool, std::string, double>>,
                                            std::tuple<double, std::string, bool, int>>);

    static_assert(std::is_same_v<head_t<3, std::tuple<int, bool, long, double>>,
                                           std::tuple<int, bool, long>>);
    static_assert(std::is_same_v<head_t<2, std::tuple<int, bool, long, double>>,
                                           std::tuple<int, bool>>);

    static_assert(std::is_same_v<tail_t<3, std::tuple<int, bool, long, double>>,
                                           std::tuple<     bool, long, double>>);
    static_assert(std::is_same_v<tail_t<2, std::tuple<int, bool, long, double>>,
                                           std::tuple<           long, double>>);

    static_assert(std::is_same_v<skip_t<3, std::tuple<int, bool, long, double>>,
                                           std::tuple<double>>);
    static_assert(std::is_same_v<skip_t<2, std::tuple<int, bool, long, double>>,
                                           std::tuple<long, double>>);

    static_assert(std::is_same_v<select_t<3, std::tuple<int, bool, long, double>>, double>);
    static_assert(std::is_same_v<select_t<2, std::tuple<int, bool, long, double>>, long>);

    static_assert(min_sz_i<std::tuple<bool, int, long, double>> == 0);
    static_assert(min_sz_i<std::tuple<int, bool, long, double>> == 1);
    static_assert(min_sz_i<std::tuple<int, long, bool, double>> == 2);
    static_assert(min_sz_i<std::tuple<int, long, double, bool>> == 3);
    static_assert(min_sz_v<std::tuple<bool, int, long, double>> == 1);
    static_assert(min_sz_v<std::tuple<int, bool, long, double>> == 1);
    static_assert(min_sz_v<std::tuple<int, long, bool, double>> == 1);
    static_assert(min_sz_v<std::tuple<int, long, double, bool>> == 1);


    ppp<sort_sz_t<std::tuple<int, bool, short, double, long>>>();
    ppp<sort_sz_t<std::tuple<int, short, char, bool, long, double >>>();


    static_assert(std::is_same_v<sort_sz_t<std::tuple<int, bool, short, double>>,
                                           std::tuple<bool, short, int, double>>);
    static_assert(std::is_same_v<sort_sz_t<std::tuple<long, int, short, char, bool, double>>,
                                           std::tuple<char, bool, short, int, long, double>>);

    return 0;
}
