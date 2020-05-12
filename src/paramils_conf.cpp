//
//  paramils_conf.cpp
//  grammar2code
//
//  Created by Franco Mascia on 26/07/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include "grammar.hpp"
#include "error.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <string>
#include <vector>

std::string grammar::paramils_conf::fmt_rule_name(const std::string& path)
{
    auto param_name = rule_name(path);
    return param_name.second;
}

std::string grammar::paramils_conf::fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index)
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

void grammar::paramils_conf::fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond)
{
    std::string alternatives = boost::join(values, ", ");
    parameters_.push_back(rule_name + " {" + alternatives + "}[" + default_value + "]");
    conditionals_.push_back(rule_cond);
}

void grammar::paramils_conf::callback_range(const pugi::xml_node& node, std::string path, int depth)
{
    std::string rule = fmt_rule_name(path);
    std::string cond = fmt_rule_cond(path, node.name());
    std::vector<std::string> choices;
    std::string type = node.attribute("type").value();
    // default value
    std::string default_value;
    if (strcmp(node.attribute("default").value(), "")) {
        default_value = node.attribute("default").value();
    } else {
        default_value = node.attribute("min").value();
    }
    // log scale sampling
    std::string attribute = node.attribute("log-scale").value();
    bool log_scale = false;
    if (boost::iequals(attribute, "true") || boost::iequals(attribute, "yes")) {
        log_scale = true;
    }
    if (type == "int") {
        for (int i = std::stoi(node.attribute("min").value());
             i <= std::stoi(node.attribute("max").value());
             i += std::stoi(node.attribute("stepIfEnumerated").value())) {
            choices.push_back(std::to_string(i));
        }
        fmt_parameter(rule, type, choices, default_value, log_scale, cond);
    }
    if (type == "real") {
        for (float i = std::stof(node.attribute("min").value());
             i <= std::stof(node.attribute("max").value());
             i += std::stof(node.attribute("stepIfEnumerated").value())) {
            choices.push_back(std::to_string(i));
        }
        fmt_parameter(rule, type, choices, default_value, log_scale, cond);
    }
}

void grammar::paramils_conf::print(std::ostream& stream)
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
