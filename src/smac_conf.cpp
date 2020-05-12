//
//  smac_conf.cpp
//  grammar2code
//
//  Created by Franco Mascia on 19/03/14.
//  Copyright (c) 2014 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include "grammar.hpp"
#include "error.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <string>
#include <vector>

std::string grammar::smac_conf::fmt_rule_name(const std::string& path)
{
    auto param_name = rule_name(path);
    return param_name.second;
}

std::string grammar::smac_conf::fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index)
{
    auto param_name = rule_name(path);
    auto cond = rule_cond(path, node_name, rec_index);
    auto cond_name = rule_name(cond.first);
    
    if (cond.first.empty() || cond.second.empty()) {
        return "";
    } else {
        return param_name.second + " | " + cond_name.second + " in {" + cond.second + "}";
    }
}

void grammar::smac_conf::fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond)
{
    if (rule_type == "int" || rule_type == "real") {
        std::string l = (log_scale)? "l" : "";
        std::string i = (rule_type == "int") ? "i" : "";
        parameters_.push_back(rule_name + " [" + values[0] + ", " + values[1] + "][" + default_value + "]" + i + l);
    } else { // rule_type == "categorical" || rule_type == "recursive"
        std::string alternatives = boost::join(values, ", ");
        parameters_.push_back(rule_name + " {" + alternatives + "}[" + values[0] + "]");
        conditionals_.push_back(rule_cond);
    }
}

void grammar::smac_conf::print(std::ostream& stream)
{
    conditionals_.clear();
    
    grammar::configuration::print(stream);

    stream << "\nConditionals:" << std::endl;
    for (auto& conditional : conditionals_) {
        if (!conditional.empty()) {
            stream << conditional << std::endl;
        }
    }
}