#include <basiceffects.h>
#include <catch2/catch.hpp>
#include <hsb.h>

SCENARIO("TransitionEffect will change based on time", "[rainbow]") {
    GIVEN("A rainbow filter at 300 seconds") {
        HSB hsb(100.f, 100.f, 100.f, 100.f, 100.f);
        TransitionEffect transition(hsb, 10000, 10000);
        WHEN("At start when all the lights are aff") {
            THEN("colors should be still off at time 0") {
                HSB hsb = transition.handleEffect(0, 10000, HSB(0.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(0.0f));
                REQUIRE(hsb.saturation() == Approx(0.0f));
                REQUIRE(hsb.brightness() == Approx(0.0f));
                REQUIRE(hsb.white1() == Approx(0.0f));
                REQUIRE(hsb.white2() == Approx(0.0f));
            }
            THEN("colors should go on at time 1") {
                HSB hsb = transition.handleEffect(0, 10001, HSB(0.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(0.01f));
                REQUIRE(hsb.saturation() == Approx(0.01f));
                REQUIRE(hsb.brightness() == Approx(0.01f));
                REQUIRE(hsb.white1() == Approx(0.01f));
                REQUIRE(hsb.white2() == Approx(0.01f));
            }
            THEN("colors should go on at time 15000") {
                HSB hsb = transition.handleEffect(0, 15000, HSB(0.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(50.0f));
                REQUIRE(hsb.saturation() == Approx(50.0f));
                REQUIRE(hsb.brightness() == Approx(50.0f));
                REQUIRE(hsb.white1() == Approx(50.0f));
                REQUIRE(hsb.white2() == Approx(50.0f));
            }
            THEN("colors should go on at time 1") {
                HSB hsb = transition.handleEffect(0, 19999, HSB(0.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                // REQUIRE(hsb.hue() == Approx(99.99f));
                REQUIRE(hsb.saturation() == Approx(99.99f));
                REQUIRE(hsb.brightness() == Approx(99.99f));
                REQUIRE(hsb.white1() == Approx(99.99f));
                REQUIRE(hsb.white2() == Approx(99.99f));
            }
        }
    }

}