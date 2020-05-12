//
//  model.cpp
//  Grammar2Code
//
//  Created by Franco Mascia on 16/03/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include "grammar.hpp"
#include "error.hpp"

#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <vector>

grammar::model::model(boost::filesystem::path xml_file,
                      boost::filesystem::path overwrite_xml_file)
{
    parse_and_merge_grammars(xml_file);
    overwrite_derivations(overwrite_xml_file);
    
    // merging rules from diffrent grammars
    clean_up_append_disjuncitons();
    
    // there can be empty cdata around
    clean_up_remove_empty_cdatas();
    
    // there can be temporary empty derivations or that had empty CDATAs
    clean_up_remove_empty_derivations();
    
    // after removing empty derivations there could be adjacent ORs or rules
    // that begin or end with an OR
    clean_up_remove_useless_ors();
    
    // preprocessing the simplest case here in which a rule like A ::= B | BA
    // can be simplified to BA (where A is used) and A ::= [] | BA. Doing this
    // will reduce the number of actual parameters generated.
    // Complex cases like A ::= CB | DBA are not simplified.
    //
    // Typical example is:                          That expands to:
    //
    // <ps:start output="...">                           _
    //   <ps:A/>                                        ⇙ ⇘
    //   <![CDATA[ something ... ]]>                   B   B+
    //   <ps:C/>                                          ⇙ ⇘
    // </ps:start>                                       B   B+
    // <ps:A>                                               ⇙
    //   <ps:B/>                                           B
    //   <or/>
    //   <ps:B/>
    //   <ps:A/>
    // </ps:A>
    // <ps:B>
    //   ...
    // </ps:B>
    //
    // The issue is that B is both on the left and on the right of the <or/> and
    // this is not automatically detected. So in the derivation tree above the
    // B+ (B and continue) is actually a duplicate of the same B on the left
    // branch, just for the recursive case.
    // If the type has 10 parameters, grammar2code will generate around 50
    // parameters, i.e., 10 for all nodes in the tree.
    //
    // A simplified grammar like this one:          Will expands to:
    //
    // <ps:start output="...">                           B
    //   <ps:B/>                                        ⇙ ⇘
    //   <ps:A/>                                       []  B
    //   <![CDATA[ something ... ]]>                      ⇙ ⇘
    //   <ps:C/>                                         []  B
    // </ps:start>
    // <ps:A>
    //   <![CDATA[ ]]>
    //   <or/>
    //   <ps:B/>
    //   <ps:A/>
    // </ps:A>
    // <ps:B>
    // ...
    // </ps:B>
    //
    // and this leads to only 30 parameters. The code is equivalent, but the
    // number of parameters much smaller.
    //
    // So whenever A ::= B | BA (or A ::= B | AB, or A ::= AB | B, etc.) is
    // detected it is replaced by A :== [] | BA and wherever A is used in the
    // grammar it becomes a BA.
    //
    // The function works whetever B has no children or is a CDATA (and the
    // CDATA on the left and right of the OR are identical).
    //
    // This preprocessing step should be done before clean_up_remove_non_choices
    // that could make the original recursive rule more complex and cannot be
    // detected anymore
    clean_up_simplify_recursions();

    // NOTE in the first implementation there was also a function for
    //      concatenating rules, don't remember for which case it was useful
    //      since we already remove non choices
    // some rules represent no choice, their content is copied where used
    clean_up_remove_non_choices();

    // rules like A ::= B | C | D where C ::= E | F, can be merged together to
    // reduce the number of parameters generated, i.e., A :: = B | E | F | D
    // TODO: this can be furhter extended to the cases where A ::= B | zC | D
    //       where one has to pay attention and A :: = B | zE | zF | D prepend
    //       the terminal suffix z to all alternatives, this could get quickly
    //       more complicate to cases where z is actually a Z and or a sequence
    //       of terminal and non terminal as prefix and suffix
    clean_up_merge_disjuncitons();
    
    // some rules can be duplicates
    // (this was more useful in the original python code where the code was
    //  duplicated for the group IDs)
    clean_up_remove_duplicates();
    
    // rules can be defined and never used
    // (also this was more useful in the original python code where the code was
    //  automatically duplicated for the group IDs)
    clean_up_remove_non_used_rules();
    
    // just for polishing adjacent CDATAs are merged together
    clean_up_merge_cdatas();
    
    // before actually creating some (legitimate) duplicates check if there are
    // already in the simplified grammar
    warn_for_duplicate_derivations();
    
    // once the grammmar is simplified, we possibly rename some rule calls that
    // have not been simplified; in fact, each time a rule is called it is
    // actually a different instantiation of what is described by the rule
    // itself;
    //
    // when generating the parameters, the translation of such parameter could
    // be done in such a way to have different parameter names for different
    // instantiation of the same rule, but we do it here to keep the code simple
    // (especially in dealing with the paths of the conditional expressions of
    // the parameters)
    //
    // for example the rule A ::= B C B will be translated to A ::= B C B2
    //
    // note that this is not necessary for rules with disjunctions such as:
    // A :: = BC | B where this is actually a choice and the content of B will
    // be distinguished in the subsequent parameters form the path in the
    // parameter name that will contain the choice made in A
    rename_calls();
}

pugi::xml_document& grammar::model::grammar()
{
    return grammar_;
}

boost::filesystem::path grammar::model::grammar_path()
{
    return base_path_;
}

void grammar::model::load_grammar(boost::filesystem::path filename, pugi::xml_document& document)
{
    pugi::xml_parse_result result = document.load_file(filename.c_str());
    if (!result) {
        if (result.offset != 0) {
            // finding line number where the mistake occourred
            std::ifstream inFile(filename.string());
            if (!inFile.good()) {
                Error::fatal("Unable to open " + filename.string() + ".");
            }
            std::string line;
            int count = 0;
            int offset = 0;
            while (getline(inFile, line)) {
                ++count;
                offset += line.length() + 1;
                if (offset >= result.offset) {
                    std::cerr << filename << ":" << count << std::endl;
                    std::cerr << "\t\"" << line << "\"" << std::endl;
                    std::cerr << "\t" << result.description() << "." << std::endl;
                }
            }
        } else {
            std::cerr << "Error parsing " << filename << ": " << result.description() << std::endl;
        }
        exit(EXIT_FAILURE);
    }
}

void grammar::model::parse_and_merge_grammars(boost::filesystem::path xml_file)
{
    base_path_  = xml_file.parent_path();
    load_grammar(xml_file, grammar_);
    
    // finding grammars included from the principal one
    // NOTE: only the main grammar has includes, other includes in other
    //       grammars are ignored, to extend the parser one has to deal with
    //       the possible duplicate includes and relative paths...
    std::vector<std::shared_ptr<pugi::xml_document>> grammar_files;
    pugi::xpath_query includes("/gr:grammar//gr:include");
    for (auto& element: includes.evaluate_node_set(grammar_)) {
        std::string filename = element.node().attribute("source").value();
        std::shared_ptr<pugi::xml_document> grammar = std::make_shared<pugi::xml_document>();
        load_grammar(base_path_ / boost::filesystem::path(filename), *grammar);
        grammar_files.push_back(grammar);
        element.node().parent().remove_child(element.node());
    }
    
    // merging included files into the main one
    pugi::xpath_query derivations_query("/gr:grammar/gr:derivations");
    pugi::xml_node derivations = derivations_query.evaluate_node_set(grammar_)[0].node();
    
    pugi::xpath_query all_elements("/gr:grammar/gr:derivations/*");
    for (auto& grammar : grammar_files) {
        for (auto& element : all_elements.evaluate_node_set(*grammar)) {
            derivations.append_copy(element.node());
        }
    }
}

void grammar::model::overwrite_derivations(boost::filesystem::path xml_file)
{
    if (!xml_file.empty()) {
        pugi::xml_document overwrite_grammar;
        load_grammar(xml_file, overwrite_grammar);

        // replacing derivations in the main grammar
        pugi::xpath_query all_elements("/gr:grammar/gr:derivations/*");
        for (auto& element : all_elements.evaluate_node_set(overwrite_grammar)) {
            // looking for the same derivation
            std::string name = element.node().name();
            pugi::xpath_query element_to_replace(("/gr:grammar/gr:derivations/" + name).c_str());
            if (element_to_replace.evaluate_node_set(grammar_).size() == 0) {
                Error::fatal("No derivation " + name + " to replace.");
            }
            pugi::xml_node target = element_to_replace.evaluate_node_set(grammar_)[0].node();
            target.parent().insert_copy_after(element.node(), target);
            target.parent().remove_child(target);
        }
    }
}

bool grammar::model::has_children(const pugi::xml_node& node) const
{
    return node.children().begin() != node.children().end();
}

void grammar::model::clean_up_append_disjuncitons()
{
    // NOTE: append=disjunction can be used to merge derivations only and
    //       not any possible node inside the grammar
    pugi::xpath_query elements_to_append("/gr:grammar/gr:derivations/*[@append='disjunction']");
    for (auto& element : elements_to_append.evaluate_node_set(grammar_)) {
        std::string name = element.node().name();
        pugi::xpath_query element_to_extend(("/gr:grammar/gr:derivations/" + name + "[not(@append)]").c_str());
        if (element_to_extend.evaluate_node_set(grammar_).size() == 0) {
            Error::fatal("No derivation " + name + " to extend.");
        }
        pugi::xml_node target = element_to_extend.evaluate_node_set(grammar_)[0].node();
        target.append_child("or");
        for (auto& child : element.node().children()) {
            target.append_copy(child);
        }
        element.node().parent().remove_child(element.node());
    }
}

void grammar::model::clean_up_remove_empty_cdatas()
{
    pugi::xpath_query elements_to_remove("/gr:grammar/gr:derivations/*[text() = '']");
    for (auto& element : elements_to_remove.evaluate_node_set(grammar_)) {
        for (auto& child : element.node().children()) {
            if (child.type() == pugi::node_cdata && !strcmp(child.value(), "")) {
                element.node().remove_child(child);
            }
        }
    }
}

void grammar::model::clean_up_remove_empty_derivations()
{
    pugi::xpath_query elements_to_remove("/gr:grammar/gr:derivations/*[count(*)=0 and count(@*)=0 and not(text())]");
    std::vector<std::string> to_remove;
    for (auto& element : elements_to_remove.evaluate_node_set(grammar_)) {
        to_remove.push_back(element.node().name());
    }
    for (auto& name : to_remove) {
        std::cout << "Removing all occurrences of empty rule " << name << "." << std::endl;
        pugi::xpath_query to_remove(("/gr:grammar/gr:derivations//" + name).c_str());
        for (auto& element : to_remove.evaluate_node_set(grammar_)) {
            element.node().parent().remove_child(element.node());
        }
    }
}

void grammar::model::clean_up_remove_useless_ors()
{
    pugi::xpath_query at_least_one_or("/gr:grammar/gr:derivations//*[count(or) > 0]");
    for (auto& element : at_least_one_or.evaluate_node_set(grammar_)) {
        // we start with true so that we remove leading ORs in the sequence
        bool was_or = true;
        const auto& children = element.node().children();
        for (pugi::xml_node_iterator it = children.begin(); it != children.end(); ++it) {
            if (!strcmp(it->name(), "or")) {
                auto next = it; next++;
                // we remove also possible last OR
                if (was_or || next == children.end()) {
                    it->parent().remove_child(*it);
                }
                was_or = true;
            } else {
                was_or = false;
            }
        }
    }
}

void grammar::model::clean_up_remove_non_choices()
{
    pugi::xpath_query non_choices("/gr:grammar/gr:derivations/*[count(or)=0 and count(@*)=0]");
    for (auto& element : non_choices.evaluate_node_set(grammar_)) {
        std::string name = element.node().name();
        pugi::xpath_query where(("/gr:grammar/gr:derivations//" + name + "[count(*)=0 and count(@*)=0 and not(text())]").c_str());
        for (auto& target : where.evaluate_node_set(grammar_)) {
            pugi::xml_node last = target.node();
            for (auto& child : element.node().children()) {
                last.parent().insert_copy_after(child, last);
                last = last.next_sibling();
            }
            target.node().parent().remove_child(target.node());
        }
        element.node().parent().remove_child(element.node());
    }
}

void grammar::model::clean_up_merge_disjuncitons()
{
    pugi::xpath_query non_choices("/gr:grammar/gr:derivations/*[count(or)>=0 and count(@*)=0]");
    for (auto& element : non_choices.evaluate_node_set(grammar_)) {
        std::string name = element.node().name();
        // we check that the rule is not recursive
        bool recursive = false;
        for (auto& child : element.node().children()) {
            if (!strcmp(child.name(), name.c_str())) {
                recursive = true;
                continue;
            }
        }
        if (recursive) {
            continue;
        }
        pugi::xpath_query where(("/gr:grammar/gr:derivations//" + name + "[count(*)=0 and count(@*)=0 and not(text())]").c_str());
        bool substitution_done = false;
        bool do_not_delete = false;
        for (auto& target : where.evaluate_node_set(grammar_)) {
            // we check that the left and right siblings of target are <or/>
            auto left = target.node().previous_sibling();
            auto right = target.node().next_sibling();
            if (!(left.type() == pugi::node_null || !strcmp(left.name(), "or")) ||
                !(right.type() == pugi::node_null || !strcmp(right.name(), "or"))) {
                // if it was not deleted at least in one position, do not delete the rule
                do_not_delete = true;
                continue;
            }
            std::cerr << "replacing " << name << " in " << left.name() << "+" << target.node().name() << "+" << right.name() << std::endl;
            pugi::xml_node last = target.node();
            for (auto& child : element.node().children()) {
                last.parent().insert_copy_after(child, last);
                last = last.next_sibling();
            }
            target.node().parent().remove_child(target.node());
            substitution_done = true;
        }
        if (substitution_done && !do_not_delete) {
            std::cerr << "deleting " << element.node().name() << " from " << element.node().parent().name() << std::endl;
            element.node().parent().remove_child(element.node());
        }
    }
}

void grammar::model::clean_up_merge_cdatas()
{
    pugi::xpath_query adjacent_cdatas("/gr:grammar/gr:derivations//*[count(text()) > 1]");
    for (auto& element : adjacent_cdatas.evaluate_node_set(grammar_)) {
        pugi::xml_node replacement = element.node().parent().insert_child_after(element.node().name(), element.node());
        for (auto attribute : element.node().attributes()) {
            replacement.append_attribute(attribute.name());
            replacement.attribute(attribute.name()).set_value(element.node().attribute(attribute.name()).value());
        }
        bool was_cdata = false;
        for (auto& child : element.node().children()) {
            if (child.type() == pugi::node_cdata) {
                if (was_cdata) {
                    std::string first = replacement.last_child().value();
                    std::string second = child.value();
                    replacement.last_child().set_value((first + second).c_str());
                } else {
                    replacement.append_copy(child);
                }
                was_cdata = true;
            } else {
                replacement.append_copy(child);
                was_cdata = false;
            }
        }
        element.node().parent().remove_child(element.node());
    }
}

void grammar::model::clean_up_remove_duplicates()
{
    pugi::xpath_query all_elements("/gr:grammar/gr:derivations/*");
    for (auto& rule1 : all_elements.evaluate_node_set(grammar_)) {
        for (auto& rule2 : all_elements.evaluate_node_set(grammar_)) {
            if (rule1 != rule2 && !strcmp(rule1.node().name(), rule2.node().name())) {
                bool same = true;
                for (auto& attribute : rule1.node().attributes()) {
                    if (!strcmp(rule1.node().attribute(attribute.name()).value(), rule2.node().attribute(attribute.name()).value())) {
                        same = false;
                        break;
                    }
                }
                if (!same) {
                    std::cout << "Removing duplicate rule " << rule2.node().name() << "." << std::endl;
                    rule2.node().parent().remove_child(rule2.node());
                }
            }
        }
    }
}

void grammar::model::clean_up_remove_non_used_rules()
{
    pugi::xpath_query all_elements("/gr:grammar/gr:derivations/*[not(@output) and not(@destination) and not(@destination_dir)]");
    for (auto& element : all_elements.evaluate_node_set(grammar_)) {
        std::string name = element.node().name();
        pugi::xpath_query where(("/gr:grammar/gr:derivations//" + name + "[count(*)=0 and count(@*)=0 and not(text())]").c_str());
        if (where.evaluate_node_set(grammar_).size() == 0) {
            std::cout << "Removing unused rule " << element.node().name() << "." << std::endl;
            element.node().parent().remove_child(element.node());
        }
    }
}

void grammar::model::warn_for_duplicate_derivations()
{
    // not catching all duplicate derivations, just those whose immediate
    // children have the same name
    bool warnings = false;
    pugi::xpath_query all_elements("/gr:grammar/gr:derivations/*");
    for (auto& element1 : all_elements.evaluate_node_set(grammar_)) {
        for (auto& element2 : all_elements.evaluate_node_set(grammar_)) {
            if (has_children(element1.node()) && has_children(element2.node()) && strcmp(element1.node().name(), element2.node().name())) {
                std::vector<std::string> children1;
                std::vector<std::string> children2;
                for (auto& child : element1.node().children()) {
                    if (child.type() == pugi::node_element) {
                        children1.push_back(child.name());
                    } else if (child.type() == pugi::node_cdata) {
                        children1.push_back(child.value());
                    }
                }
                for (auto& child : element2.node().children()) {
                    if (child.type() == pugi::node_element) {
                        children2.push_back(child.name());
                    } else if (child.type() == pugi::node_cdata) {
                        children2.push_back(child.value());
                    }
                }
                if (children1.size() == children2.size()) {
                    std::sort(children1.begin(), children1.end(),
                              [](std::string const &f, std::string const &s) { return f < s; });
                    std::sort(children2.begin(), children2.end(),
                              [](std::string const &f, std::string const &s) { return f < s; });
                    if (children1 == children2) {
                        std::string n1 = element1.node().name();
                        std::string n2 = element2.node().name();
                        Error::warning(n1 + " could be a duplicate of " + n2);
                        warnings = true;
                    }
                }
            }
        }
    }
    if (warnings) {
        std::cout << std::endl;
    }
}

void grammar::model::rename_calls_inside_block(std::vector<pugi::xml_node>& block)
{
    if (block.size() == 1) {
        return;
    }
    // check that there are no multiple instantiation of rules in the same block
    std::unordered_map<std::string, int> names;
    for (auto& element : block) {
        std::string name = element.name();
        auto match = names.find(name);
        if (match == names.end()) {
            names[name] = 1;
        } else {
            int curr_val = match->second;
            names[name] = match->second + 1;
            // rename the node
            std::string new_name = name + std::to_string(names[name]);
            element.set_name(new_name.c_str());
            // check if there is already a derivation with this name
            pugi::xpath_query all_elements(("/gr:grammar/gr:derivations/" + new_name).c_str());
            if (all_elements.evaluate_node_set(grammar_).empty()) {
                std::string to_duplicate = (curr_val == 1) ? name : name + std::to_string(curr_val);
                pugi::xpath_query target_query(("/gr:grammar/gr:derivations/" + to_duplicate).c_str());
                auto target = target_query.evaluate_node_set(grammar_).first();
                target.node().parent().insert_copy_after(target.node(), target.node());
                target.node().next_sibling(to_duplicate.c_str()).set_name(new_name.c_str());
            }
        }
    }
}

void grammar::model::rename_calls()
{
    pugi::xpath_query derivation("/gr:grammar/gr:derivations/*[count(*)>=0]");
    for (auto& element : derivation.evaluate_node_set(grammar_)) {
        // for all children we make subset of elements separated by ORs
        std::vector<pugi::xml_node> block;
        block.clear();
        for (auto& child : element.node().children()) {
            if (child.type() == pugi::node_element) {
                if (!strcmp(child.name(), "or")) {
                    rename_calls_inside_block(block);
                    block.clear();
                } else if (!has_children(child)) {
                    block.push_back(child);
                }
            }
        }
        rename_calls_inside_block(block);
    }
}

// From samples/save_custom_writer.cpp as in the documentation at
// http://pugixml.googlecode.com/svn/tags/latest/docs/manual/saving.html
struct xml_string_writer: pugi::xml_writer
{
    std::string result;
    
    virtual void write(const void* data, size_t size)
    {
        result += std::string(static_cast<const char*>(data), size);
    }
};

void grammar::model::clean_up_simplify_recursions()
{
    pugi::xpath_query all_elements("/gr:grammar/gr:derivations/*[not(@output) and not(@destination) and not(@destination_dir)]");
    for (auto& element : all_elements.evaluate_node_set(grammar_)) {
        std::string name = element.node().name();
        bool recursive = false;
        for (auto& child : element.node().children()) {
            if (child.type() == pugi::node_element && !strcmp(element.node().name(), child.name())) {
                recursive = true;
            }
        }
        // rule is recursive check if one of the following four cases:
        // A ::= B | BA
        // A ::= B | AB
        // A ::= BA | B
        // A ::= AB | B
        if (recursive) {
            // checking the number of ORs
            int num_ors = 0;
            for (auto& child : element.node().children()) {
                if (!strcmp(child.name(), "or")) {
                    num_ors++;
                }
            }
            if (num_ors != 1) {
                break;
            }
            // collecting elements before and after the OR
            std::vector<pugi::xml_node> left;
            std::vector<pugi::xml_node> right;
            std::vector<pugi::xml_node> *current = &left;
            for (auto& child : element.node().children()) {
                if (!strcmp(child.name(), "or")) {
                    current = &right;
                } else {
                    current->push_back(child);
                }
            }
            // check they have the right number of elements
            std::vector<pugi::xml_node> *stop;
            std::vector<pugi::xml_node> *cont;
            if (left.size() == 1 && right.size() == 2) {
                stop = &left;
                cont = &right;
            } else if (left.size() == 1 && right.size() == 1) {
                stop = &right;
                cont = &left;
            } else {
                break;
            }
            // find name of the recursive rule
            std::string rec_node = element.node().name();
            // find name of the other rule or content (in case of CDATA)
            std::string oth_node;
            if ((*stop)[0].type() == pugi::node_element) {
                oth_node = (*stop)[0].name();
                // element with children
                if (has_children((*stop)[0])) {
                    break;
                }
                // element has the same name of the recursive rule
                if (rec_node == oth_node) {
                    break;
                }
            } else if ((*stop)[0].type() == pugi::node_cdata) {
                std::string oth_node = (*stop)[0].value();
            } else {
                break;
            }
            // selecting which of the two in cont is the one to non recursive
            pugi::xml_node *to_check;
            if ((*cont)[0].name() == rec_node) {
                to_check = &((*cont)[1]);
            } else if ((*cont)[1].name() == rec_node) {
                to_check = &((*cont)[0]);
            } else {
                break;
            }
            // checking if rule is one of the four cases (in the check above
            // we already verified that one of the cont is the recursive one
            // now we have to see if the other in cont corresponds to stop
            if (to_check->type() != (*stop)[0].type()) {
                break;
            }
            if (to_check->type() == pugi::node_element) {
                if (has_children(*to_check)) {
                    break;
                }
                if (strcmp(to_check->name(), (*stop)[0].name())) {
                    break;
                }
            } else {
                if (strcmp(to_check->value(), (*stop)[0].value())) {
                    break;
                }
            }

            // the rule is actually recursive now we have to replace wherever
            // it is used with oth_node and rec_node and replace the single
            // occourrence of oth_node with an empty CDATA
            
            // replacing where needed rec_node with rec_node oth_node (the order
            // here is not important)
            pugi::xpath_query where(("/gr:grammar/gr:derivations//" + rec_node + "[count(*)=0 and count(@*)=0 and not(text())]").c_str());
            for (auto& used_elem : where.evaluate_node_set(grammar_)) {
                if (used_elem.node() == (*cont)[1]) {
                    continue;
                }
                pugi::xml_node temp = used_elem.node().parent().insert_child_before((*stop)[0].type(), used_elem.node());
                if (temp.type() == pugi::node_cdata) {
                    temp.set_value((*stop)[0].value());
                } else if (temp.type() == pugi::node_element) {
                    temp.set_name((*stop)[0].name());
                }
            }
            
            // saving old rule for printing a note
            xml_string_writer writer1;
            element.node().print(writer1, "\t", pugi::format_raw | pugi::format_no_declaration);
            std::string old_rule = writer1.result;
            
            // changing current oth_node with cdata
            pugi::xml_node empty = (*stop)[0].parent().insert_child_after(pugi::node_cdata, (*stop)[0]);
            empty.set_value(" ");
            (*stop)[0].parent().remove_child((*stop)[0]);
            
            // saving new rule for printing a note
            xml_string_writer writer2;
            element.node().print(writer2, "\t", pugi::format_raw | pugi::format_no_declaration);
            std::string new_rule = writer2.result;
            
            // printing a note as a warning that such transformation took place
            std::cout << "To reduce the number of parameters to generate, the recursive rule:\n" << old_rule << std::endl;
            std::cout << "has been simplified to:\n" + new_rule << "." << std::endl;
            std::cout << "All occurrences of <" << rec_node << "/> in the grammar ";
            std::cout << "have also been replaced by <" << oth_node << "/><" << rec_node << "/>." << std::endl;
        }
    }
}
