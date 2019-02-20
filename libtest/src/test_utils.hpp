#include <catch2/catch.hpp>

#include <propertyutils.h>


TEST_CASE("Should hold propertyvalue", "[propertyvalue]") {
    Properties props;
    using PV = PropertyValue;
    props.put("fooLong", PV(1));
    REQUIRE(props.get("fooLong").getLong() == 1);
    props.put("fooFloat", PV(12.0f));
    REQUIRE(props.get("fooFloat").getFloat() == 12.0f);
    props.put("fooBoo1", PV(true));
    REQUIRE(props.get("fooBoo1").getBool() ==  true);
    props.put("fooBool2", PV(false));
    REQUIRE(props.get("fooBool2").getBool() ==  false);
    props.put("fooChar", PV("hello"));
    REQUIRE(strcmp(props.get("fooChar").getCharPtr(), "hello") == 0);
}

TEST_CASE("Should convert long values to other types", "[propertyvalue]") {
    Properties props;
    using PV = PropertyValue;
    props.put("longValue", PV(123));
    REQUIRE(props.get("longValue").asBool() == true);
    REQUIRE(props.get("longValue").asFloat() == Approx(123.0));
    REQUIRE(props.get("longValue").asLong() == 123);
    props.put("longValue0", PV(0));
    REQUIRE(props.get("longValue0").asBool() == false);
}

TEST_CASE("Should convert float values to other types", "[propertyvalue]") {
    Properties props;
    using PV = PropertyValue;
    props.put("floatValue", PV(123.23f));
    REQUIRE(props.get("floatValue").asBool() == true);
    REQUIRE(props.get("floatValue").asFloat() == Approx(123.23f));
    REQUIRE(props.get("floatValue").asLong() == 123);
    props.put("floatValue1", PV(865.67f));
    REQUIRE(props.get("floatValue1").asLong() == 866);
}

TEST_CASE("Should convert bool values to other types", "[propertyvalue]") {
    Properties props;
    using PV = PropertyValue;
    props.put("boolValue", PV(true));
    REQUIRE(props.get("boolValue").asBool() == true);
    REQUIRE(props.get("boolValue").asFloat() == Approx(1.0f));
    REQUIRE(props.get("boolValue").asLong() == 1);
    props.put("boolValue1", PV(false));
    REQUIRE(props.get("boolValue1").asBool() == false);
    REQUIRE(props.get("boolValue1").asFloat() == Approx(0.0f));
    REQUIRE(props.get("boolValue1").asLong() == 0);
}

TEST_CASE("Should convert char values to other types", "[propertyvalue]") {
    Properties props;
    using PV = PropertyValue;
    props.put("charValue", PV("-1234.56"));
    REQUIRE(props.get("charValue").asBool() == false);
    REQUIRE(props.get("charValue").asFloat() == Approx(-1234.56f));
    REQUIRE(props.get("charValue").asLong() == -1235);
    props.put("charValue1", PV("1"));
    REQUIRE(props.get("charValue1").asBool() == true);
    props.put("charValue0", PV("0"));
    REQUIRE(props.get("charValue0").asBool() == false);
}