#include <algorithm>
#include <iostream>
#include <logger.h>
#include <model.h>
#include <vector>

#define NUM_ELEMENTS 1000000

using namespace common;

int main() {
    auto arr1 = new Model[NUM_ELEMENTS];
    std::vector<Model> vec1;
    vec1.resize(NUM_ELEMENTS);

    {
        ScopeStopWatch sw("1) write array ptr");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            arr1[i] = Model();
        }
    }

    {
        ScopeStopWatch sw("1) write vector");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            vec1[i] = Model();
        }
    }

    {
        ScopeStopWatch sw("2) read array ptr");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            auto x = arr1[i];
        }
    }

    {
        ScopeStopWatch sw("2) read vector");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            auto x = vec1[i];
        }
    }

    std::vector<Model> vec2;
    Model *arr2;
    Model *arr3;

    {
        ScopeStopWatch sw("3) push_back");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            vec2.push_back(arr1[i]);
        }
    }

    {
        ScopeStopWatch sw("3) new_delete");
        int count = 0;
        // count
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            count++;
        }
        // write
        arr2 = new Model[count];
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            arr2[i] = arr1[i];
        }
    }

    {
        ScopeStopWatch sw("3) move");
        int count = 0;
        // count
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            count++;
        }
        // write
        arr3 = new Model[count];
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            arr3[i] = std::move(arr1[i]);
        }
    }

    return 0;
}
