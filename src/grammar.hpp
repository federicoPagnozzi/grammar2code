//
//  grammar.hpp
//  Grammar2Code
//
//  Created by Franco Mascia on 16/03/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#ifndef __Grammar2Code__Grammar__
#define __Grammar2Code__Grammar__

// version of the library
#define G2C_VERSION "0.4-internal"

#include <boost/filesystem.hpp>

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

#include "pugixml.hpp"

// regexp works on OS X with clang 4.2 but not yet on linux with GCC 4.7.2
#ifdef __APPLE__
#include <regex>
namespace regex_ns = std;
#else
#include <boost/regex.hpp>
namespace regex_ns = boost;
#endif

namespace grammar {

class model {
public:
    model(boost::filesystem::path xml_file, boost::filesystem::path overwrite_xml_file);
    pugi::xml_document& grammar();
    boost::filesystem::path grammar_path();
    
private:
    pugi::xml_document grammar_;
    boost::filesystem::path base_path_;
    
    void load_grammar(boost::filesystem::path filename, pugi::xml_document& document);
    void parse_and_merge_grammars(boost::filesystem::path xml_file);
    void overwrite_derivations(boost::filesystem::path xml_file);
    
    // TODO: duplicate of same function in walker
    bool has_children(const pugi::xml_node& node) const;
    
    void clean_up_append_disjuncitons();
    void clean_up_remove_empty_cdatas();
    void clean_up_remove_empty_derivations();
    void clean_up_remove_useless_ors();
    void clean_up_remove_non_choices();
    void clean_up_merge_disjuncitons();
    void clean_up_remove_duplicates();
    void clean_up_remove_non_used_rules();
    void clean_up_merge_cdatas();
    void clean_up_simplify_recursions();
    void warn_for_duplicate_derivations();
    void rename_calls();

    void rename_calls_inside_block(std::vector<pugi::xml_node>& block);
};
    
class walker {
public:
    walker(std::shared_ptr<grammar::model>& a_model, int max_depth);
    virtual ~walker();

    void walk();
    // NOTE: categorical and recursive callbacks return the actual choice done
    //       this allows to prune the DFS when generating the final code, and
    //       most importantly visiting only the required nodes allows us to
    //       warn the user that a parameter for a non terminal symbol has not
    //       been passed and that the translation is incomplete.
    //       In the case of irace_conf or other classes that have to visit the
    //       whole tree the callbacks should return -1 meaning that no choice
    //       has been taken.
    virtual void callback_call(const pugi::xml_node& node, std::string path, int depth) = 0;
    virtual int callback_categorical(const pugi::xml_node& node, std::string path, int depth) = 0;
    virtual int callback_recursive(const pugi::xml_node& node, std::string path, int depth) = 0;
    virtual void callback_range(const pugi::xml_node& node, std::string path, int depth) = 0;
    virtual void callback_copy(const pugi::xml_node& node, std::string path, int depth) = 0;
    virtual void callback_cdata(const pugi::xml_node& node, std::string path, int depth) = 0;
    virtual void callback_plain(const pugi::xml_node& node, std::string path, int depth) = 0;
    
protected:
    std::shared_ptr<grammar::model> model_;
    int max_depth_;

    //--------------------------------node types--------------------------------
    // call         empty element <element/> that should be replaced by the
    //              content of a derivation rule in the derivations list
    // categorical  this is the standard rule that contains children separated
    //              by <or/> elements
    // recursive    rule that has a "call" rule among its children
    // range        range rules usually they have a type attribute that allows
    //              distinguishing between real-valued and integer-valued ranges
    // copy         rule in the form <gr::copy source="..." destination="..." />
    // cdata        pure text to be copied and pasted, no choices
    // plain        node that contains only cdatas or "calls" to derivations
    //              these nodes are usually top-level nodes with an output
    //              attribute to generate a source file
    //--------------------------------------------------------------------------
    enum class node_type { call, categorical, recursive, range, copy, cdata, plain };

    std::vector<std::vector<pugi::xml_node>> get_choice(pugi::xml_object_range<pugi::xml_node_iterator> children);

    // TODO: duplicate of same function in model
    bool has_children(const pugi::xml_node& node) const;
    bool has_attributes(const pugi::xml_node& node) const;

private:
    void do_walk(const pugi::xml_node& node, std::string parent, int depth);
    node_type type(const pugi::xml_node& node);
};
    
class configuration : public walker {
public:
    configuration(std::shared_ptr<grammar::model>& a_model, int max_depth) : walker(a_model, max_depth) {};
    virtual ~configuration() {}
    
    virtual void callback_call(const pugi::xml_node& node, std::string path, int depth);
    virtual int callback_categorical(const pugi::xml_node& node, std::string path, int depth);
    virtual int callback_recursive(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_range(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_copy(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_cdata(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_plain(const pugi::xml_node& node, std::string path, int depth);
    
    virtual void print(std::ostream& stream);
    virtual void printToFile(const std::string& filename) {}
protected:
    std::vector<std::string> parameters_;
   
    // in the format parameter function there is also a default value and a log-scale value
    // that are taken in consideration only by some type of parameters for some specific
    // parameter formats
    virtual std::string fmt_rule_name(const std::string& path) = 0;
    virtual std::string fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index = -1) = 0;
    virtual void fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond) = 0;

    std::pair<std::string, std::string> rule_name(const std::string& path);
    std::pair<std::string, std::string> rule_cond(const std::string& path, const std::string& node_name, int rec_index);

private:
    // store parameter names to extra check that ther are no duplicates
    std::vector<std::string> parameter_names_;
    
    void stop_if_duplicate_parameters(std::string parameter);
};
    
class irace_conf : public configuration {
public:
    irace_conf(std::shared_ptr<grammar::model>& a_model, int max_depth) : configuration(a_model, max_depth) {};
    virtual ~irace_conf() {}

protected:
    virtual std::string fmt_rule_name(const std::string& path);
    virtual std::string fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index = -1);
    virtual void fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond);
};

class smac_conf : public configuration {
public:
    smac_conf(std::shared_ptr<grammar::model>& a_model, int max_depth) : configuration(a_model, max_depth) {};
    virtual ~smac_conf() {}
    
    virtual void print(std::ostream& stream);

protected:
    std::vector<std::string> conditionals_;

    virtual std::string fmt_rule_name(const std::string& path);
    virtual std::string fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index = -1);
    virtual void fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond);
};

class paramils_conf : public configuration {
public:
    paramils_conf(std::shared_ptr<grammar::model>& a_model, int max_depth) : configuration(a_model, max_depth) {};
    virtual ~paramils_conf() {}
    
    virtual void callback_range(const pugi::xml_node& node, std::string path, int depth);
    virtual void print(std::ostream& stream);
protected:
    std::vector<std::string> conditionals_;

    virtual std::string fmt_rule_name(const std::string& path);
    virtual std::string fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index = -1);
    virtual void fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond);
};
    
class params2code : public walker {
public:
    params2code(std::shared_ptr<grammar::model>& a_model, std::unordered_map<std::string, std::string>& parameters, boost::filesystem::path target_dir, std::ostream& stream, bool do_not_reindent);
    virtual ~params2code();

    void generate_code();
    
    virtual void callback_call(const pugi::xml_node& node, std::string path, int depth);
    virtual int callback_categorical(const pugi::xml_node& node, std::string path, int depth);
    virtual int callback_recursive(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_range(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_copy(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_cdata(const pugi::xml_node& node, std::string path, int depth);
    virtual void callback_plain(const pugi::xml_node& node, std::string path, int depth);

private:
    std::unordered_map<std::string, std::string> parameters_;
    std::unordered_map<std::string, std::string> parameters_bckp_;
    boost::filesystem::path target_dir_;
    std::ostream& stream_;
    bool do_not_reindent_;
    std::vector<std::string> code_;
    
    std::unique_ptr<std::ofstream> current_fout_;
    
    void write_and_close_current_output_file();
    void output_file(boost::filesystem::path output_file);

    void copy_single_files();
    void copy_files_with_filter();    
};
    
class emili_conf : public configuration {
   public:
       emili_conf(std::shared_ptr<grammar::model>& a_model, int max_depth) : configuration(a_model, max_depth),once(1) {};
    virtual void print(std::ostream& stream);
    virtual void printToFile(const std::string& filename);
    virtual ~emili_conf() {}
protected:
    int once;
    std::vector< std::string > header_;
    std::string cname;
    virtual std::string fmt_rule_name(const std::string& path);
    virtual std::string fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index = -1);
    virtual void fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond);
    virtual void writeParametersCode(const char* parameter_name, int parameter_index);
    virtual void writeComponentsCode(pugi::xpath_query& component);

    virtual void writeAlgosCode();
    virtual void writeInitialSolutionsCode();
    virtual void writeTerminationCode();
    virtual void writeNeighborhoodCode();
    virtual void writePerturbationCode();
    virtual void writeAcceptanceCode();
    virtual void writeTabuMemoryCode();
    virtual void writeShakeCode();
    virtual void writeNeighborhoodChangeCode();
    virtual void startClass();
    virtual void endClass();
};
    
}

#endif /* defined(__Grammar2Code__Grammar__) */
