
/*
 * File:   globals.hpp
 * Author: jcavalie
 *
 * Created on November 20, 2016, 6:51 PM
 */

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <cstdint>

#define DO_DEBUG true


static const char* Hosts[] = { "192.168.1.2", "192.168.1.2", "192.168.1.2", "192.168.1.2" };
static const char* Ports[] = { "3433", "3366", "4444", "3254" };


constexpr int NODECOUNT = 2;

constexpr int NUMFLAVORS = 4;

enum class Action : uint32_t
{
    CONSUME = 1, PRODUCE = 2
};

enum class subAction : uint32_t
{
    FLAVOR_1, FLAVOR_2, FLAVOR_3, FLAVOR_4
};




#endif /* GLOBALS_HPP */

