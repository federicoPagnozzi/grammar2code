//
//
//  crace_conf.cpp
//  grammar2code
//
//
//  Created by Federico Pagnozzi on 25/02/2023.
//  Copyright (c) 2023 Federico Pagnozzi. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include "grammar.hpp"

#include <boost/algorithm/string/join.hpp>


#include <string>
#include <vector>

std::string grammar::crace_conf::fmt_rule_name(const std::string& path)
{
    auto param_name = rule_name(path);
    return param_name.first + "\t\"--" + param_name.second + "=\"\t";
}

std::string grammar::crace_conf::fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index)
{
    auto cond = rule_cond(path, node_name, rec_index);
    auto cond_name = rule_name(cond.first);

    if (cond.first.empty() || cond.second.empty()) {
        return "";
    } else {
        return "\t| " + cond_name.first + " == " + cond.second;
    }
}

void grammar::crace_conf::fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond)
{
    std::string type;
    if (rule_type == "int") {
        type = "i";
    } else if (rule_type == "real") {
        type = "r";
    } else if (rule_type == "categorical") {
        type = "c";
    } else if (rule_type == "recursive") {
        type = "c";
    }

    std::string alternatives = boost::join(values, ", ");
    parameters_.push_back(rule_name + " " + type + " (" + alternatives + ")" + rule_cond);
}
