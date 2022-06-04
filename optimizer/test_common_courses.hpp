#pragma once
#include "schedule.hpp"

LatLong loc_mlb{-83.739053, 42.27986781375627};
LatLong loc_mh{-83.73927753299714, 42.277090888546354};

ClassSection eecs183_001{{{{8, 30}, {10, 0}, 0b0101000, loc_mlb, eecs183_001}},
                         {"bentorra"},
                         "LEC",
                         10335,
                         1,
                         4};
ClassSection math116_023{{{{11, 30}, {13, 0}, 0b0100100, loc_mh, math116_023},
                          {{11, 30}, {13, 0}, 0b0010000, loc_mh, math116_023}},
                         {},
                         "LEC",
                         11447,
                         23,
                         4};
ClassSection stats250_204{{{{13, 0}, {16, 0}, 0b0100000, loc_mh, stats250_204}},
                          {"alromero"},
                          "LAB",
                          14113,
                          204,
                          4};
ClassSection ala223_001{{{{11, 0}, {12, 0}, 0b1010000, loc_mh, ala223_001}},
                        {"ebfretz"}, "LEC", 24619, 1, 3};