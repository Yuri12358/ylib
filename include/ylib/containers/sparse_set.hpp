#include<type_traits>
#include<vector>
#include<cstdint>
#include<ostream>
#include"free_list_vector.hpp"
#include"../../../externals/tl/optional.hpp"

template<class ElemType>
class sparse_set {
private:
    using internal_id_type = std::int32_t;
    using generation_type = std::uint32_t;
    using index_type = std::int32_t;

public:
    class external_id {
    public:
        internal_id_type m_internal_id;
        generation_type m_generation;

        external_id(internal_id_type internal_id, generation_type generation)
            : m_internal_id(internal_id)
            , m_generation(generation) {}

        bool operator ==(external_id other) const noexcept {
            return m_internal_id == other.m_internal_id and m_generation == other.m_generation;
        }

        friend std::ostream& operator <<(std::ostream& os, external_id i) {
            return os << "{id=" << i.m_internal_id << ", gen=" << i.m_generation << "}";
        }
    };

    bool empty() const noexcept {
        return m_dense.empty();
    }

    std::size_t size() const noexcept {
        return m_dense.size();
    }

    bool contains(external_id id) const noexcept {
        return m_sparse.at(id.m_internal_id)
            .map_or([this, id] (index_type index) {
                return is_valid_id_for(index, id);
            }, false);
    }

    external_id push_back(ElemType const& elem) {
        index_type pos = m_dense.size();
        m_dense.push_back(elem);

        auto internal_id = m_sparse.push_back(pos);
        m_generations.resize(m_sparse.pool_size());

        m_dense_ids.push_back(internal_id);

        return external_id(internal_id, m_generations[internal_id]);
    }

    ElemType& operator[](external_id id) noexcept {
        return unsafe_at_impl<ElemType>(*this, id);
    }

    ElemType const& operator[](external_id id) const noexcept {
        return unsafe_at_impl<const ElemType>(*this, id);
    }

    tl::optional<ElemType&> at(external_id id) noexcept {
        return safe_at_impl<ElemType>(*this, id);
    }

    tl::optional<ElemType const&> at(external_id id) const noexcept {
        return safe_at_impl<const ElemType>(*this, id);
    }

    bool erase_at(external_id id) noexcept {
        return m_sparse.at(id.m_internal_id)
            .map_or([this, id] (index_type index) {
                if (not is_valid_id_for(index, id))
                    return false;

                m_dense[index] = m_dense.back();
                m_dense.pop_back();

                m_dense_ids[index] = m_dense_ids.back();
                m_dense_ids.pop_back();

                m_sparse[m_dense_ids[index]] = index;

                m_sparse.erase_at(id.m_internal_id);
                ++m_generations[id.m_internal_id];

                return true;
            }, false);
    }

private:
    template<class Res, class SparseSet>
    static Res& unsafe_at_impl(SparseSet& ss, external_id id) {
        static_assert(std::is_same_v<std::decay_t<SparseSet>, sparse_set>);
        static_assert(std::is_same_v<std::decay_t<Res>, ElemType>);

        assert(ss.contains(id));

        return ss.m_dense[ss.m_sparse[id.m_internal_id]];
    }

    template<class Res, class SparseSet>
    static tl::optional<Res&> safe_at_impl(SparseSet& ss, external_id id) {
        static_assert(std::is_same_v<std::decay_t<SparseSet>, sparse_set>);
        static_assert(std::is_same_v<std::decay_t<Res>, ElemType>);

        return ss.m_sparse.at(id.m_internal_id)
            .and_then([&ss, id] (index_type index) -> tl::optional<Res&> {
                if (ss.is_valid_id_for(index, id)) {
                    return { ss.m_dense[index] };
                } else {
                    return tl::nullopt;
                }
            });
    }

    bool is_valid_id_for(index_type index, external_id id) const noexcept {
        assert(index < m_dense.size());
        return m_dense_ids[index] == id.m_internal_id
            and m_generations[id.m_internal_id] == id.m_generation;
    }

    free_list_vector<index_type, internal_id_type> m_sparse;
    std::vector<generation_type> m_generations;
    std::vector<internal_id_type> m_dense_ids;
    std::vector<ElemType> m_dense;
};

