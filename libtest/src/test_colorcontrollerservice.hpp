#include <catch2/catch.hpp>

#include <colorcontrollerservice.h>



SCENARIO("The color Controller service", "[colors]") {
    GIVEN("a color service gets created") {
        HSB calculatedHsb(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        HSB setHsb(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        bool called = false;
        std::unique_ptr<ColorControllerService> colorControllerService(nullptr);
        colorControllerService.reset(
        new ColorControllerService(HSB(0.0f, 0.0f, 0.0f, 0.0f, 0.0f), [&colorControllerService, &calculatedHsb, &setHsb, &called](const HSB & currentHsb) {
            calculatedHsb = currentHsb;
            setHsb = colorControllerService->hsb();
            called = true;
        }));
        WHEN("we are at time 0") {
            colorControllerService->handle(0);
            THEN("should return given default color at time and have been called") {
                REQUIRE(called == true);
                REQUIRE(calculatedHsb == HSB(0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
                WHEN("we are at time 10 and set HSB") {
                    colorControllerService->hsb(HSB(0.0f, 100.0f, 100.0f, 0.0f, 0.0f));
                    colorControllerService->handle(10);
                    THEN("Should turn on light with fading") {
                        REQUIRE(calculatedHsb == HSB(0.0f, 10.0f, 10.0f, 0.0f, 0.0f));
                        WHEN("we turn off the power") {
                            colorControllerService->power(false);
                            colorControllerService->handle(15); // call twice so filter can do it's work
                            colorControllerService->handle(20);
                            THEN("Should turn off light with fading") {
                                REQUIRE(calculatedHsb == HSB(0.0f, 27.1f, 8.1f, 0.0f, 0.0f));
                            }
                        }
                    }
                }
            }
        }
    }
}