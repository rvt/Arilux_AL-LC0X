#include <catch2/catch.hpp>
#include <cmdhandler.h>
#include <memory>
#include <hsb.h>
#include <propertyutils.h>

#define BASE_TOPIC "baseTopic"

std::ostream& operator << (std::ostream& os, HSB const& value) {
    os << "HSB(" << value.hue() << "," << value.saturation() << "," << value.brightness() << "," << value.white1() << "," << value.white2() << ")" ;
    return os;
}

TEST_CASE("HSB can be parsed from Strings with a given HSB", "[CmdHandler]") {
    HSB givenHsb = HSB(1, 2, 3, 4, 5);
    SECTION("when empty string is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "", nullptr);
        REQUIRE(result == givenHsb);
    }
    SECTION("when only hue is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "h=10.0", nullptr);
        REQUIRE(result == HSB(10, 2, 3, 4, 5));
    }
    SECTION("when only saturation is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "s=10.0", nullptr);
        REQUIRE(result == HSB(1, 10, 3, 4, 5));
    }
    SECTION("when only brightness is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "b=10.0", nullptr);
        REQUIRE(result == HSB(1, 2, 10.0, 4, 5));
    }
    SECTION("when only w1 is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "w1=10.0", nullptr);
        REQUIRE(result == HSB(1, 2, 3, 10, 5));
    }
    SECTION("when only w2 is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "w2=10.0", nullptr);
        REQUIRE(result == HSB(1, 2, 3, 4, 10.0));
    }
    SECTION("when only hsb is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "hsb=10,11,12,13,14", nullptr);
        REQUIRE(result == HSB(10, 11, 12, 13, 14));
    }
    SECTION("when only hsb and w2 is given") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "hsb=10,11,12 w2=13", nullptr);
        REQUIRE(result == HSB(10, 11, 12, 4, 13));
    }
    SECTION("when only hsb and w2 is given with some extra spaces") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "  hsb = 10,11,12   w2 = 13  ", nullptr);
        REQUIRE(result == HSB(10, 11, 12, 4, 13));
    }
    SECTION("when just hsb values are given without key") {
        HSB result = CmdHandler::hsbFromString(givenHsb, "12,23,34", nullptr);
        REQUIRE(result == HSB(12, 23, 34, 4, 5));
    }
}



TEST_CASE("Commands can be parser", "[CmdHandler]") {
    HSB testHsb(0, 0, 0, 0, 0);
    bool restarted = false;
    bool power = false;
    bool bChanged = false;
    uint32_t baseAddress = 0;
    Effect* effect = nullptr;
    Filter* filter = nullptr;
    CmdHandler cmdHandler(
        (int32_t)strlen(BASE_TOPIC),
    [&power, &bChanged](bool p) {
        power = p;
    },
    [&testHsb](const HSB & hsb) {
        testHsb = hsb;
    },
    [&filter](const std::unique_ptr<Filter> f) {
        filter = f.get();
    },
    [&effect](const std::unique_ptr<Effect> e) {
        effect = e.get();
    },
    [&baseAddress](const uint32_t base) {
        baseAddress = base;
    },
    [&restarted]() {
        restarted = true;
    }
    );
    SECTION("should set color") {
        REQUIRE(power == false);
        cmdHandler.handle(BASE_TOPIC "/color", "hsb=80,81,82 ON", HSB(1, 2, 3, 4, 5), HSB(11, 12, 13, 14, 15), 0);
        REQUIRE(power == true);
        REQUIRE(testHsb == HSB(80, 81, 82, 4, 5));
    }
    SECTION("should not set bChanged") {
        REQUIRE(power == false);
        cmdHandler.handle(BASE_TOPIC "/color", "ON", HSB(1, 2, 0, 4, 5), HSB(11, 12, 13, 14, 15), 0);
        REQUIRE(power == true);
        REQUIRE(testHsb == HSB(1, 2, 0, 4, 5));
    }
    SECTION("should set power") {
        REQUIRE(power == false);
        cmdHandler.handle(BASE_TOPIC "/color", "ON", HSB(1, 2, 3, 4, 5), HSB(11, 12, 13, 14, 15), 0);
        REQUIRE(power == true);
        cmdHandler.handle(BASE_TOPIC "/color", "OFF", HSB(1, 2, 3, 4, 5), HSB(11, 12, 13, 14, 15), 0);
        REQUIRE(power == false);
    }
    SECTION("should beable to restart device") {
        cmdHandler.handle(BASE_TOPIC "/restart", "1", HSB(1, 2, 3, 4, 5), HSB(11, 12, 13, 14, 15), 0);
        REQUIRE(restarted == true);
    }
    SECTION("should beable to set filter") {
        cmdHandler.handle(BASE_TOPIC "/filter", "name=fading", HSB(1, 2, 3, 4, 5), HSB(11, 12, 13, 14, 15), 0);
        REQUIRE(filter != nullptr);
    }
    SECTION("should beable to set effect") {
        cmdHandler.handle(BASE_TOPIC "/effect", "name=rainbow duration=300", HSB(1, 2, 3, 4, 5), HSB(11, 12, 13, 14, 15), 0);
        REQUIRE(effect != nullptr);
    }
}

