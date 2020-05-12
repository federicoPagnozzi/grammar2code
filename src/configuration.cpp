//
//  configuration.cpp
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

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <string>
#include <vector>

std::pair<std::string, std::string> grammar::configuration::rule_name(const std::string& path)
{
    std::string command_name = path;
    boost::erase_all(command_name, "@");
    boost::erase_all(command_name, "%");
    boost::erase_all(command_name, ":");
    
    std::string command_line = path;
    boost::replace_all(command_line, ":", "-");
    
    return std::make_pair(command_name, command_line);
}

std::pair<std::string, std::string> grammar::configuration::rule_cond(const std::string& path, const std::string& node_name, int rec_index)
{
    std::string condition = path;
    std::string value;
    
    // recursive rule at depth0 or non recursive rule
    bool standard_rule = true;
    
    // first check if this was a recursive rule and which depth
    regex_ns::smatch m;
    if (regex_ns::regex_search(condition, m, regex_ns::regex("@[0-9]+$"))) {
        std::string x = *(m.begin());
        boost::erase_last(condition, x);
        boost::erase_first(x, "@");
        int depth = std::stoi(x);
        if (depth > 0) {
            standard_rule = false;
            condition = condition + "@" + std::to_string(depth - 1);
            value = std::to_string(rec_index);
        }
    }
    
    // if the rule is non recursive or recursive at level 0 we continue
    // stripping the last part to set the right condition
    if (standard_rule) {
        // remove node name and see what remains of path
        boost::erase_last(condition, "%" + node_name);
        if (regex_ns::regex_search(condition, m, regex_ns::regex("%[0-9]+$"))) {
            // just one match
            std::string x = *(m.begin());
            boost::erase_last(condition, x);
            boost::erase_first(x, "%");
            value = x;
        } else {
            condition.clear();
        }
    }
    boost::trim(condition);
    boost::trim(value);
    return std::make_pair(condition, value);
}

void grammar::configuration::callback_call(const pugi::xml_node& node, std::string path, int depth)
{
}

int grammar::configuration::callback_categorical(const pugi::xml_node& node, std::string path, int depth)
{
    std::string rule = fmt_rule_name(path);
    stop_if_duplicate_parameters(rule);
    std::string cond = fmt_rule_cond(path, node.name());
    std::vector<std::string> choices;
    int count = 0;
    for (auto& child : node.children()) {
        if (!strcmp(child.name(), "or")){
            ++count;
        }
    }
    for (int i = 0; i < count + 1; i++) {
        choices.push_back(std::to_string(i));
    }
    fmt_parameter(rule, "categorical", choices, "", false, cond);
    
    return -1;
}

int grammar::configuration::callback_recursive(const pugi::xml_node& node, std::string path, int depth)
{
    std::string rule = fmt_rule_name(path);
    stop_if_duplicate_parameters(rule);
    std::vector<std::string> choices;
    auto enum_choices = get_choice(node.children());
    int count = 0;
    int rec_value = -1;
    for (auto& choice : enum_choices) {
        // check if this is the recursive rule
        bool recursive = false;
        for (auto& child : choice){
            if (!strcmp(child.name(), node.name())) {
                recursive = true;
            }
        }
        if (recursive) {
            if (depth + 1 < max_depth_) {
                choices.push_back(std::to_string(count));
            }
            rec_value = count;
        }else{
            choices.push_back(std::to_string(count));
        }
        ++count;
    }
    std::string cond = fmt_rule_cond(path, node.name(), rec_value);
    fmt_parameter(rule, "categorical", choices, "", false, cond);
    
    return -1;
}

void grammar::configuration::callback_range(const pugi::xml_node& node, std::string path, int depth)
{
    std::string rule = fmt_rule_name(path);
    stop_if_duplicate_parameters(rule);
    std::string cond = fmt_rule_cond(path, node.name());
    std::vector<std::string> choices;
    std::string type = node.attribute("type").value();
    if (type != "int" && type != "real") {
        return;
    }
    choices.push_back(node.attribute("min").value());
    choices.push_back(node.attribute("max").value());
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

    fmt_parameter(rule, type, choices, default_value, log_scale, cond);
}

void grammar::configuration::callback_copy(const pugi::xml_node& node, std::string path, int depth)
{
}

void grammar::configuration::callback_cdata(const pugi::xml_node& node, std::string path, int depth)
{
}

void grammar::configuration::callback_plain(const pugi::xml_node& node, std::string path, int depth)
{
}

void grammar::configuration::print(std::ostream& stream)
{
    parameters_.clear();
    parameter_names_.clear();
    walk();
    for (auto& parameter : parameters_) {
        stream << parameter << std::endl;
    }
}

void grammar::configuration::stop_if_duplicate_parameters(std::string parameter) {
    // there can be rare cases in which parameters have the same name, in this
    // case we stop and return a message to the user to solve the name clash
    // manually
    //
    // for example A221 could be choice 1 at the second level of recursion of
    // parameter A2 or choice 1 of parameter A22 (this could happen also because
    // of the duplication of rule calls at the end of simplification rules
    if (std::find(parameter_names_.begin(), parameter_names_.end(), parameter) != parameter_names_.end()) {
        for (auto& parameter : parameters_) {
            std::cerr << parameter << std::endl;
        }
        Error::fatal(parameter);
    } else {
        parameter_names_.push_back(parameter);
    }
}