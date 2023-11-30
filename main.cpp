/*!
    \file "main.cpp"

    Author: Matt Ervin <matt@impsoftware.org>
    Formatting: 4 spaces/tab (spaces only; no tabs), 120 columns.
    Doc-tool: Doxygen (http://www.doxygen.com/)

    Experimental C++ heap implementation (for learning and practice).
*/

//!\sa https://github.com/doctest/doctest/blob/master/doc/markdown/main.md
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <algorithm>
#include <cassert>
#include <doctest/doctest.h> //!\sa https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md
#include <functional>
#include <iterator>
#include <iostream>
#include <vector>

// #define USE_RECURSIVE_HEAPIFY
// #define USE_PRECISION_CHILD_OFFSET

using namespace std;

/*
    left_child_idx = 2 * left_parent_idx + 1
    right_child_idx = 2 * right_parent_idx + 2
    left_parent_idx = (left_child_idx - 1) / 2
    right_parent_idx = (right_child_idx - 2) / 2
    Rule: left node indexes are always odd,
          right node indexes are always even.
*/
template <
    typename t_iter_t
    , typename t_cmp_op_t = std::greater<
        typename std::iterator_traits<t_iter_t>::value_type
    >
>
struct heapify_up_t
{
    using iter_type = t_iter_t;
    using cmp_op_type = t_cmp_op_t;

    void operator()(iter_type begin, iter_type node)
    {
#if defined(USE_RECURSIVE_HEAPIFY)
        // Recursive implementation: space complexity = O(n)
        //                           time complexity = O(lg(n))
        auto const node_idx = node - begin;
#if USE_PRECISION_CHILD_OFFSET
        auto const child_offset = (1 << (~node_idx & 0x1)) & 0x3;
#else // #if defined(USE_PRECISION_CHILD_OFFSET)
        auto const child_offset = 1;
#endif // #if defined(USE_PRECISION_CHILD_OFFSET)
        auto const parent_idx = (node_idx - child_offset) / 2;
        if (0 <= parent_idx)
        {
            auto parent = begin + parent_idx;
            if (cmp_op_type{}(*node, *parent))
            {
                std::swap(*parent, *node);
                heapify_up_t{}(std::move(begin), std::move(parent));
            }
        }
#else // #if defined(USE_RECURSIVE_HEAPIFY)
        // Iterative implementation: space complexity = O(1)
        //                           time complexity = O(lg(n))
        while (true)
        {
            auto const node_idx = node - begin;
#if defined(USE_PRECISION_CHILD_OFFSET)
            auto const child_offset = (1 << (~node_idx & 0x1)) & 0x3;
#else // #if defined(USE_PRECISION_CHILD_OFFSET)
            auto const child_offset = 1;
#endif // #if defined(USE_PRECISION_CHILD_OFFSET)
            auto const parent_idx = (node_idx - child_offset) / 2;
            if (0 <= parent_idx)
            {
                auto parent = begin + parent_idx;
                if (cmp_op_type{}(*node, *parent))
                {
                    std::swap(*parent, *node);
                    node = parent;

                    continue;
                }
            }

            break;
        }
#endif // #if defined(USE_RECURSIVE_HEAPIFY)
    }
};

template <
    typename t_iter_t
    , typename t_cmp_op_t = std::greater<
        typename std::iterator_traits<t_iter_t>::value_type
    >
>
struct heapify_down_t
{
    using iter_type = t_iter_t;
    using cmp_op_type = t_cmp_op_t;

    void operator()(iter_type begin, iter_type end, iter_type node)
    {
        auto const ary_size = end - begin;
        decltype(node) child = node;

        while (true)
        {
            auto const node_idx = node - begin;

            auto const left_child_idx = node_idx * 2 + 1;
            auto left_child = begin + left_child_idx;
            if (ary_size > left_child_idx
                && cmp_op_type{}(*left_child, *child))
            {
                child = left_child;
            }

            auto const right_child_idx = node_idx * 2 + 2;
            auto right_child = begin + right_child_idx;
            if (ary_size > right_child_idx
                && cmp_op_type{}(*right_child, *child))
            {
                child = right_child;
            }
        
            auto const value_has_stopped_moving = node == child;
            if (value_has_stopped_moving)
            {
                break;
            }

            std::swap(*child, *node);
            node = child;
        }
    }
};

/*
    Heap [complete] binary tree is left-weighted and stored in an array.

     0 1 2 3 4 5 6 7 8 9   (array indexes)
    [0 1 2 3 4 5 6 7 8 9]  (heap)

          Unheapified (usually invalid; happend to be valid min heap due to initial array values):

                0
             1     2
           3   4 5   6
          7 8 9

          Heapified (valid MAX heap):

                9
             8     5
           6   7 1   4
          0 3 2
    
     0 1 2 3 4 5 6 7 8 9   (array indexes)
    [9 8 5 6 7 1 4 0 3 2]  (max heap)

          Heapified (valid MIN heap):

                0
             1     2
           3   4 5   6
          7 8 9
    
     0 1 2 3 4 5 6 7 8 9   (array indexes)
    [0 1 2 3 4 5 6 7 8 9]  (min heap)

    Max heap:
        o Add node to bottom leftmost available leaf.
        o Recursively max_heapify_up_t upward while the new value is greater than the parent, until the root is reached.

        for idx in range(0, size(ary)):
            max_heapify_up_t(ary, idx)
*/
template <
    typename t_iter_t
    , typename t_cmp_op_t = std::greater<
        typename std::iterator_traits<t_iter_t>::value_type
    >
>
struct heapify_t
{
    using iter_type = t_iter_t;
    using heapify_up_type = heapify_up_t<iter_type, t_cmp_op_t>;
    using heapify_down_type = heapify_down_t<iter_type, t_cmp_op_t>;
    using cmp_op_type = t_cmp_op_t;

    void operator()(iter_type begin, iter_type end)
    {
        for (auto iter = begin; end != iter; ++iter)
        {
            heapify_up_type{}(begin, iter);
        }
    }
};

template<typename t_iter_t>
using max_heapify_t = heapify_t<
    t_iter_t
    , std::greater<
        typename std::iterator_traits<t_iter_t>::value_type
    >
>;

template<typename t_iter_t>
using min_heapify_t = heapify_t<
    t_iter_t
    , std::less<
        typename std::iterator_traits<t_iter_t>::value_type
    >
>;

template <
    typename t_item_t
    , typename t_container_t = std::vector<t_item_t>
    , typename t_heapify_t = max_heapify_t<typename t_container_t::iterator>
>
class heap_t
{
public:
    using item_type = t_item_t;
    using container_t = t_container_t;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;
    using heapify_type = t_heapify_t;
    using heapify_up_type = typename heapify_type::heapify_up_type;
    using heapify_down_type = typename heapify_type::heapify_down_type;
    using cmp_op_type = typename heapify_type::cmp_op_type;

    heap_t() = default;

    //!\brief Initialize from array.
    template<typename A, std::size_t S>
    heap_t(A const (&ary)[S])
        : heap_t{ary, ary + S}
    {
        // Do nothing.
    }

    //!\brief Initialize from begin/end iterator pair.
    template<typename I>
    heap_t(I begin, I end)
        : array_(begin, end)
    {
        heapify_type{}(this->begin(), this->end());
    }
    
#if 0
    // //!\brief Initialize from initializer list.
    heap_t(std::initializer_list<item_type> list)
        : array_(list.begin(), list.end())
    {
        heapify_type{}(array_.begin(), array_.end());
    }
#endif // #if 0
    
    //!\brief Initialize from initializer list, e.g. {1, 2, 3, 4, 5}.
    template<typename... t_vals_t>
    heap_t(item_type&& val1, t_vals_t&&... value)
        : array_{std::forward<item_type>(val1), std::forward<t_vals_t>(value)...}
    {
        heapify_type{}(begin(), end());
    }

    [[nodiscard]] iterator begin() { return array_.begin(); }
    [[nodiscard]] iterator end() { return array_.end(); }

    [[nodiscard]] const_iterator begin() const { return array_.begin(); }
    [[nodiscard]] const_iterator end() const { return array_.end(); }

    [[nodiscard]] std::size_t size() const { return array_.size(); }
    [[nodiscard]] bool empty() const { return array_.empty(); }
    
    [[nodiscard]] auto const& operator[](std::size_t idx) const { return array_[idx]; }
    
    //!\brief Remove the head element from the heap.
    item_type top()
    {
        if (empty()) { throw std::out_of_range{"empty"}; }
        return (*this)[0];
    }
    
    //!\brief Remove the head element from the heap.
    [[nodiscard]] item_type pop_value()
    {
        if (empty()) { throw std::out_of_range{"empty"}; }
        auto const result = (*this)[0];
        pop();
        return result;
    }
    
    //!\brief Remove the head element from the heap.
    void pop()
    {
        if (empty()) { throw std::out_of_range{"empty"}; }
        std::swap(*begin(), *(end() - 1));
        array_.resize(size() - 1);
        heapify_down_type{}(begin(), end(), begin());
    }

    //!\todo Add an element to the heap.
    heap_t& push(item_type value)
    {
        // Add the item to the leftmost available child position in the tree.
        // (Append the item to the array.)
        array_.emplace_back(std::move(value));

        // Heapify, starting from the new child to correctly position it within the tree.
        heapify_up_type{}(begin(), end() - 1);

        return *this;
    }

    //!\brief Add an element to or replace an element in the heap.
    heap_t& insert(iterator position, item_type value)
    {
        if (end() == position)
        {
            push(std::move(value));
        }
        else if (*position == value)
        {
            // Update existing item (in case updating has side effects.)
            *position = std::move(value);
        }
        else
        {
            auto const move_value_up_tree = cmp_op_type{}(value, *position);
            *position = std::move(value);
            if (move_value_up_tree)
            {
                heapify_up_type{}(begin(), std::move(position));
            }
            else
            {
                heapify_down_type{}(begin(), end(), std::move(position));
            }
        }

        return *this;
    }

private:
    container_t array_;
};

template <typename t_item_t>
using max_heap_t = heap_t<
    t_item_t
    , std::vector<t_item_t>
    , max_heapify_t<typename std::vector<t_item_t>::iterator>
>;

template <typename t_item_t>
using min_heap_t = heap_t<
    t_item_t
    , std::vector<t_item_t>
    , min_heapify_t<typename std::vector<t_item_t>::iterator>
>;

// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]

template<std::size_t S>
std::ostream&
operator<<(std::ostream& os, int const (&items)[S])
{
    for (auto const& val : items)
    {
        cout << val << ' ';
    }

    return os;
}

template<typename T>
std::ostream&
operator<<(std::ostream& os, std::initializer_list<T> const heap)
{
    for (auto const& val : heap)
    {
        cout << val << ' ';
    }

    return os;
}

template<typename I, typename V, typename H>
std::ostream&
operator<<(std::ostream& os, heap_t<I, V, H> const& heap)
{
    for (auto const& val : heap)
    {
        cout << val << ' ';
    }

    return os;
}

// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]

template <
    class I
    , typename t_heapify_t = max_heapify_t<I>
>
struct heap_sort_t
{
    using heapify_type = t_heapify_t;
    using heapify_down_type = typename heapify_type::heapify_down_type;

    void operator()(I begin, I end)
    {
        auto const empty = end == begin;
        if (!empty)
        {
            heapify_type{}(begin, end);
            while (begin < end)
            {
                auto const next_value = *begin;
                *begin = std::move(*(end - 1)); // Move the last item in the tree to the root.
                --end; // Remove the item from the collection.
                heapify_down_type{}(begin, end, begin);
                *end = std::move(next_value); // Store the removed value in the unused space at the end of the collection.
            }
        }
    }
};

template <class I>
inline void heap_sort_ascending(I begin, I end)
{
    return heap_sort_t<I, max_heapify_t<I>>{}(std::move(begin), std::move(end));
}

template <class T, std::size_t S>
inline void heap_sort_ascending(T (&ary)[S])
{
    return heap_sort_ascending(ary, ary + S);
}

template <class I>
inline void heap_sort(I begin, I end)
{
    return heap_sort_ascending(std::move(begin), std::move(end));
}

template <class T, std::size_t S>
inline void heap_sort(T (&ary)[S])
{
    return heap_sort_ascending(ary);
}

template <class I>
inline void heap_sort_decending(I begin, I end)
{
    return heap_sort_t<I, min_heapify_t<I>>{}(std::move(begin), std::move(end));
}

template <class T, std::size_t S>
inline
void
heap_sort_decending(T (&ary)[S])
{
    return heap_sort_decending(ary, ary + S);
}

// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]

//!< Explicitly instantiate MAX heap template to ensure all of it compiles.
template class heap_t<
    int
    , std::vector<int>
    , max_heapify_t<typename std::vector<int>::iterator>
>;

static int const max_heap_init_val[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

TEST_CASE("max_heap_heapification")
{
    cout << "((( max_heap_heapification )))" << std::endl;
    cout << "Before heapification: " << max_heap_init_val << '\n';
    auto heap = max_heap_t<int>{max_heap_init_val};
    static int const heapified_val[] = { 9, 8, 5, 6, 7, 1, 4, 0, 3, 2 };
    cout << "After heapification: " << heap << '\n';
    static_assert(std::size(max_heap_init_val) == std::size(heapified_val));
    CHECK(std::size(max_heap_init_val) == heap.size());
    for (std::size_t idx = 0; std::size(heapified_val) > idx; ++idx)
    {
        CHECK(heapified_val[idx] == heap[idx]);
    }

    cout << "Extracting: ";
    for (int expected_value = 9; !heap.empty(); --expected_value)
    {
        auto const value = heap.pop_value();
        cout << value << ' ';
        CHECK(value == expected_value);
    }
    cout << std::endl;
}

TEST_CASE("max_heap_push")
{
    cout << "((( max_heap_push )))" << std::endl;
    auto heap = max_heap_t<int>{max_heap_init_val};
    heap.push(10);
    cout << "Added '10': " << heap << '\n';
    cout << "Extracting: ";
    for (int expected_value = 10; !heap.empty(); --expected_value)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_value);
    }
    cout << std::endl;
}

TEST_CASE("max_heap_increment")
{
    cout << "((( max_heap_increment )))" << std::endl;
    auto heap = max_heap_t<int>{max_heap_init_val};
    heap.insert([&]{
        auto iter = heap.begin();
        for (; heap.end() != iter; ++iter)
        {
            if (5 == *iter)
            {
                break;
            }
        }
        CHECK(heap.end() != iter);
        return iter;
    }(), 10);
    cout << "Changed '5' to '10': " << heap << '\n';
    cout << "Extracting: ";
    int const expected_values[] = { 10, 9, 8, 7, 6, 4, 3, 2, 1, 0 };
    for (int idx = 0; std::size(expected_values) > idx; ++idx)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_values[idx]);
    }
    cout << std::endl;
}

TEST_CASE("max_heap_decrement")
{
    cout << "((( max_heap_decrement )))" << std::endl;
    auto heap = max_heap_t<int>{max_heap_init_val};
    heap.insert([&]{
        auto iter = heap.begin();
        for (; heap.end() != iter; ++iter)
        {
            if (5 == *iter)
            {
                break;
            }
        }
        CHECK(heap.end() != iter);
        return iter;
    }(), -1);
    cout << "Changed '5' to '-1': " << heap << '\n';
    cout << "Extracting: ";
    int const expected_values[] = { 9, 8, 7, 6, 4, 3, 2, 1, 0, -1 };
    for (int idx = 0; std::size(expected_values) > idx; ++idx)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_values[idx]);
    }
    cout << std::endl;
}

TEST_CASE("max_heap_insert_same")
{
    cout << "((( max_heap_insert_same )))" << std::endl;
    auto heap = max_heap_t
    <int>{max_heap_init_val};
    heap.insert([&]{
        auto iter = heap.begin();
        for (; heap.end() != iter; ++iter)
        {
            if (5 == *iter)
            {
                break;
            }
        }
        CHECK(heap.end() != iter);
        return iter;
    }(), 5);
    cout << "Changed '5' to '5': " << heap << '\n';
    cout << "Extracting: ";
    int const expected_values[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    for (int idx = 0; std::size(expected_values) > idx; ++idx)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_values[idx]);
    }
    cout << std::endl;
}

TEST_CASE("max_heap_sort_ascending")
{
    cout << "((( max_heap_sort_ascending )))" << std::endl;
    int values[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    cout << "Before sorting: " << values << '\n';
    heap_sort_ascending(values);
    cout << "After sorting: ";
    for (std::size_t idx = 0; std::size(values) > idx; ++idx)
    {
        auto const expected_value = idx;
        auto const value = values[idx];
        cout << value << ' ';
        CHECK(value == expected_value);
    }
    cout << std::endl;
}

// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]

//!< Explicitly instantiate MIN heap template to ensure all of it compiles.
template class heap_t<
    int
    , std::vector<int>
    , min_heapify_t<typename std::vector<int>::iterator>
>;

static int const min_heap_init_val[] = { 9, 8, 5, 6, 7, 1, 4, 0, 3, 2 };

TEST_CASE("min_heap_heapification")
{
    /*
            Heapified (valid MIN heap):

                    0
                 1     4
               3   2 8   5
              9 6 7
        
         0 1 2 3 4 5 6 7 8 9   (array indexes)
        [0 1 4 3 2 8 5 9 6 7]  (min heap)
    */    
    cout << "((( min_heap_heapification )))" << std::endl;
    cout << "Before heapification: " << min_heap_init_val << '\n';
    auto heap = min_heap_t<int>{min_heap_init_val};
    int const heapified_val[] = { 0, 1, 4, 3, 2, 8, 5, 9, 6, 7 };
    cout << "After heapification: " << heap << '\n';
    static_assert(std::size(min_heap_init_val) == std::size(heapified_val));
    CHECK(std::size(min_heap_init_val) == heap.size());
    for (std::size_t idx = 0; std::size(heapified_val) > idx; ++idx)
    {
        CHECK(heapified_val[idx] == heap[idx]);
    }

    cout << "Extracting: ";
    for (int expected_value = 0; !heap.empty(); ++expected_value)
    {
        auto const value = heap.pop_value();
        cout << value << ' ';
        CHECK(value == expected_value);
    }
    cout << std::endl;
}

TEST_CASE("min_heap_push")
{
    cout << "((( min_heap_push )))" << std::endl;
    auto heap = min_heap_t<int>{min_heap_init_val};
    heap.push(10);
    cout << "Added '10': " << heap << '\n';
    cout << "Extracting: ";
    for (int expected_value = 0; !heap.empty(); ++expected_value)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_value);
    }
    cout << std::endl;
}

TEST_CASE("min_heap_increment")
{
    cout << "((( min_heap_increment )))" << std::endl;
    auto heap = min_heap_t<int>{min_heap_init_val};
    heap.insert([&]{
        auto iter = heap.begin();
        for (; heap.end() != iter; ++iter)
        {
            if (5 == *iter)
            {
                break;
            }
        }
        CHECK(heap.end() != iter);
        return iter;
    }(), 10);
    cout << "Changed '5' to '10': " << heap << '\n';
    cout << "Extracting: ";
    int const expected_values[] = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10 };
    for (int idx = 0; std::size(expected_values) > idx; ++idx)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_values[idx]);
    }
    cout << std::endl;
}

TEST_CASE("min_heap_decrement")
{
    cout << "((( min_heap_decrement )))" << std::endl;
    auto heap = min_heap_t<int>{min_heap_init_val};
    heap.insert([&]{
        auto iter = heap.begin();
        for (; heap.end() != iter; ++iter)
        {
            if (5 == *iter)
            {
                break;
            }
        }
        CHECK(heap.end() != iter);
        return iter;
    }(), -1);
    cout << "Changed '5' to '-1': " << heap << '\n';
    cout << "Extracting: ";
    int const expected_values[] = { -1, 0, 1, 2, 3, 4, 6, 7, 8, 9 };
    for (int idx = 0; std::size(expected_values) > idx; ++idx)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_values[idx]);
    }
    cout << std::endl;
}

TEST_CASE("min_heap_insert_same")
{
    cout << "((( min_heap_insert_same )))" << std::endl;
    auto heap = min_heap_t<int>{min_heap_init_val};
    heap.insert([&]{
        auto iter = heap.begin();
        for (; heap.end() != iter; ++iter)
        {
            if (5 == *iter)
            {
                break;
            }
        }
        CHECK(heap.end() != iter);
        return iter;
    }(), 5);
    cout << "Changed '5' to '5': " << heap << '\n';
    cout << "Extracting: ";
    int const expected_values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    for (int idx = 0; std::size(expected_values) > idx; ++idx)
    {
        auto const value = heap.pop_value();
        cout << value << ' ' << std::flush;
        CHECK(value == expected_values[idx]);
    }
    cout << std::endl;
}

TEST_CASE("min_heap_sort_descending")
{
    cout << "((( min_heap_sort_descending )))" << std::endl;
    int values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    cout << "Before sorting: " << values << '\n';
    heap_sort_decending(values);
    cout << "After sorting: ";
    for (std::size_t idx = 0; std::size(values) > idx; ++idx)
    {
        auto const expected_value = std::size(values) - idx - 1;
        auto const value = values[idx];
        cout << value << ' ';
        CHECK(value == expected_value);
    }
    cout << std::endl;
}

/*
    End of "main.cpp"
*/
