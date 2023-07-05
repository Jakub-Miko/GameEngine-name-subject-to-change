#pragma once
#include <Core/Defines.h>
#include <Core/ModuleInterface.h>
#include <TestLayer.h>

LIBEXP class TestImpl1 : public TestModular {
public:
    RUNTIME_TAG("TestImpl1");
    virtual TestModular* clone() override {
        return new TestImpl1;
    }
    virtual int GetNumber1() override {
        return 1;
    }
    virtual int GetNumber2() override {
        return 2;
    }
    virtual int GetNumber3() override {
        return 3;
    }
};

LIBEXP class TestImpl2 : public TestModular {
public:
    RUNTIME_TAG("TestImpl2");
    virtual TestModular* clone() override {
        return new TestImpl2;
    }
    virtual int GetNumber1() override {
        return 4;
    }
    virtual int GetNumber2() override {
        return 5;
    }
    virtual int GetNumber3() override {
        return 6;
    }
};

