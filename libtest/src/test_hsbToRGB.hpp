#include <catch2/catch.hpp>
#include <stdint.h>
#include <iostream>
#include "arduinostubs.hpp"
#include <hsbToRGB.h>


float rgb[] = {-1.f, -1.f, -1.f};
auto margin = 5.f;

HSBToRGB* test = HsbToRGBGeneric::genericLedStrip();
///////////////// YELLOW /////////////////
TEST_CASE("should convert to YELLOW s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(60.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(81.f).margin(margin));
    REQUIRE(rgb[1] == Approx(30.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

///////////////// CYAN /////////////////
TEST_CASE("should convert to CYAN s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(180.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin)); // 85
    REQUIRE(rgb[1] == Approx(75.f).margin(margin));   // 33
    REQUIRE(rgb[2] == Approx(25.f).margin(margin));
}

///////////////// RED /////////////////
TEST_CASE("should convert to R s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(0.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(100.f).margin(margin));
    REQUIRE(rgb[1] == Approx(0.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

TEST_CASE("should convert to R s=100 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(0.f, 100.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(18.f).margin(margin));
    REQUIRE(rgb[1] == Approx(0.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

TEST_CASE("should convert to R s100=0 b=0", "[HsbToRGBGeneric]") {
    test->toRgb(0.f, 100.f, 0.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin));
    REQUIRE(rgb[1] == Approx(0.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

///////////////// GREEN /////////////////
TEST_CASE("should convert to G s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(135.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin));
    REQUIRE(rgb[1] == Approx(100.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

TEST_CASE("should convert to G s=100 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(135.f, 100.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin));
    REQUIRE(rgb[1] == Approx(16.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

TEST_CASE("should convert to G s=100 b=0", "[HsbToRGBGeneric]") {
    test->toRgb(135.f, 100.f, 0.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin));
    REQUIRE(rgb[1] == Approx(0.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

///////////////// BLUE /////////////////
TEST_CASE("should convert to B s=100 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(225.f, 100.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin));
    REQUIRE(rgb[1] == Approx(0.f).margin(margin));
    REQUIRE(rgb[2] == Approx(100.f).margin(margin));
}

TEST_CASE("should convert to B s=100 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(225.f, 100.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin));
    REQUIRE(rgb[1] == Approx(0.f).margin(margin));
    REQUIRE(rgb[2] == Approx(15.f).margin(margin));
}

TEST_CASE("should convert to B s=100 b=0", "[HsbToRGBGeneric]") {
    test->toRgb(225.f, 100.f, 0.f, rgb);
    REQUIRE(rgb[0] == Approx(0.f).margin(margin));
    REQUIRE(rgb[1] == Approx(0.f).margin(margin));
    REQUIRE(rgb[2] == Approx(0.f).margin(margin));
}

///////////////// SATURATION /////////////////
TEST_CASE("should handle saturation to s=0 b=50", "[HsbToRGBGeneric]") {
    test->toRgb(120.f, 0.f, 50.f, rgb);
    REQUIRE(rgb[0] == Approx(18.f).margin(margin));
    REQUIRE(rgb[1] == Approx(18.f).margin(margin));
    REQUIRE(rgb[2] == Approx(18.f).margin(margin));
}
TEST_CASE("should handle saturation to s=50 b=100", "[HsbToRGBGeneric]") {
    test->toRgb(240.f, 50.f, 100.f, rgb);
    REQUIRE(rgb[0] == Approx(54.f).margin(margin));
    REQUIRE(rgb[1] == Approx(50.f).margin(margin));
    REQUIRE(rgb[2] == Approx(95.5f).margin(margin));
}