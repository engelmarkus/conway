#include <iostream>
#include <tuple>
#include <utility>

using namespace std;

// Parser for the input field.
template <typename R, char C, int I, typename... Ts> struct parser;
template <typename R, char C, int I, typename... Ts> using parser_t = typename parser<R, C, I, Ts...>::type;

template <typename R, char C, int I, typename... Ts>
struct parser {
    // skip unknown tokens
    using type = parser_t<R, FIELD[I + 1], I + 1, Ts...>;
};

template <typename R, char C, typename... Ts>
struct parser<R, C, -1, Ts...> {
    // entry point
    using type = parser_t<tuple<>, C, 0, Ts...>;
};

template <typename R, char C, typename... Ts>
struct parser<R, C, sizeof(FIELD) - 1, Ts...> {
    // end of string
    using type = R;
};

template <typename R, int I, typename... Ts>
struct parser<R, '0', I, Ts...> {
    using type = parser_t<R, FIELD[I + 1], I + 1, Ts..., false_type>;
};

template <typename R, int I, typename... Ts>
struct parser<R, '1', I, Ts...> {
    using type = parser_t<R, FIELD[I + 1], I + 1, Ts..., true_type>;
};

template <typename R, int I, typename... Ts>
struct parser<R, '}', I, Ts...> {
    using type = parser_t<decltype(tuple_cat(declval<R>(), declval<tuple<tuple<Ts...>>>())), ',', I + 1>;
};

template <bool B, typename F, int X, int Y>
struct get_elem_helper {
    using type = decltype(get<X>(get<Y>(declval<F>())));
    //using type = tuple_element_t<X, tuple_element_t<Y, F>>;
};

template <typename F, int X, int Y>
struct get_elem_helper<true, F, X, Y> {
    using type = false_type;
};

template <typename F, int X, int Y>
struct get_elem {
    using type = typename get_elem_helper<(X < 0 || Y < 0 || X > WIDTH - 1 || Y > HEIGHT - 1), F, X, Y>::type;
};

template <typename F, int X, int Y> using get_elem_t = typename get_elem<F, X, Y>::type;

// The pattern to check.
template <typename F, int X, int Y>
struct collector {
    using type = tuple<
        get_elem_t<F, X - 1, Y - 1>,
        get_elem_t<F, X    , Y - 1>,
        get_elem_t<F, X + 1, Y - 1>,
        get_elem_t<F, X - 1, Y    >,
        get_elem_t<F, X + 1, Y    >,
        get_elem_t<F, X - 1, Y + 1>,
        get_elem_t<F, X    , Y + 1>,
        get_elem_t<F, X + 1, Y + 1>
    >;
};

template <typename F, int X, int Y> using collector_t = typename collector<F, X, Y>::type;

template <typename A, int I = 0> struct filter;
template <typename A, int I = 0> using filter_t = typename filter<A, I>::type;

template <typename A, int I>
struct filter {
    using type = conditional_t<
        is_same<tuple_element_t<I, A>, true_type&&>::value,

        decltype(tuple_cat(make_tuple(declval<true_type>()), declval<filter_t<A, I + 1>>())),
        filter_t<A, I + 1>
    >;
};

template <typename A>
struct filter<A, 8> {
    using type = tuple<>;
};

// The actual rules of the game.
template <typename T, int N>
struct cell_step {
    using type = conditional_t<
        is_same<T, false_type&&>::value,
        conditional_t<
            N == 3,
            true_type,
            false_type
        >,
        conditional_t<
            N == 2 || N == 3,
            true_type,
            false_type
        >
    >;
};

template <typename T, int N> using cell_step_t = typename cell_step<T, N>::type;

template <typename F, int X = 0, int Y = 0> struct row_step;
template <typename F, int X = 0, int Y = 0> using row_step_t = typename row_step<F, X, Y>::type;

template <typename F, int X, int Y>
struct row_step {
    using type = decltype(tuple_cat(make_tuple(declval<cell_step_t<get_elem_t<F, X, Y>, tuple_size<filter_t<collector_t<F, X, Y>>>::value>>()), declval<row_step_t<F, X + 1, Y>>()));
};

template <typename F, int Y>
struct row_step<F, WIDTH - 1, Y> {
    using type = std::tuple<cell_step_t<get_elem_t<F, WIDTH - 1, Y>, tuple_size<filter_t<collector_t<F, WIDTH - 1, Y>>>::value>>;
};

template <typename F, int X = 0, int Y = 0> struct field_step;
template <typename F, int X = 0, int Y = 0> using field_step_t = typename field_step<F, X, Y>::type;

template <typename F, int X, int Y>
struct field_step {
    using type = decltype(tuple_cat(make_tuple(declval<row_step_t<F, X, Y>>()), declval<field_step_t<F, X, Y + 1>>()));
};

template <typename F, int X>
struct field_step<F, X, HEIGHT - 1> {
    using type = std::tuple<row_step_t<F, X, HEIGHT - 1>>;
};


template <typename F, int N = 1> struct conway;
template <typename F, int N = 1> using conway_t = typename conway<F, N>::type;

template <typename F, int N>
struct conway {
    using type = conway_t<field_step_t<F>, N - 1>;
};

template <typename F>
struct conway<F, 1> {
    using type = field_step_t<F>;
};

template <typename F>
struct conway<F, 0> {
    using type = F;
};

// Stuff for building the result string.
template <typename T>
struct val_to_str;

template <>
struct val_to_str<true_type> {
    static inline constexpr char const value = '1';
};

template <>
struct val_to_str<false_type> {
    static inline constexpr char const value = '0';
};

template <typename, typename>
struct row_to_str;

template <typename R, size_t... Ri>
struct row_to_str<R, index_sequence<Ri...>> {
    using type = integer_sequence<char, '{', val_to_str<tuple_element_t<Ri, R>>::value..., '}', '\n'>;
};


template <typename, typename>
struct concat_seqs;

template <typename T, T... Ai, T... Bi>
struct concat_seqs<integer_sequence<T, Ai...>, integer_sequence<T, Bi...>> {
    using type = integer_sequence<T, Ai..., Bi...>;
};


template <typename F, size_t Y = 0>
struct f2_to_seq {
    using type = typename concat_seqs<typename row_to_str<tuple_element_t<Y, F>, make_index_sequence<WIDTH>>::type, typename f2_to_seq<F, Y + 1>::type>::type;
};

template <typename F>
struct f2_to_seq<F, HEIGHT> {
    using type = integer_sequence<char, '\0'>;
};

template <typename>
struct seq_to_array;

template <char... Ci>
struct seq_to_array<integer_sequence<char, Ci...>> {
    static inline constexpr char const value[] { Ci... };
};


auto result [[gnu::used]] = seq_to_array<f2_to_seq<conway_t<parser<std::tuple<>, '-', -1>::type, STEPS>>::type>::value;


int main() {
    std::cout << result;
}
