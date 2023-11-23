#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

using namespace std;

/*
    left_child_idx = 2 * left_parent_idx + 1
    right_child_idx = 2 * right_parent_idx + 2
    left_parent_idx = (left_child_idx - 1) / 2
    right_parent_idx = (right_child_idx - 2) / 2
    Rule: left node indexes are always odd,
          right node indexes are always even.
*/
template <typename t_iter_t>
void max_heapify(t_iter_t begin, t_iter_t node)
{
#if 0
    // Recursive implementation: space complexity = O(n)
    //                           time complexity = O(lg(n))
    auto const node_idx = node - begin;
    auto const child_offset = (1 << (~node_idx & 0x1)) & 0x3;
    auto const parent_idx = (node_idx - child_offset) / 2;
    if (0 <= parent_idx)
    {
        auto parent = begin + parent_idx;
        if (*parent < *node)
        {
            std::swap(*parent, *node);
            max_heapify(begin, parent);
        }
    }
#else // #if 0
    // Iterative implementation: space complexity = O(1)
    //                           time complexity = O(lg(n))
    while (true)
    {
        auto const node_idx = node - begin;
        auto const child_offset = (1 << (~node_idx & 0x1)) & 0x3;
        auto const parent_idx = (node_idx - child_offset) / 2;
        if (0 <= parent_idx)
        {
            auto parent = begin + parent_idx;
            if (*parent < *node)
            {
                std::swap(*parent, *node);
                node = parent;

                continue;
            }
        }

        break;
    }
#endif // #if 0
}

template <typename t_iter_t>
void trickle_down_max_heap(t_iter_t begin, t_iter_t end, t_iter_t node)
{
    auto const ary_size = end - begin;
    auto const node_idx = node - begin;

    auto const left_child_idx = node_idx * 2 + 1;
    auto left_child = begin + left_child_idx;
    if (ary_size > left_child_idx
        && *left_child > *node)
    {
        std::swap(*left_child, *node);
        trickle_down_max_heap(begin, end, left_child);
    }

    auto const right_child_idx = node_idx * 2 + 2;
    auto right_child = begin + right_child_idx;
    if (ary_size > right_child_idx
        && *right_child > *node)
    {
        std::swap(*right_child, *node);
        trickle_down_max_heap(begin, end, right_child);
    }
}

/*
    Heap binary tree is left-weighted and stored in an array.

     0 1 2 3 4 5 6 7 8 9   (array indexes)
    [0 1 2 3 4 5 6 7 8 9]  (heap)

          Unheapified (invalid heap):

                0
             1     2
           3   4 5   6
          7 8 9

          Heapified (valid heap):

                9
             8     5
           6   7 1   4
          0 3 2
    
     0 1 2 3 4 5 6 7 8 9   (array indexes)
    [9 8 5 6 7 1 4 0 3 2]  (heap)

    o Add node to bottom leftmost available leaf.
    o Recursively max_heapify upward while the new value is greater than the parent, until the root is reached.

    for idx in range(0, size(ary)):
        max_heapify(ary, idx)
*/
template <typename t_iter_t>
void make_max_heap(t_iter_t begin, t_iter_t end)
{
    for (auto iter = begin; end != iter; ++iter)
    {
        max_heapify(begin, iter);
    }
}

template <typename t_item_t>
class heap_t
{
public:
    using container_t = std::vector<t_item_t>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    heap_t() = default;

    //!\brief Initialize from array.
    template<typename A, std::size_t S>
    heap_t(A const (&ary)[S])
        : heap_t{ary, ary + S}
    {
        // Do nothing.
    }

    //!\brief Initialize from begin/end iterator pair.
    template<typename t_iter_t>
    heap_t(t_iter_t begin, t_iter_t end)
        : array_(begin, end)
    {
        make_max_heap(this->begin(), this->end());
    }
    
    // //!\brief Initialize from initializer list.
    // heap_t(std::initializer_list<t_item_t> list)
    //     : array_(list.begin(), list.end())
    // {
    //     make_max_heap(array_.begin(), array_.end());
    // }
    
    //!\brief Initialize from initializer list, e.g. {1, 2, 3, 4, 5}.
    template<typename... t_val_t>
    heap_t(t_item_t&& val1, t_val_t&&... value)
        : array_{std::forward<t_item_t>(val1), std::forward<t_val_t>(value)...}
    {
        make_max_heap(begin(), end());
    }

    [[nodiscard]] iterator begin() { return array_.begin(); }
    [[nodiscard]] iterator end() { return array_.end(); }

    [[nodiscard]] const_iterator begin() const { return array_.begin(); }
    [[nodiscard]] const_iterator end() const { return array_.end(); }

    [[nodiscard]] std::size_t size() const { return array_.size(); }
    [[nodiscard]] bool empty() const { return array_.empty(); }
    
    [[nodiscard]] auto const& operator[](std::size_t idx) const { return array_[idx]; }
    
    t_item_t pop()
    {
        if (empty()) { throw std::runtime_error{"empty"}; }
        auto const result = (*this)[0];
        std::swap(*begin(), *(end() - 1));
        array_.resize(size() - 1);
        trickle_down_max_heap(begin(), end(), begin());
        return result;
    }

    //!\todo Add an item to the heap.
    heap_t& push(t_item_t value)
    {
        // Add the item to the leftmost available child position in the tree.
        // (Append the item to the array.)
        array_.emplace_back(std::move(value));

        // Heapify, starting from the new child to correctly position it within the tree.
        max_heapify(begin(), end() - 1);

        return *this;
    }

    //!\brief Add an item to or replace an item in the heap.
    heap_t& insert(iterator position, t_item_t value)
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
            auto const increment = *position < value;
            *position = std::move(value);
            if (increment)
            {
                max_heapify(begin(), std::move(position));
            }
            else
            {
                trickle_down_max_heap(begin(), end(), std::move(position));
            }
        }

        return *this;
    }

private:
    container_t array_;
};

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

template<typename T>
std::ostream&
operator<<(std::ostream& os, heap_t<T> const& heap)
{
    for (auto const& val : heap)
    {
        cout << val << ' ';
    }

    return os;
}

template class heap_t<int>;

int main(int, char**)
{
    int const init_val[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int const heapified_val[] = { 9, 8, 5, 6, 7, 1, 4, 0, 3, 2 };
    cout << "Before heapification:" << '\n' << init_val << '\n';
    
    { // New scope.
        auto heap = heap_t<int>{init_val};
        cout << "After heapification:" << '\n' << heap << '\n';
        assert(std::size(init_val) == std::size(heapified_val));
        assert(std::size(init_val) == heap.size());
        for (std::size_t idx = 0; std::size(heapified_val) > idx; ++idx)
        {
            assert(heapified_val[idx] == heap[idx]);
        }

        cout << "Extracting: " << '\n';
        for (int expected_value = 9; !heap.empty(); --expected_value)
        {
            auto const value = heap.pop();
            cout << value << ' ' << std::flush;
            assert(value == expected_value);
        }
        cout << std::endl;
    }

    cout << std::endl;

    { // New scope.
        auto heap = heap_t<int>{init_val};
        heap.push(10);
        cout << "Added '10': " << '\n';
        for (int expected_value = 10; !heap.empty(); --expected_value)
        {
            auto const value = heap.pop();
            cout << value << ' ' << std::flush;
            assert(value == expected_value);
        }
        cout << std::endl;
    }

    cout << std::endl;

    { // New scope.
        auto heap = heap_t<int>{init_val};
        heap.insert([&]{
            auto iter = heap.begin();
            for (; heap.end() != iter; ++iter)
            {
                if (5 == *iter)
                {
                    break;
                }
            }
            assert(heap.end() != iter);
            return iter;
        }(), 10);
        cout << "Changed '5' to '10': " << '\n';
        int const expected_values[] = { 10, 9, 8, 7, 6, 4, 3, 2, 1, 0 };
        for (int idx = 0; std::size(expected_values) > idx; ++idx)
        {
            auto const value = heap.pop();
            cout << value << ' ' << std::flush;
            assert(value == expected_values[idx]);
        }
        cout << std::endl;
    }

    cout << std::endl;

    { // New scope.
        auto heap = heap_t<int>{init_val};
        heap.insert([&]{
            auto iter = heap.begin();
            for (; heap.end() != iter; ++iter)
            {
                if (5 == *iter)
                {
                    break;
                }
            }
            assert(heap.end() != iter);
            return iter;
        }(), -1);
        cout << "Changed '5' to '-1': " << '\n';
        int const expected_values[] = { 9, 8, 7, 6, 4, 3, 2, 1, 0, -1 };
        for (int idx = 0; std::size(expected_values) > idx; ++idx)
        {
            auto const value = heap.pop();
            cout << value << ' ' << std::flush;
            assert(value == expected_values[idx]);
        }
        cout << std::endl;
    }

    cout << std::endl;

    { // New scope.
        auto heap = heap_t<int>{init_val};
        heap.insert([&]{
            auto iter = heap.begin();
            for (; heap.end() != iter; ++iter)
            {
                if (5 == *iter)
                {
                    break;
                }
            }
            assert(heap.end() != iter);
            return iter;
        }(), 5);
        cout << "Changed '5' to '5': " << '\n';
        int const expected_values[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
        for (int idx = 0; std::size(expected_values) > idx; ++idx)
        {
            auto const value = heap.pop();
            cout << value << ' ' << std::flush;
            assert(value == expected_values[idx]);
        }
        cout << std::endl;
    }

    return 0;
}

// End of file.
