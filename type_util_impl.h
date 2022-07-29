#include <type_traits>
#include <tuple>
#include <array>
#include <algorithm>

namespace t {
namespace detail {

// Sequence shorthand

template <typename T, T... Is>
using seq_t = std::integer_sequence<T, Is...>;

template <typename T, T... Is>
static constexpr std::array<T, sizeof...(Is)> seq_v = {Is...};

// Size of tuple or seq

static constexpr size_t npos = ~size_t(0);

template <class _T>
struct size;

template <typename T, T... Is>
struct size<std::integer_sequence<T, Is...>> {
    static constexpr size_t value = sizeof...(Is);
};

template <typename... Ts>
struct size<std::tuple<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

template <class _T>
static constexpr size_t size_v = size<_T>::value;

// Concatenate tuples or sequences

template <class... _T>
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

template <class T, T... Is>
struct concat<seq_t<T, Is...>> {
    using type = seq_t<T, Is...>;
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

template <class T, T... I1s, T... I2s>
struct concat<seq_t<T, I1s...>, seq_t<T, I2s...>> {
    using type = seq_t<T, I1s..., I2s...>;
};

// 2+ types = 1 type + 1+ types
template <class T1, class T2, class... Ts>
struct concat<T1, T2, Ts...> {
    using type = typename concat<
                    typename concat<T1>::type,
                    typename concat<T2, Ts...>::type
                >::type;
};

template <class... Ts>
using concat_t = typename concat<Ts...>::type;

// First few

template <int N, class _T, bool Stop = (N == 0)> struct head;

template <int N, class T, class... Ts>
struct head<N, std::tuple<T, Ts...>, false> {
    using type = concat_t<
                    T,
                    typename head<
                        N - 1,
                        std::tuple<Ts...>
                    >::type
                 >;
};

template <class T, class... Ts>
struct head<0, std::tuple<T, Ts...>, true> {
    using type = std::tuple<>;
};

template <int N, class T, T I, T... Is>
struct head<N, seq_t<T, I, Is...>, false> {
    using type = concat_t<
                    seq_t<T, I>,
                    typename head<
                        N - 1,
                        seq_t<T, Is...>
                    >::type
                 >;
};

template <class T, T... Is>
struct head<0, seq_t<T, Is...>, true> {
    using type = seq_t<T>;
};

template <int N, class _T>
using head_t = typename head<N, _T>::type;

// Last few

template <int N, class _T, bool Stop = (N == size_v<_T>)> struct tail;

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

template <int N, class T, T... Is>
struct tail<N, seq_t<T, Is...>, true> {
    using type = seq_t<T, Is...>;
};

template <int N, class T, T I, T... Is>
struct tail<N, seq_t<T, I, Is...>, false> {
    using type = typename tail<
                    N,
                    seq_t<T, Is...>
                >::type;
};

template <int N, class _T>
using tail_t = typename tail<N, _T>::type;

// A few past the start

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

template <class T, T... Is>
struct skip<0, seq_t<T, Is...>, true> {
    using type = seq_t<T, Is...>;
};

template <int N, class T, T I, T... Is>
struct skip<N, seq_t<T, I, Is...>, false> {
    using type = typename skip<
                    N - 1,
                    seq_t<T, Is...>
                >::type;
};

template <int N, class _T>
using skip_t = typename skip<N, _T>::type;

// Remove one

template <int I, class _T> struct erase {
    using type = concat_t<
                    head_t<I, _T>,
                    skip_t<I + 1, _T>
                >;
};

template <int I, class _T>
using erase_t = typename erase<I, _T>::type;

// Pick one

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

template <class T, T I, T... Is>
struct select<0, seq_t<T, I, Is...>, true> {
    using type      = seq_t<T, I>;
    static constexpr T value = I;
};

template <int N, class T, T I, T... Is>
struct select<N, seq_t<T, I, Is...>, false> {
    using type      = typename select<N - 1, seq_t<T, Is...>>::type;
    static constexpr T value = select<N - 1, seq_t<T, Is...>>::value;
};

template <int N, class _T>
using select_t = typename select<N, _T>::type;

template <int N, class _T>
static constexpr auto select_v = select<N, _T>::value;

// Find the first type in tuple or int in sequence that Pred
// The "returned" value is "end()" if not found

template <class Haystack>
static constexpr size_t end_v = size_v<Haystack>;

template <class Haystack,
          template <class, class> class Pred,
          class PredParam = void,
          bool Stop = (size_v<Haystack> == 0)>
struct find_if;

template <class Haystack, template <class, class> class Pred, class PredParam>
struct find_if<Haystack, Pred, PredParam, true> {
    static constexpr size_t value = 0;
};

template <class Haystack, template <class, class> class Pred, class PredParam>
struct find_if<Haystack, Pred, PredParam, false> {
    static constexpr size_t value = Pred<select_t<0, Haystack>, PredParam>::value ? 0
                                    : (1 + find_if<skip_t<1, Haystack>, Pred, PredParam>::value);
};

template <class Haystack, template <class, class> class Pred, class PredParam = void>
static constexpr size_t find_if_v = find_if<Haystack, Pred, PredParam>::value;

template <class Haystack, class Needle>
struct find {
    static constexpr size_t value = find_if<Haystack, std::is_same, Needle>::value;
};

template <class Haystack, class Needle>
static constexpr size_t find_v = find<Haystack, Needle>::value;

template <class T, class _T>
struct is_one_of {
    static constexpr bool value = find_v<_T, T> != end_v<_T>;
};

template <class T, class _T>
struct is_not_one_of {
    static constexpr bool value = find_v<_T, T> == end_v<_T>;
};

template <class Haystack, class Needles>
struct find_one_of {
    static constexpr size_t value = find_if_v<Haystack, is_one_of, Needles>;
};

template <class Haystack, class Needles>
struct find_not_one_of {
    static constexpr size_t value = find_if_v<Haystack, is_not_one_of, Needles>;
};

template <class Haystack, class Needles>
constexpr size_t find_one_of_v = find_one_of<Haystack, Needles>::value;

template <class Haystack, class Needles>
constexpr size_t find_not_one_of_v = find_not_one_of<Haystack, Needles>::value;

// Filter a tuple or sequence

template <class _T, template <class, class> class Pred, class PredParam = void> struct filter;

template <class T, template <class, class> class Pred, class PredParam>
struct filter<std::tuple<T>, Pred, PredParam> {
    using type = std::conditional_t<Pred<T, PredParam>::value, std::tuple<T>, std::tuple<>>;
};

template <class T, class... Ts, template <class, class> class Pred, class PredParam>
struct filter<std::tuple<T, Ts...>, Pred, PredParam> {
    using type = concat_t<
                    typename filter<std::tuple<T>,     Pred, PredParam>::type,
                    typename filter<std::tuple<Ts...>, Pred, PredParam>::type
                >;
};

template <class T, T I, template <class, class> class Pred, class PredParam>
struct filter<seq_t<T, I>, Pred, PredParam> {
    using type = std::conditional_t<Pred<seq_t<T, I>, PredParam>::value, seq_t<T, I>, seq_t<T>>;
};

template <class T, T I, T... Is, template <class, class> class Pred, class PredParam>
struct filter<seq_t<T, I, Is...>, Pred, PredParam> {
    using type = concat_t<
                    typename filter<seq_t<T, I>, Pred, PredParam>::type,
                    typename filter<seq_t<T, Is...>, Pred, PredParam>::type
                >;
};

template <class _T, template <class, class> class Pred, class PredParam = void>
using filter_t = typename filter<_T, Pred, PredParam>::type;


// Reverse a tuple or sequence

template <class _T> struct reverse;

template <class T>
struct reverse<std::tuple<T>> {
    using type = std::tuple<T>;
};

template <class T, class... Ts>
struct reverse<std::tuple<T, Ts...>> {
    using type = concat_t<
                    typename reverse<std::tuple<Ts...>>::type,
                    std::tuple<T>
                >;
};

template <class T, T I>
struct reverse<seq_t<T, I>> {
    using type = seq_t<T, I>;
};

template <class T, T I, T... Is>
struct reverse<seq_t<T, I, Is...>> {
    using type = concat_t<
                    typename reverse<seq_t<T, Is...>>::type,
                    seq_t<T, I>
                >;
};

template <class _T>
using reverse_t = typename reverse<_T>::type;

// Reverse a tuple or sequence differently

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

template <class T, T I>
struct reverse2<seq_t<T, I>> {
    using type = seq_t<T, I>;
};

template <class T, T... Is>
struct reverse2<seq_t<T, Is...>> {
    static constexpr size_t n = sizeof...(Is) / 2;
    using type = concat_t<
                    typename reverse2<skip_t<n, seq_t<T, Is...>>>::type,
                    typename reverse2<head_t<n, seq_t<T, Is...>>>::type
                >;
};

template <class _T>
using reverse2_t = typename reverse2<_T>::type;

// Selection sort - O(N^2)

template <template <class> class Select, class _T> struct selection_sort;

template <template <class> class Select, class T>
struct selection_sort<Select, std::tuple<T>> {
    using type = std::tuple<T>;
};
template <template <class> class Select, class... Ts>
struct selection_sort<Select, std::tuple<Ts...>> {
    using tup_t = std::tuple<Ts...>;
    static constexpr int first_i = Select<tup_t>::index;
    using type = concat_t<
                    select_t<first_i, tup_t>,
                    typename selection_sort<
                        Select,
                        erase_t<first_i, tup_t>
                    >::type
                >;
};

template <template <class> class Select, class T, T I>
struct selection_sort<Select, seq_t<T, I>> {
    using type = seq_t<T, I>;
};

template <template <class> class Select, class T, T... Is>
struct selection_sort<Select, seq_t<T, Is...>> {
    using s_t = seq_t<T, Is...>;
    static constexpr int first_i = Select<s_t>::index;
    using type = concat_t<
                    select_t<first_i, s_t>,
                    typename selection_sort<
                        Select,
                        erase_t<first_i, s_t>
                    >::type
                >;
};


// Minimum of sequence

template <class _T>
struct min;

template <class T, T... Is>
struct min<seq_t<T, Is...>> {
    static constexpr auto a = seq_v<T, Is...>;
    static constexpr size_t index = std::min_element(a.begin(), a.end()) - a.begin();
    static constexpr T      value = a[index];
};

template <class _T>
using sorted_t = typename selection_sort<min, _T>::type;

} // namespace detail
} // namespace t


