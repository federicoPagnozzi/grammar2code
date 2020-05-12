//
//  walker.cpp
//  grammar2code
//
//  Created by Franco Mascia on 17/03/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include "grammar.hpp"
#include "error.hpp"

#include <boost/algorithm/string/erase.hpp>

grammar::walker::walker(std::shared_ptr<grammar::model>& a_model, int max_depth) : model_{a_model}, max_depth_{max_depth}
{   
}

grammar::walker::~walker()
{
}

bool grammar::walker::has_children(const pugi::xml_node& node) const
{
    return node.children().begin() != node.children().end();
}

bool grammar::walker::has_attributes(const pugi::xml_node& node) const
{
    return node.attributes().begin() != node.attributes().end();
}

grammar::walker::node_type grammar::walker::type(const pugi::xml_node& node)
{
    if (!has_children(node) && !has_attributes(node) && strcmp(node.name(), "or") && node.type() != pugi::node_cdata) {
        return grammar::walker::node_type::call;
    } else if (!has_children(node) && has_attributes(node) && strcmp(node.attribute("type").value(), "")) {
        return grammar::walker::node_type::range;
    } else if (!strcmp(node.name(), "gr:copy")) {
        return grammar::walker::node_type::copy;
    } else if (node.type() == pugi::node_cdata) {
        return grammar::walker::node_type::cdata;
    } else if (has_children(node)) {
        for (auto& child : node.children()) {
            if (!strcmp(child.name(), node.name())) {
                return grammar::walker::node_type::recursive;
            }
        }
    }
    // checking if there are choices
    for (auto& child : node.children()) {
        if (!strcmp(child.name(), "or")) {
            return grammar::walker::node_type::categorical;
        }
    }
    return grammar::walker::node_type::plain;
}

std::vector<std::vector<pugi::xml_node>> grammar::walker::get_choice(pugi::xml_object_range<pugi::xml_node_iterator> children)
{
    int count = 0;
    for (auto& child : children) {
        if (!strcmp(child.name(), "or")) {
            ++count;
        }
    }
    std::vector<std::vector<pugi::xml_node>> results(count + 1);
    count = 0;
    for (auto& child : children) {
        if (!strcmp(child.name(), "or")) {
            ++count;
        } else {
            results[count].push_back(child);
        }
    }
    return results;
}

void grammar::walker::do_walk(const pugi::xml_node& node, std::string parent, int depth)
{
    if (type(node) == grammar::walker::node_type::call) {
        callback_call(node, parent, depth);
        std::string name = node.name();
        std::string path = parent + "%" + name;
        pugi::xpath_query element_q(("/gr:grammar/gr:derivations/" + name).c_str());
        const auto &iter = element_q.evaluate_node_set(model_->grammar());
        if (iter.size() == 0) {
            Error::fatal("No definition for " + name + ".");
        }
        for (auto& element : iter) {
            do_walk(element.node(), path, depth);
        }
    } else if (type(node) == grammar::walker::node_type::categorical) {
        // if node is at the root of a series of derivation (attribute output)
        // and is transformed to a parameter (categorical or recursive), it
        // should have a parameter name
        if (parent.empty()) {
            parent = node.name();
        }
        int callback_choice = callback_categorical(node, parent, depth);
        auto choices = get_choice(node.children());
        int count = 0;
        for (auto& choice : choices) {
            if (callback_choice != -1 && callback_choice != count) {
                ++count;
                continue;
            }
            for (auto& child : choice) {
                std::string path = parent + "%" + std::to_string(count);
                do_walk(child, path, depth);
            }
            ++count;
        }
    } else if (type(node) == grammar::walker::node_type::recursive) {
        // if node is at the root of a series of derivation (attribute output)
        // and is transformed to a parameter (categorical or recursive), it
        // should have a parameter name
        if (parent.empty()) {
            parent = node.name();
        }
        int callback_choice = callback_recursive(node, parent + "@" + std::to_string(depth), depth);
        auto choices = get_choice(node.children());        
        int count = 0;
        for (auto& choice : choices) {
            if (callback_choice != -1 && callback_choice != count) {
                ++count;
                continue;
            }
            // check if this is the recursive rule
            bool recursive = false;
            for (auto& child : choice){
                if (!strcmp(child.name(), node.name())) {
                    recursive = true;
                }
            }
            if (recursive) {
                if (depth + 1 < max_depth_) {
                    for (auto& child : choice) {
                        if (!strcmp(child.name(), node.name())) {
                            std::string path = parent;
                            std::string name = node.name();
                            boost::erase_last(path, "%" + name);
                            do_walk(child, path, depth + 1);
                        } else {
                            std::string path = parent + "@" + std::to_string(depth) + "%" + std::to_string(count);
                            do_walk(child, path, depth + 1);
                        }
                    } 
                }
            } else {
                for (auto& child : choice){
                    std::string path = parent + "@" + std::to_string(depth) + "%" + std::to_string(count);
                    do_walk(child, path, depth + 1);
                }
            }
            ++count;
        }
    } else if (type(node) == grammar::walker::node_type::range) {
        callback_range(node, parent, depth);
    } else if (type(node) == grammar::walker::node_type::copy) {
        callback_copy(node, parent, depth);
    } else if (type(node) == grammar::walker::node_type::cdata) {
        callback_cdata(node, parent, depth);
    } else if (type(node) == grammar::walker::node_type::plain) {
        callback_plain(node, parent, depth);
        for (auto&child : node.children()) {
            if (strcmp(child.name(), "or")) {
                std::string path;
                if (!parent.empty()) {
                    path = parent + "%";
                } else {
                    // node is at the root of a series of derivation
                    // (has the output attribute)
                    path += node.name();
                }
                do_walk(child, path, depth);
            }
        }
    }
}

void grammar::walker::walk()
{
    pugi::xpath_query all_elements("/gr:grammar/gr:derivations/*[@output]");
    for (auto& element : all_elements.evaluate_node_set(model_->grammar())) {
        do_walk(element.node(), "", 0);
    }
}
