#include "type_util_impl.h"
#include <type_traits>


namespace t {

// Sequence shorthand

template <typename T, T... Is>
using seq_t = detail::seq_t<T, Is...>;

template <typename T, T... Is>
static constexpr auto seq_v = detail::seq_v<T, Is...>;

// Size of tuple or seq

template <class T>
using size = detail::size<T>;

template <class T>
static constexpr size_t size_v = size<T>::value;

// Concatenate tuples or sequences.
// Tuples concatenate with tuples of classes.
// Sequences concatenate only with sequences.

template <class... Ts>
using concat = detail::concat<Ts...>;

template <class... Ts>
using concat_t = detail::concat_t<Ts...>;

// First few, like head -NUM

template <int N, class T>
using head = detail::head<N, T>;

template <int N, class T>
using head_t = detail::head_t<N, T>;

// Last few, like tail -NUM

template <int N, class T>
using tail = detail::tail<N, T>;

template <int N, class T>
using tail_t = detail::tail_t<N, T>;

// A few past the start, like tail -n +NUM

template <int N, class T>
using skip = detail::skip<N, T>;

template <int N, class T>
using skip_t = detail::skip_t<N, T>;

// Remove one

template <int I, class T>
using erase = detail::erase<I, T>;

template <int I, class T>
using erase_t = detail::erase_t<I, T>;

// Pick one

template <int N, class T>
using select = detail::select<N, T>;

template <int N, class T>
using select_t = detail::select_t<N, T>;

template <int N, class T>
static constexpr auto select_v = detail::select_v<N, T>;

// Find the first type in tuple or something in sequence where Pred returns true.
// Pred is used like - Pred<TestT, PredParam>::value
// The "returned" value is "end()" if not found

template <class T>
static constexpr size_t end_v = detail::end_v<T>;  // size of T tuple of sequence

template <class Haystack, template <class, class> class Pred, class PredParam = void>
using find_if = detail::find_if<Haystack, Pred, PredParam>;

template <class Haystack, template <class, class> class Pred, class PredParam = void>
static constexpr size_t find_if_v = detail::find_if_v<Haystack, Pred, PredParam>;

// Find type in tuple or literal in sequence
// Thats like find_if(Haystack, [](T, Needle){ return T == Needle; }, Needle)

template <class Haystack, class Needle>
struct find {
    static constexpr size_t value = find_if<Haystack, std::is_same, Needle>::value;
};

template <class Haystack, class Needle>
static constexpr size_t find_v = find<Haystack, Needle>::value;

// Find one of / not one of

template <class T, class G>
struct is_one_of {
    static constexpr bool value = find_v<G, T> != end_v<G>;
};

template <class T, class G>
struct is_not_one_of {
    static constexpr bool value = find_v<G, T> == end_v<G>;
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

template <class T, template <class, class> class Pred, class PredParam = void>
using filter = detail::filter<T, Pred, PredParam>;

template <class T, template <class, class> class Pred, class PredParam = void>
using filter_t = detail::filter_t<T, Pred, PredParam>;

// Reverse a tuple or sequence

template <class T>
using reverse = detail::reverse<T>;

template <class T>
using reverse_t = detail::reverse_t<T>;

// Selection sort - O(N^2)
// Select<Tuple or sequence>::value returns the index of the first one

template <template <class> class Select, class T>
using selection_sort = detail::selection_sort<Select, T>;

// Sort sequence by less for this literal type

template <class T>
using min = detail::min<T>;

template <class T>
using sorted_t = typename selection_sort<min, T>::type;

}  // namespace t
