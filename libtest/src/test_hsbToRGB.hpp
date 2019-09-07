#include <catch2/catch.hpp>
#include <stdint.h>
#include <iostream>
#include "arduinostubs.hpp"
#include <hsbToRGB.h>


float rgb[] = {-1.f, -1.f, -1.f};
HSBToRGB* test = HsbToRGBGeneric::genericLedStrip();

///////////////// RED /////////////////
TEST_CASE("should convert to R s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(0.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(100.f));
    REQUIRE(rgb[1] == Approx(0.f));
    REQUIRE(rgb[2] == Approx(0.f));
}

TEST_CASE("should convert to R s=100 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(0.f, 100.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(18.4186f));
    REQUIRE(rgb[1] == Approx(0.f));
    REQUIRE(rgb[2] == Approx(0.f));
}

TEST_CASE("should convert to R s100=0 b=0", "[HsbToRGBGeneric]") {
    test->toRgb(0.f, 100.f, 0.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f));
    REQUIRE(rgb[1] == Approx(0.f));
    REQUIRE(rgb[2] == Approx(0.f));
}

///////////////// GREEN /////////////////
TEST_CASE("should convert to G s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(135.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f));
    REQUIRE(rgb[1] == Approx(90.625f));
    REQUIRE(rgb[2] == Approx(9.375f));
}

TEST_CASE("should convert to G s=100 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(135.f, 100.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f));
    REQUIRE(rgb[1] == Approx(16.6919f));
    REQUIRE(rgb[2] == Approx(1.72675f));
}

TEST_CASE("should convert to G s=100 b=0", "[HsbToRGBGeneric]") {
    test->toRgb(135.f, 100.f, 0.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f));
    REQUIRE(rgb[1] == Approx(0.f));
    REQUIRE(rgb[2] == Approx(0.f));
}

///////////////// BLUE /////////////////
TEST_CASE("should convert to B s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(225.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(15.625f));
    REQUIRE(rgb[1] == Approx(0.f));
    REQUIRE(rgb[2] == Approx(84.375f));
}

TEST_CASE("should convert to B s=100 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(225.f, 100.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(2.87791f));
    REQUIRE(rgb[1] == Approx(0.f));
    REQUIRE(rgb[2] == Approx(15.54074f));
}

TEST_CASE("should convert to B s=100 b=0", "[HsbToRGBGeneric]") {
    test->toRgb(225.f, 100.f, 0.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f));
    REQUIRE(rgb[1] == Approx(0.f));
    REQUIRE(rgb[2] == Approx(0.f));
}

///////////////// SATURATION /////////////////
TEST_CASE("should handle saturation to s=0 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(120.f, 0.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(18.41865f));
    REQUIRE(rgb[1] == Approx(18.41865f));
    REQUIRE(rgb[2] == Approx(18.41865f));
}
TEST_CASE("should handle saturation to s=50 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(240.f, 50.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(62.5f));
    REQUIRE(rgb[1] == Approx(50.f));
    REQUIRE(rgb[2] == Approx(87.5f));
}