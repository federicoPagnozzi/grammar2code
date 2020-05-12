//
//  error.cpp
//  Grammar2Code
//
//  Created by Franco Mascia on 02/04/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include "error.hpp"

#include <iostream>
#include <cstdlib>

std::string Error::executable_name = "";

void Error::set_exec_name(const std::string& name)
{
    executable_name = name;
}

void Error::fatal(std::string message)
{
    std::cerr << std::endl << "Error";
    if (!executable_name.empty()) {
        std::cerr << " (" << executable_name << ")";
    }
    std::cerr << ": " << message << std::endl;
    exit(EXIT_FAILURE);
}

void Error::warning(std::string message)
{
    std::cerr << std::endl << "Warning";
    if (!executable_name.empty()) {
        std::cerr << " (" << executable_name << ")";
    }
    std::cerr << ": " << message << std::endl;
}
