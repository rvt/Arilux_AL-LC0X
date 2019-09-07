#include <catch2/catch.hpp>
#include <stdint.h>
#include <iostream>
#include <Helpers.h>


const float valuemap[] = { 1, 5, 10, 10 };

TEST_CASE("should map value", "[helpers]") {
    REQUIRE(Helpers::lookupTableInterp(valuemap, 4, 0.f) == Approx(1.0f));
    REQUIRE(Helpers::lookupTableInterp(valuemap, 4, 0.5f) == Approx(3.0f));
    REQUIRE(Helpers::lookupTableInterp(valuemap, 4, 1.f) == Approx(5.0f));
    REQUIRE(Helpers::lookupTableInterp(valuemap, 4, 1.75f) == Approx(8.75f));
    REQUIRE(Helpers::lookupTableInterp(valuemap, 4, 2.f) == Approx(10.0f));
    REQUIRE(Helpers::lookupTableInterp(valuemap, 4, 2.5f) == Approx(10.0f));
}

TEST_CASE("should not go out of range", "[helpers]") {
    REQUIRE(Helpers::lookupTableInterp(valuemap, 3, -10.f) == Approx(1.0f));
    REQUIRE(Helpers::lookupTableInterp(valuemap, 3, 20.f) == Approx(10.0f));
}