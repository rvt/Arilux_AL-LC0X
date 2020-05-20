#pragma once

#include <WiFiManager.h>

class IntParameter : public WiFiManagerParameter {
public:
    IntParameter(const char *id, const char *placeholder, const char *custom, int labelPlacement)
        : WiFiManagerParameter("") {
        init(id, placeholder, "", 1, custom, labelPlacement);
    }

    long getValue() {
        return atol(WiFiManagerParameter::getValue());
    }

    void setValue(long value) {
        char buffer[10] = "";
        snprintf(buffer, sizeof(buffer), "%ld", value);
        WiFiManagerParameter::setValue(buffer, 10);
    }
};

class BoolParameter : public WiFiManagerParameter {
public:
    BoolParameter(const char *id, const char *placeholder, const char *custom, int labelPlacement)
        : WiFiManagerParameter("") {
        init(id, placeholder, "", 1, custom, labelPlacement);
    }

    bool getValue() {
        return strcmp(WiFiManagerParameter::getValue(), "1")==0;
    }
    void setValue(bool val) {
        WiFiManagerParameter::setValue(val?"1":"", 2);
    }
};
