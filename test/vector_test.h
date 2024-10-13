#pragma once

#include <algorithm>
#include <iostream>
#include <logger.h>
#include <model.h>
#include <vector>

#define NUM_ELEMENTS 10000000

using namespace common;

class VectorTest 
{
  public:
    static void Run()
    {
        Model *arr1 = new Model[NUM_ELEMENTS];
        std::vector<Model *> vec1;
        std::vector<std::shared_ptr<Model>> vec2;

        // write
        {
            ScopeStopWatch sw("1) write array");
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                auto x = new Model();
                arr1[i] = *x;
            }
        }

        {
            ScopeStopWatch sw("1) write vector");
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                auto x = new Model();
                vec1.push_back(x);
            }
        }

        {
            ScopeStopWatch sw("1) write vector(shared_ptr)");
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                auto x = std::make_shared<Model>();
                vec2.push_back(x);
            }
        }

        // read
        {
            ScopeStopWatch sw("2) read array");
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

        {
            ScopeStopWatch sw("2) read vector(shared_ptr)");
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                auto x = vec2[i].get();
            }
        }

        // move
        {
            Model *dst_arr;
            ScopeStopWatch sw("3) move array");
            dst_arr = new Model[NUM_ELEMENTS];
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                dst_arr[i] = arr1[i];
            }
        }

        {
            std::vector<Model *> dst_vec;
            ScopeStopWatch sw("3) move vector");
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                dst_vec.push_back(vec1[i]);
            }
        }

        {
            std::vector<Model *> dst_vec2;
            ScopeStopWatch sw("3) move vector(shared_ptr)");
            for (int i = 0; i < NUM_ELEMENTS; i++) {
                dst_vec2.push_back(vec2[i].get());
            }
        }

    };
};