#ifndef FREE_LIST_VECTOR
#define FREE_LIST_VECTOR

#include<vector>
#include<cassert>
#include<type_traits>
#include"../../../externals/tl/optional.hpp"

template<class ElemType, class IndexType = std::size_t>
class free_list_vector {
public:
    static_assert(std::is_trivial_v<ElemType>);
    static_assert(std::is_trivial_v<IndexType>);

    using value_type = ElemType;

private:
    union elem_data {
        ElemType m_elem;
        IndexType m_next_free;

        elem_data(ElemType const& elem)
            : m_elem(elem) {}
    };

public:
    static constexpr IndexType npos = -1;

    bool empty() const noexcept {
        return m_size == 0;
    }

    IndexType size() const noexcept {
        return m_size;
    }

    IndexType pool_size() const noexcept {
        assert(m_alive.size() == m_data.size());

        return m_alive.size();
    }

    IndexType push_back(ElemType const& elem) {
        if (m_next_free == npos) {
            return real_push_back(elem);
        } else {
            m_size++;
            return assign_at(pop_free(), elem);
        }
    }

    bool erase_at(IndexType pos) noexcept {
        if (pos < 0 or pos >= m_alive.size() or not m_alive[pos])
            return false;

        m_alive[pos] = false;
        --m_size;
        push_free(pos);
        return true;
    }

    ElemType& operator[](IndexType pos) noexcept {
        return unsafe_at_impl<ElemType>(*this, pos);
    }

    ElemType const& operator[](IndexType pos) const noexcept {
        return unsafe_at_impl<const ElemType>(*this, pos);
    }

    tl::optional<ElemType&> at(IndexType pos) noexcept {
        return safe_at_impl<ElemType>(*this, pos);
    }

    tl::optional<ElemType const&> at(IndexType pos) const noexcept {
        return safe_at_impl<const ElemType>(*this, pos);
    }

private:
    IndexType real_push_back(ElemType const& elem) {
        m_data.emplace_back(elem);
        m_alive.push_back(true);
        return m_size++;
    }

    IndexType assign_at(IndexType pos, ElemType const& elem) noexcept {
        assert(pos < m_data.size());

        m_data[pos] = elem;
        m_alive[pos] = true;
        return pos;
    }

    IndexType pop_free() noexcept {
        assert(m_next_free != npos);
        assert(m_last_free != npos);
        assert(m_next_free < m_alive.size());
        assert(not m_alive[m_next_free]);

        if (m_next_free == m_last_free) {
            m_last_free = npos;
        }
        return std::exchange(m_next_free, m_data[m_next_free].m_next_free);
    }

    void push_free(IndexType pos) noexcept {
        assert(pos < m_alive.size());
        assert(not m_alive[pos]);

        if (m_last_free != npos) {
            assert(m_last_free < m_alive.size());
            assert(not m_alive[m_last_free]);
            assert(m_data[m_last_free].m_next_free == npos);
            assert(m_next_free != npos);

            m_data[m_last_free].m_next_free = pos;
        } else {
            assert(m_next_free == npos);
            m_next_free = pos;
        }

        m_last_free = pos;
        m_data[pos].m_next_free = npos;
    }

    template<class Res, class FLVect>
    static Res& unsafe_at_impl(FLVect& flv, IndexType pos) noexcept {
        static_assert(std::is_same_v<std::decay_t<FLVect>, free_list_vector>);
        static_assert(std::is_same_v<std::decay_t<Res>, ElemType>);

        assert(pos >= 0 and pos < flv.m_alive.size());
        assert(flv.m_alive[pos]);

        return flv.m_data[pos].m_elem;
    }

    template<class Res, class FLVect>
    static tl::optional<Res&> safe_at_impl(FLVect& flv, IndexType pos) noexcept {
        static_assert(std::is_same_v<std::decay_t<FLVect>, free_list_vector>);
        static_assert(std::is_same_v<std::decay_t<Res>, ElemType>);

        if (pos >= 0 and pos < flv.m_alive.size() and flv.m_alive[pos]) {
            return { flv.m_data[pos].m_elem };
        } else {
            return tl::nullopt;
        }
    }

    std::vector<elem_data> m_data;
    std::vector<bool> m_alive;
    IndexType m_next_free = npos;
    IndexType m_last_free = npos;
    IndexType m_size = 0;
};

#endif // FREE_LIST_VECTOR

