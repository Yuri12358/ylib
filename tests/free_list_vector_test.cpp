#include"../include/ylib/containers/free_list_vector.hpp"

#define CATCH_CONFIG_MAIN
#include"../externals/catch2/catch.hpp"

TEST_CASE("push_back generates consecutive ids", "[free_list_vector]") {
    free_list_vector<int> flv;
    int count = 0;
    for (int x : {42, 100500, 100500}) {
        REQUIRE(flv.push_back(x) == count++);
    }
}

TEST_CASE("size returns correct value", "[free_list_vector]") {
    free_list_vector<int> flv;
    int count = 0;
    for (int x : {100, 101, 102}) {
        flv.push_back(x);
    }
    REQUIRE(flv.size() == 3);
    flv.erase_at(0);
    flv.erase_at(2);

    REQUIRE(flv.size() == 1);
    flv.push_back(100);
    REQUIRE(flv.size() == 2);
}

TEST_CASE("empty on creation", "[free_list_vector]") {
    free_list_vector<int> flv;
    
    CHECK(flv.empty());
    CHECK(flv.size() == 0);
}

TEST_CASE("empty return false after pushing", "[free_list_vector]") {
    free_list_vector<int> flv;

    flv.push_back(12);
    REQUIRE(not flv.empty());
    flv.push_back(123);
    REQUIRE(not flv.empty());
}

TEST_CASE("empty after erasure of all the elements", "[free_list_vector]") {
    free_list_vector<int> flv;

    int count{};
    for (int x : {42, 100500, 100500}) {
        flv.push_back(x);
        ++count;
    }

    for (int i = 0; i < count; ++i) {
        flv.erase_at(i);
    }

    REQUIRE(flv.empty());
}

TEST_CASE("push_back reuses ids", "[free_list_vector]") {
    free_list_vector<int> flv;

    int count{};
    for (int x : {42, 100500, 100500, 123, 101010}) {
        flv.push_back(x);
        ++count;
    }

    int idx[] = {3, 2, 0};
    for (int x : idx) {
        flv.erase_at(x);
    }

    for (int x : idx) {
        REQUIRE(flv.push_back(0) == x);
    }
}

TEST_CASE("erase_at returns whether the erasure actually took place", "[free_list_vector]") {
    free_list_vector<int> flv;

    int count{};
    for (int x : {42, 100500, 100500, 123, 101010}) {
        flv.push_back(x);
        ++count;
    }

    CHECK(flv.erase_at(2));
    CHECK(not flv.erase_at(2));
    CHECK(not flv.erase_at(-1));
    CHECK(not flv.erase_at(flv.npos));
    CHECK(not flv.erase_at(100500));

    flv.push_back(123123);
    CHECK(flv.erase_at(2));
}

TEST_CASE("brackets test", "[free_list_vector]") {
    free_list_vector<int> flv;

    int count{};
    int elems[] = {42, 100500, 100500, 123, 101010};
    for (int x : elems) {
        flv.push_back(x);
        REQUIRE(flv[count] == x);
        ++count;
    }
}

TEST_CASE("at() test", "[free_list_vector]") {
    free_list_vector<int> flv;

    int count{};
    int elems[] = {42, 100500, 100500, 123, 101010};
    for (int x : elems) {
        flv.push_back(x);
        CHECK(flv.at(count) == tl::optional{ x });
        ++count;
    }
    CHECK(flv.at(-1) == tl::nullopt);
    CHECK(flv.at(flv.npos) == tl::nullopt);
    CHECK(flv.at(count) == tl::nullopt);

    flv.erase_at(2);

    CHECK(flv.at(2) == tl::nullopt);

    flv.push_back(0);

    CHECK(flv.at(2) == tl::optional{ 0 });
}

TEST_CASE("pool_size() must return the internal vectors size", "[free_list_vector]") {
    free_list_vector<int> flv;

    int count{};
    int elems[] = {42, 100500, 100500, 123, 101010};
    for (int x : elems) {
        flv.push_back(x);
        REQUIRE((++count) == flv.pool_size());
    }
    flv.erase_at(2);
    REQUIRE(flv.pool_size() == count);
    flv.push_back(0);
    flv.push_back(0);
    REQUIRE(flv.pool_size() == (count + 1));
}

