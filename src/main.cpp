//
//  main.cpp
//  Grammar2Code
//
//  Created by Franco Mascia on 15/03/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "grammar.hpp"
#include "error.hpp"

void splash()
{
    std::cout << R"(                                     ___            _       )" << std::endl;
    std::cout << R"(   __ _ _ _ __ _ _ __  _ __  __ _ _ |_  )__ ___  __| |___   )" << std::endl;
    std::cout << R"(  / _` | '_/ _` | '  \| '  \/ _` | '_/ // _/ _ \/ _` / -_)  )" << std::endl;
    std::cout << R"(  \__, |_| \__,_|_|_|_|_|_|_\__,_|_|/___\__\___/\__,_\___|  )" << std::endl;
    std::cout << R"(  |___/                                                     )" << std::endl;
    std::cout << R"(                                                            )" << std::endl;
}

void version()
{
    std::cout << R"(  version )" << G2C_VERSION << std::endl;
    std::cout << R"(                                                                             )" << std::endl;
    std::cout << R"(  Copyright (c) 2013, Franco Mascia                                          )" << std::endl;
    std::cout << R"(  All rights reserved.                                                       )" << std::endl;
    std::cout << R"(                                                                             )" << std::endl;
    std::cout << R"(  Redistribution and use in source and binary forms, with or without         )" << std::endl;
    std::cout << R"(  modification, are permitted provided that the following conditions are met:)" << std::endl;
    std::cout << R"(                                                                             )" << std::endl;
    std::cout << R"(    1. Redistributions of source code must retain the above copyright        )" << std::endl;
    std::cout << R"(       notice, this list of conditions and the following disclaimer.         )" << std::endl;
    std::cout << R"(                                                                             )" << std::endl;
    std::cout << R"(    2. Redistributions in binary form must reproduce the above copyright     )" << std::endl;
    std::cout << R"(       notice, this list of conditions and the following disclaimer in the   )" << std::endl;
    std::cout << R"(       documentation and/or other materials provided with the distribution.  )" << std::endl;
    std::cout << R"(                                                                             )" << std::endl;
    std::cout << R"(  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS")" << std::endl;
    std::cout << R"(  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  )" << std::endl;
    std::cout << R"(  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE )" << std::endl;
    std::cout << R"(  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  )" << std::endl;
    std::cout << R"(  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR        )" << std::endl;
    std::cout << R"(  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF       )" << std::endl;
    std::cout << R"(  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   )" << std::endl;
    std::cout << R"(  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN    )" << std::endl;
    std::cout << R"(  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)    )" << std::endl;
    std::cout << R"(  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE )" << std::endl;
    std::cout << R"(  POSSIBILITY OF SUCH DAMAGE.                                                )" << std::endl;
    std::cout << R"(                                                                             )" << std::endl;
}

void usage(std::string prg_name, boost::program_options::options_description desc)
{
    std::cout << "Usage: " << std::endl;
    std::cout << "  " << prg_name << " grammar.xml <options> <options_to_generate_parameters>" << std::endl;
    std::cout << "  " << prg_name << " grammar.xml <options> <options_to_generate_code> parameters" << std::endl;
    std::cout << desc << std::endl;
}

void help(std::string prg_name, boost::program_options::options_description desc)
{
    usage(prg_name, desc);
    std::cout << "-----------------------------------------------------------------------------\n" << std::endl;
    std::cout << "Note that the options for the generation of the parameters and those for the" << std::endl;
    std::cout << "generation of the code can not be specified at the same time.\n"  << std::endl;
    std::cout << "Examples: " << std::endl;
    std::cout << "  " << prg_name << " grammar.xml [-o test.xml] [-d 5] [-f irace] -p parameters.txt" << std::endl;
    std::cout << "  " << prg_name << " grammar.xml [-o test.xml] -t src_code [-x] \\" << std::endl;
    std::cout << "               --parameter1=value1 [--parameter2=value2 ...]\n" << std::endl;
}

void define_options(boost::program_options::options_description& desc_full,
                    boost::program_options::options_description& desc_visible,
                    boost::program_options::positional_options_description& positional)
{
    boost::program_options::options_description desc_comm("Options");
    desc_comm.add_options()
        ("help,h", "this help message and some examples")
        ("version,v", "prints the version and license")
        ("overwrite,o", boost::program_options::value<std::string>(), "optional xml file with derivations that overwrite parts of the original grammar")
    ;

    boost::program_options::options_description desc_pars("Options for generating the parameters");
    desc_pars.add_options()
        ("depth,d", boost::program_options::value<int>()->default_value(3), "maximum recursion depth")
        ("params_format,f", boost::program_options::value<std::string>()->default_value("irace"), "format: 'irace', 'ParamILS', 'SMAC', 'crace' or 'emili' ")
        ("parameters,p", boost::program_options::value<std::string>(), "save generated parameters to file")
    ;

    boost::program_options::options_description desc_code("Options for generating the code");
    desc_code.add_options()
        ("do_not_reindent,x", boost::program_options::bool_switch()->default_value(false), "do not re-indent the geneated code")
        ("target_dir,t", boost::program_options::value<std::string>(), "target directory for the geneated code")
    ;

    positional.add("grammar", 1);
    positional.add("others", -1);
    boost::program_options::options_description desc_hidden;
    desc_hidden.add_options()
        ("grammar,g", boost::program_options::value<std::string>(), "grammar xml file")
        ("others", boost::program_options::value<std::vector<std::string>>(), "grammar xml file")
    ;

    desc_visible.add(desc_comm).add(desc_pars).add(desc_code);
    desc_full.add(desc_comm).add(desc_pars).add(desc_code).add(desc_hidden);
}

void cleanup_parameters(std::vector<std::string>& args, int argc, const char * argv[])
{
    // we clean up the parameters passed since --parameter= 1 are not easily
    // parsed by boost program_options, so we remove all white space around
    // the equal symbols (this should not impact on any parameter)
    std::vector<std::string> s_argv(argv, argv + argc);
    std::string parameters = boost::algorithm::join(s_argv, " ");
    parameters = boost::algorithm::replace_all_copy(parameters, " =", "=");
    parameters = boost::algorithm::replace_all_copy(parameters, "= ", "=");
    boost::split(args, parameters, boost::is_any_of(" "));
    args.erase(args.begin());
}

void parse_grammar_parameters(std::unordered_map<std::string, std::string>& grammar_parameters,
                              std::vector<std::string>& further_parameters)
{
    std::cout << "\x1B[33mparameters for the code generation\x1B[m\n" << std::endl;
    std::string name = "";
    std::string value = "";
    for (auto it = further_parameters.begin() ; it != further_parameters.end(); ++it) {
        // --parameter=value should be already ok from intial filtering of
        // the program options where we remove spaces around the = symbol
        if (boost::starts_with(*it, "--")) {
            std::vector<std::string> tokens;
            boost::split(tokens, *it, boost::is_any_of("="));
            name = tokens[0].substr(2);
            value = tokens[1];
        } else if (boost::starts_with(*it, "-")) {
            name = (*it).substr(1);
            value = *(++it);
        } else {
            // parameter without dashes or something went wrong previously
            Error::fatal("Cannot parse parameter " + *it + ".");
        }
        std::cerr << name << " : " << value << std::endl;
        grammar_parameters[name] = value;
    }
}


int main(int argc, const char * argv[])
{
    // clean up long parameters
    std::vector<std::string> args;
    cleanup_parameters(args, argc, argv);

    std::string prg_name = boost::filesystem::path(argv[0]).filename().string();
    Error::set_exec_name(prg_name);

    splash();

    // defining available options
    boost::program_options::options_description desc_full;
    boost::program_options::options_description desc_visible;
    boost::program_options::positional_options_description positional;
    define_options(desc_full, desc_visible, positional);

    // parsing parameters (options and eventually further parameters to transform the grammar)
    boost::program_options::variables_map vm;
    boost::program_options::parsed_options parsed = boost::program_options::command_line_parser(args).options(desc_full).positional(positional).allow_unregistered().run();
    std::vector<std::string> further_parameters = collect_unrecognized(parsed.options, boost::program_options::include_positional);
    boost::program_options::store(parsed, vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        help(prg_name, desc_visible);
        return EXIT_SUCCESS;
    }

    if (vm.count("version")) {
        version();
        return EXIT_SUCCESS;
    }

    // positional and non-optional parameters (no grammar or no (parameters XOR target_dir)
    if (vm.count("grammar") == 0 || !((vm.count("parameters") != 0) != (vm.count("target_dir") != 0))) {
        usage(prg_name, desc_visible);
        return EXIT_FAILURE;
    }

    // get and parse the grammar xml file
    boost::filesystem::path grammar_xml(vm["grammar"].as<std::string>());
    boost::filesystem::path overwrite_xml;
    if (vm.count("overwrite") != 0) {
        overwrite_xml = vm["overwrite"].as<std::string>();
    }
    std::shared_ptr<grammar::model> ruleset = std::make_shared<grammar::model>(grammar_xml, overwrite_xml);
    std::cout << "\n\x1B[33mcleaned up grammar\x1B[m\n" << std::endl;
    ruleset->grammar().print(std::cout);
    std::cout << std::endl;

    if (vm.count("parameters") != 0) {
        // generating list of parameters
        int depth = vm["depth"].as<int>();
        std::string format = vm["params_format"].as<std::string>();
        boost::algorithm::to_lower(format);
        std::shared_ptr<grammar::configuration> configuration;
        if (format == "irace") {
            configuration = std::make_shared<grammar::irace_conf>(ruleset, depth);
        } else if (format == "paramils") {
            configuration = std::make_shared<grammar::paramils_conf>(ruleset, depth);
        } else if (format == "smac") {
            configuration = std::make_shared<grammar::smac_conf>(ruleset, depth);
        } else if (format == "emili") {
            configuration = std::make_shared<grammar::emili_conf>(ruleset, depth);
        }else if (format == "crace") {
            configuration = std::make_shared<grammar::crace_conf>(ruleset, depth);
        } else {
            Error::fatal("Unrecognized file format " + vm["params_format"].as<std::string>() + ".");
        }
        std::cout << "\n\x1B[33m" << vm["params_format"].as<std::string>() << " parameters\x1B[m\n" << std::endl;
        configuration->print(std::cout);
        std::cout << std::endl;

        // saving parameters to file
        boost::filesystem::path parameters(vm["parameters"].as<std::string>());
        boost::filesystem::path param_file = parameters;
        std::ofstream par_file(param_file.string());
        if (par_file.good()) {
            if(format == "emili")
            {
                configuration->print(par_file);
                configuration->printToFile(param_file.string());
            }
            else
            {
                configuration->print(par_file);
            }
        } else {
            Error::fatal("Could not open " + parameters.string() + ".");
        }
        par_file.close();
    }

    if (vm.count("target_dir") != 0) {
        boost::filesystem::path target_dir(vm["target_dir"].as<std::string>());

        // check if there are further parameters to transfortm the grammar into code
        if (further_parameters.size() == 0) {
            Error::fatal("No parameters found for generating the code from the grammar.");
        }

        std::unordered_map<std::string, std::string> grammar_parameters;
        // the first positional parameter is the grammar name
        if (further_parameters[0] != vm["grammar"].as<std::string>()) {
            Error::fatal("First positional parameter does not correspond to the grammar.");
        }
        further_parameters.erase(further_parameters.begin());
        parse_grammar_parameters(grammar_parameters, further_parameters);

        // translating from a list of parameters
        std::cout << "\n\x1B[33mgenerating code\x1B[m\n" << std::endl;
        std::cout << "Target directory: " << target_dir << "\n" << std::endl;
        bool do_not_Reindent = vm["do_not_reindent"].as<bool>();

        grammar::params2code p2c(ruleset, grammar_parameters, target_dir, std::cout, do_not_Reindent);
        p2c.generate_code();
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
