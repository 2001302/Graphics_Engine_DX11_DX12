#pragma once

#include <query.h>
#include <logger.h>

#define NUM_ELEMENTS 10000000

using namespace common;

class Dummy {
  public:
    bool IsEnabled() { return enabled; }
    void SetEnabled(bool enabled) { this->enabled = enabled; }

  private:
    bool enabled = true;
};

class LinqTest {
  public:
    static void Run() {
        auto vec = std::make_shared<std::vector<Dummy *>>();

        // init
        {
            ScopeStopWatch sw("init");
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                auto x = new Dummy();
                if (i % 2 == 0) {
                    x->SetEnabled(false);
                } else {
                    x->SetEnabled(true);
                }
                vec->push_back(x);
            }
        }

        // where
        {
            ScopeStopWatch sw("where");
            auto result =
                query::From(vec).Where([](Dummy *x) { return x->IsEnabled(); });
        }

        {
            ScopeStopWatch sw("where");
            auto result =
                query::From(vec).Where([](Dummy * x) -> FUNC(x->IsEnabled()));
        }
    }
};