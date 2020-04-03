#define CATCH_CONFIG_MAIN
#include"../externals/catch2/catch.hpp"
#include"../include/ylib/containers/sparse_set.hpp"

using sset = sparse_set<int>;
using ssid = sset::external_id;

class test_case {
public:
    using on_init_fn = std::function<void(sset&)>;
    using before_push_fn = std::function<void(sset&, int cnt, int elem)>;
    using after_push_fn = std::function<void(sset&, int cnt, int elem, ssid)>;
    using finally_fn = std::function<void(sset&, int cnt)>;

    test_case on_init(on_init_fn fn) { m_on_init = std::move(fn); return *this; }
    test_case before_push(before_push_fn fn) { m_before_push = std::move(fn); return *this; }
    test_case after_push(after_push_fn fn) { m_after_push = std::move(fn); return *this; }
    test_case finally(finally_fn fn) { m_finally = std::move(fn); return *this; }

    test_case run(std::initializer_list<int> ints) {
        sset ss;

        m_on_init(ss);

        int count{};
        for (int x : ints) {
            m_before_push(ss, count, x);
            auto id = ss.push_back(x);
            count++;
            m_after_push(ss, count, x, id);
        }

        m_finally(ss, count);
        
        m_executed = true;

        return *this;
    }

    void run() {
        run({1, 12, 123, 1234, 12345});
    }

    ~test_case() {
        if (not m_executed)
            run();
    }

private:
    on_init_fn m_on_init = [](auto&& ...){};
    before_push_fn m_before_push = [](auto&& ...){};
    after_push_fn m_after_push = [](auto&& ...){};
    finally_fn m_finally = [](auto&& ...){};
    bool m_executed = false;
};

TEST_CASE("push_back() generates consecutive ids with generation 0 at first", "[sparse_set]") {
    test_case{}
        .after_push([] (sset& ss, int cnt, int elem, ssid id) {
            CHECK(id == ssid(cnt - 1, 0));
        });
}

TEST_CASE("empty() and size(), default-init", "[sparse_set]") {
    test_case{}
        .on_init([] (sset const& ss) {
            CHECK(ss.empty());
            CHECK(ss.size() == 0);
        });
}

TEST_CASE("empty() and size(), pushing", "[sparse_set]") {
    test_case{}
        .after_push([] (sset const& ss, int cnt, int elem, ssid id) {
            CHECK(not ss.empty());
            CHECK(ss.size() == cnt);
        });
}

TEST_CASE("no value before push, has value after push", "[sparse_set]") {
    test_case{}
        .before_push([] (sset const& ss, int cnt, int elem) {
            ssid id(cnt, 0);
            CHECK(not ss.contains(id));
            CHECK(not ss.at(id).has_value());
        })
        .after_push([] (sset const& ss, int cnt, int elem, ssid id) {
            CHECK(ss.contains(id));
            CHECK(ss[id] == elem);
            CHECK(ss.at(id) == tl::optional{ elem });
        });
}

TEST_CASE("doesn't contain ids with wrong generation", "[sparse_set]") {
    test_case{}
        .after_push([] (sset const& ss, int cnt, int elem, ssid id) {
            for (auto gen : { id.m_generation - 1, id.m_generation + 1}) {
                ssid id{ cnt - 1, gen };
                CHECK(not ss.contains(id));
                CHECK(not ss.at(id).has_value());
            }
        });
}

TEST_CASE("can erase pushed elements, cannot erase before push", "[sparse_set]") {
    test_case{}
        .before_push([] (sset& ss, int cnt, int elem) {
            ssid id{ cnt, 0 };
            CHECK(not ss.erase_at(id));
        })
        .finally([] (sset& ss, int cnt) {
            for (int i = 0; i < cnt; ++i) {
                CHECK(ss.erase_at(ssid{ i, 0 }));
            }
        });
}

TEST_CASE("generations grow after erase, internal_id is reused", "[sparse_set]") {
    test_case{}
        .after_push([] (sset& ss, int cnt, int elem, ssid id) {
            CHECK(id == ssid{ 0, cnt - 1u });
            CHECK(ss.erase_at(id));
            CHECK(not ss.erase_at(id));
        });
}

