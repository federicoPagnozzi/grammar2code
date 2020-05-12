//
//  params2code.cpp
//  grammar2code
//
//  Created by Franco Mascia on 19/03/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include "grammar.hpp"
#include "error.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>

#if (BOOST_VERSION / 100000) < 1 || ((BOOST_VERSION / 100) % 1000) < 48
#define normalise_path(path_s)(boost::filesystem::absolute(path_s))
#else
#define normalise_path(path_s)(boost::filesystem::canonical(path_s))
#endif

// passing std::numeric_limits<int>::max() as max_depth to the constructor of
// walker since when generating the code the depth is actually limited by the
// parameters
grammar::params2code::params2code(std::shared_ptr<grammar::model>& a_model, std::unordered_map<std::string, std::string>& parameters, boost::filesystem::path target_dir, std::ostream& stream, bool do_not_reindent) : walker(a_model, std::numeric_limits<int>::max()), parameters_{parameters}, target_dir_{target_dir}, stream_(stream), do_not_reindent_(do_not_reindent), code_(), current_fout_(new std::ofstream())
{
}

grammar::params2code::~params2code()
{
}

void grammar::params2code::callback_call(const pugi::xml_node& node, std::string path, int depth)
{
}

int grammar::params2code::callback_categorical(const pugi::xml_node& node, std::string path, int depth)
{    
    if (strcmp(node.attribute("output").value(), "")) {
        output_file(node.attribute("output").value());
    }
    
    // selecting choice to prune the DFS and visit only required nodes
    int choice = -1;
    boost::replace_all(path, ":", "-");
    for (auto it = parameters_.begin() ; it != parameters_.end(); ++it) {
        if (it->first ==  path) {
            choice = std::stoi(it->second);
            parameters_.erase(it);
            break;
        }
    }
    if (choice == -1) {
        Error::fatal("No parameter to translate '" + path + "'.");
    }
    
    return  choice;
}

int grammar::params2code::callback_recursive(const pugi::xml_node& node, std::string path, int depth)
{
    if (strcmp(node.attribute("output").value(), "")) {
        output_file(node.attribute("output").value());
    }
    
    // selecting choice to prune the DFS and visit only required nodes
    int choice = -1;
    boost::replace_all(path, ":", "-");
    for (auto it = parameters_.begin() ; it != parameters_.end(); ++it) {
        if (it->first ==  path) {
            choice = std::stoi(it->second);
            parameters_.erase(it);
            break;
        }
    }
    if (choice == -1) {
        Error::fatal("No parameter to translate '" + path + "'.");
    }
    
    return choice;
}

void grammar::params2code::callback_range(const pugi::xml_node& node, std::string path, int depth)
{
    bool found = false;
    boost::replace_all(path, ":", "-");
    for (auto it = parameters_.begin() ; it != parameters_.end(); ++it) {
        if (it->first ==  path) {
            code_.push_back(it->second);
            parameters_.erase(it);
            found = true;
            break;
        }
    }
    if (!found) {
        Error::fatal("No parameter to translate '" + path + "'.");
    }

}

void grammar::params2code::callback_copy(const pugi::xml_node& node, std::string path, int depth)
{
}

void grammar::params2code::callback_cdata(const pugi::xml_node& node, std::string path, int depth)
{
    code_.push_back(node.value());
}

void grammar::params2code::callback_plain(const pugi::xml_node& node, std::string path, int depth)
{
    if (strcmp(node.attribute("output").value(), "")) {
        output_file(node.attribute("output").value());
    }
}

void grammar::params2code::output_file(boost::filesystem::path output_file)
{
    write_and_close_current_output_file();
    
    // create target dir if not there
    // NOTE: we make absolute and not canonical since file is not yet there
    boost::filesystem::path output = boost::filesystem::absolute(target_dir_ / output_file);
    if (!boost::filesystem::is_directory(output.parent_path())) {
        if(!boost::filesystem::create_directories(output.parent_path())) {
            Error::fatal("Could not create " + output.parent_path().string() + ".");
        }
    }
    
    stream_ << "Output file " << output << "\n" << std::endl;
    current_fout_->open(output.string());
    if (!current_fout_->good()) {
        Error::fatal("Could not write " + output_file.string() + ".");
    }
}

void grammar::params2code::copy_single_files()
{
    pugi::xpath_query to_be_copied("/gr:grammar/gr:derivations/*[@source and @destination]");
    for (auto& element : to_be_copied.evaluate_node_set(model_->grammar())) {
        boost::filesystem::path source(element.node().attribute("source").value());
        boost::filesystem::path destination(element.node().attribute("destination").value());
        
        boost::filesystem::path src = normalise_path(model_->grammar_path() / source);
        // create target dir if not there
        // NOTE: we make absolute and not canonical since file is not yet there
        boost::filesystem::path dst = boost::filesystem::absolute(target_dir_ / destination);
        if (!boost::filesystem::is_directory(dst.parent_path())) {
            if(!boost::filesystem::create_directories(dst.parent_path())) {
                Error::fatal("Could not create " + dst.parent_path().string() + ".");
            }
        }
        
        stream_ << "Copying " << src.string() << " to " << dst.string() << std::endl;
        boost::filesystem::copy_file(src, dst, boost::filesystem::copy_option::overwrite_if_exists);
    }
    stream_ << std::endl;
}

void grammar::params2code::copy_files_with_filter()
{
    pugi::xpath_query to_be_copied("/gr:grammar/gr:derivations/*[@source_dir and @destination_dir and @regex_filter]");
    for (auto& element : to_be_copied.evaluate_node_set(model_->grammar())) {
        boost::filesystem::path source(element.node().attribute("source_dir").value());
        boost::filesystem::path destination(element.node().attribute("destination_dir").value());
        std::string files_filter = element.node().attribute("regex_filter").value();
    
        boost::filesystem::path src = normalise_path(model_->grammar_path() / source);
        // create target dir if not there
        // NOTE: we make absolute and not canonical since file is not yet there
        boost::filesystem::path dst = boost::filesystem::absolute(target_dir_ / destination);
        if (!boost::filesystem::is_directory(dst.parent_path())) {
            if(!boost::filesystem::create_directories(dst.parent_path())) {
                Error::fatal("Could not create " + dst.parent_path().string() + ".");
            }
        }
        
        // iterate through all files in source directory
        regex_ns::regex filter(files_filter);
        regex_ns::smatch m;
        boost::filesystem::directory_iterator begin(src);
        boost::filesystem::directory_iterator end;
        for (auto it = begin; it != end; ++it) {
            if (boost::filesystem::is_regular_file(it->status())) {
                std::string filename = it->path().filename().string();
                if (regex_ns::regex_search(filename, m, filter)) {
                    boost::filesystem::path cp_src = src / filename;
                    boost::filesystem::path cp_dst = dst / filename;
                    stream_ << "Copying " << cp_src << " to " << cp_dst << std::endl;
                    boost::filesystem::copy_file(cp_src, cp_dst, boost::filesystem::copy_option::overwrite_if_exists);
                }
            }
        }
    }
    stream_ << std::endl;
}

void grammar::params2code::write_and_close_current_output_file()
{
    if (!code_.empty()) {
        // merging cdata blocks and then re-splitting into lines
        std::string code = boost::join(code_, "");
        std::vector<std::string> lines;
        boost::split(lines, code, boost::is_any_of("\n\r"));
        
        // remove trailing empty lines
        auto line = lines.rbegin();
        std::string trimmed;
        do {
            trimmed = *line;
            boost::trim(trimmed);
            if (trimmed.empty()) {
                line = std::vector<std::string>::reverse_iterator(lines.erase((++line).base()));
            } else {
                line++;
            }
        } while (line != lines.rend() && trimmed.empty());
     
        // computing the indentation to be removed
        long indentation;
        if (do_not_reindent_) {
            indentation = 0;
        } else {
            indentation = std::numeric_limits<long>::max();
            for(auto& line : lines) {
                long len = line.find_first_not_of(" \t\r\n");
                if (len >= 0 and len < indentation) {
                    indentation = len;
                }
            }
            if (indentation == std::numeric_limits<long>::max()) {
                indentation = 0;
            }
        }
        
        // writing the lines on file and out stream
        for(auto& line : lines) {
            std::string trimmed = line;
            boost::trim(trimmed);
            if (!trimmed.empty()) {
                (*current_fout_) << line.substr(indentation);
                stream_ << line.substr(indentation);
            }
            (*current_fout_) << std::endl;
            stream_ << std::endl;
        }
        (*current_fout_) << std::endl;
        stream_ << std::endl;
        
        code_.clear();
    }
    current_fout_->close();
}

void grammar::params2code::generate_code()
{
    // files to be copied first
    copy_single_files();
    copy_files_with_filter();

    // generate other files
    parameters_bckp_ = parameters_;
    walk();
    for (auto& param : parameters_) {
        Error::warning("parameter \"" + param.first + " : " + param.second + \
                       "\" was not used during code generation.");
    }
    parameters_ = parameters_bckp_;
    
    write_and_close_current_output_file();
}
