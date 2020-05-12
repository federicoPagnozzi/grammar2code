//
//  emili_conf.cpp
//  grammar2code
//
//  Created by Federico Pagnozzi on 16/08/2019.
//  Copyright © 2019 Franco Mascia. All rights reserved.
//


#include "grammar.hpp"
#include "error.hpp"
#include "emili_constants.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <vector>
#include <sstream>

void grammar::emili_conf::print(std::ostream& stream)
{
    parameters_.clear();
    header_.clear();
    pugi::xpath_query output_file("/gr:grammar/gr:derivations/*[@output]");
    for(auto& el : output_file.evaluate_node_set(model_->grammar()))
    {
        const pugi::xml_node& node = el.node();
        const pugi::xml_attribute& outfile = node.attribute("output");
       // std::cout <<" " << node.name() << " : " << outfile.value() <<  "\n";
        cname = outfile.value();
    }
    /*
    pugi::xpath_query all_elements("/gr:grammar/gr:derivations/ *[contains(name(),\"acceptance\")]");
    for (auto& element : all_elements.evaluate_node_set(model_->grammar())) {
        //do_walk(element.node(), "", 0);
        const pugi::xml_node& node = element.node();
        std::cout << node.path() << "\n";
        int k = 0;
        int params = 0;
        for(auto& child : node.children())
        {
           // std::cout << child.name() << "\n";
            if (child.type() == pugi::node_cdata && k==0)
            {
                std::cout << "---" << child.value() << " " << k ;
                k = 1;
            }
            else if(strcmp( child.name(), "or")==0)
            {
                k = 0;
                std::cout << " # " << params << " +++\n";
                params = 0;
            }
            else if(child.type() != pugi::node_cdata)
            {
              //  xml_attribute& k = child.attribute("type");
                writeParametersCode(child.name(), params);
                std::string parameter_query;
                parameter_query = "/gr:grammar/gr:derivations/ *[contains(name(),\"" +  std::string(child.name()) + "\")]";
                pugi::xpath_query parameter(parameter_query.c_str());
                pugi::xml_node par;
                for (auto& element : parameter.evaluate_node_set(model_->grammar()))
                {
                    par = element.node();
                    pugi::xml_attribute att = par.attribute("type");
                    if(att)
                    {
                        std::cout << att.name() << "  " << att.value() << "\n";
                        break;
                    }
                }
                
                std::cout << "parameter : " << child.name() << " " <<  par.attribute("type") << " \n";
                params++;
                //std:: cout << "\n" ;
            }
        }
        std::cout << " # " << params << " +++\n";
        
    }*/
    startClass();
    writeAlgosCode();
    writeInitialSolutionsCode();
    writeTerminationCode();
    writeNeighborhoodCode();
    writePerturbationCode();
    writeAcceptanceCode();
    writeTabuMemoryCode();
    writeShakeCode();
    writeNeighborhoodChangeCode();
    endClass();
    
    for (auto& header : header_)
    {
        stream << header << std::endl;
    }
    for (auto& parameter : parameters_) {
        stream << parameter << std::endl;
    }
}

std::string grammar::emili_conf::fmt_rule_name(const std::string& path)
{
    auto param_name = rule_name(path);
    return param_name.first + "\t\"--" + param_name.second + "=\"\t";
}

std::string grammar::emili_conf::fmt_rule_cond(const std::string& path, const std::string& node_name, int rec_index)
{
    auto cond = rule_cond(path, node_name, rec_index);
    auto cond_name = rule_name(cond.first);
    
    if (cond.first.empty() || cond.second.empty()) {
        return "";
    } else {
        return "\t| " + cond_name.first + " %in% c(" + cond.second + ")";
    }
}

void grammar::emili_conf::fmt_parameter(const std::string& rule_name, const std::string& rule_type, const std::vector<std::string>& values, std::string default_value, bool log_scale, std::string rule_cond)
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

void grammar::emili_conf::writeParametersCode(const char* parameter_name, int parameter_index)
{
    std::string parameter_query;
    parameter_query = "/gr:grammar/gr:derivations/*[contains(name(),\"" +  std::string(parameter_name) + "\")]";
    pugi::xpath_query parameter(parameter_query.c_str());
    std::ostringstream oss;
    int found = 0;
    for (auto& element : parameter.evaluate_node_set(model_->grammar()))
    {
        const pugi::xml_node& par = element.node();
        const pugi::xml_attribute& att = par.attribute("type");
        
        if(att)
        {
           // std::cout << att.name() << "  " << att.value() << "\n";
            if (strcmp( att.value(), "real")==0)
            {
                oss << "    float parameter" << parameter_index << " = tm.getDecimal();\n";
                oss << "    printTabPlusOne(\"float parameter"<< parameter_index << "\", parameter"<< parameter_index << ");\n";
               parameters_.push_back(oss.str());
            }
            else if(strcmp(att.value(),"int")==0)
            {                
                oss << "    int parameter" << parameter_index << " = tm.getInteger();\n";
                oss << "    printTabPlusOne(\"int parameter"<< parameter_index << "\", parameter"<< parameter_index << ");\n";
                parameters_.push_back(oss.str());
            }
            found = 1;
            break;
        }
    }
        if(!found)
        {
            //std::cout << "\n" << parameter_name << " " << parameter_index << "\n";
            if(strstr(parameter_name,ALGO) || strstr(parameter_name,LOCSER))
            {
                oss << "    emili::LocalSearch* parameter"<< parameter_index <<" = retrieveComponent(COMPONENT_ALGORITHM).get<emili::LocalSearch>();\n";
            }
            else if(strstr(parameter_name, INITSOL))
            {
                oss << "    emili::InitialSolution* parameter"<< parameter_index << " = retrieveComponent(COMPONENT_INITIAL_SOLUTION_GENERATOR).get<emili::InitialSolution>();\n";
            }
            else if(strstr(parameter_name, NEIHG))
            {
                oss << "    emili::Neighborhood* parameter"<< parameter_index <<" = retrieveComponent(COMPONENT_NEIGHBORHOOD).get<emili::Neighborhood>();\n";
            }
            else if(strstr(parameter_name, TERM))
            {
                oss << "    emili::Termination* parameter"<< parameter_index << " = retrieveComponent(COMPONENT_TERMINATION_CRITERION).get<emili::Termination>();\n";
            }
            else if(strstr(parameter_name, PERT))
            {
                oss << "    emili::Perturbation* parameter" << parameter_index << " = retrieveComponent(COMPONENT_PERTURBATION).get<emili::Perturbation>();\n";
            }
            else if(strstr(parameter_name, ACCP))
            {
                oss << "    emili::Acceptance* parameter"<< parameter_index << " = retrieveComponent(COMPONENT_ACCEPTANCE).get<emili::Acceptance>();\n";
            }
            else if(strstr(parameter_name, TABUM))
            {
                oss << "    emili::TabuMemory* parameter"<< parameter_index << " = retrieveComponent(COMPONENT_TABU_TENURE).get<emili::TabuMemory>();\n";
            }
            else if(strstr(parameter_name, SHAKE))
            {
                oss << "    emili::Shake* parameter" << parameter_index << " = retrieveComponent(COMPONENT_SHAKE).get<emili::Shake>();\n";

            }
            else if(strstr(parameter_name, NEIGHBORHOODCHANGE))
            {
                oss << "    emili::NeighborhoodChange* parameter" << parameter_index << " = retrieveComponent(COMPONENT_NEIGHBORHOOD_CHANGE).get<emili::NeighborhoodChange>();\n";
            }
            else
            {
                oss << "    //Grammar2code is not able to load paramter " << parameter_name <<  " correctly and it has to be loaded manually \n";
                oss << "    //TYPE paramter"<< parameter_index << " = ? ;\n";
            }
            parameters_.push_back(oss.str());
        }
    
}

void grammar::emili_conf::writeComponentsCode(pugi::xpath_query& component)
{
    std::ostringstream oss;
    int index = 0;
    parameters_.push_back("  prs::incrementTabLevel();\n  std::ostringstream oss;");
    for (auto& element : component.evaluate_node_set(model_->grammar())) {
        //do_walk(element.node(), "", 0);
        oss.str("");
        const pugi::xml_node& node = element.node();
       // std::cout << node.path() << "\n";
        int k = 0;
        int params = 0;
        for(auto& child : node.children())
        {
            // std::cout << child.name() << "\n";
            if (child.type() == pugi::node_cdata && k==0)
            {
                oss.str("");
               // std::cout << "---" << child.value() << " " << k ;
                if(index)
                {
                    oss << "  else";
                }
                else
                {
                    index++;
                }
                std::string chiname(child.value());
                boost::algorithm::trim(chiname);
                oss << "  if( tm.checkToken(\"" << chiname << "\"))\n  {\n  " << "  printTab(\""<< child.value() <<"\" );\n";
                parameters_.push_back(oss.str());
                k = 1;
            }
            else if(strcmp( child.name(), "or")==0)
            {
                k = 0;
                parameters_.push_back("    //TODO write code to instantiate component\n    //comp = new Component(parameters);\n    comp = nullptr;\n  }\n");
              //  std::cout << " # " << params << " +++\n";
                params = 0;
            }
            else if(child.type() != pugi::node_cdata)
            {
                //  xml_attribute& k = child.attribute("type");
                writeParametersCode(child.name(), params);
                params++;
                //std:: cout << "\n" ;
            }
        }
        parameters_.push_back("    //TODO write code to instantiate component\n    //comp = new Component(parameters);\n    comp = nullptr;\n  }\n");
        //std::cout << " # " << params << " +++\n";
    }
}

void grammar::emili_conf::writeInitialSolutionsCode()
{
    header_.push_back("virtual emili::InitialSolution* buildInitialSolution();");
    parameters_.push_back("emili::InitialSolution* prs::"+cname+"Builder::buildInitialSolution()");
    parameters_.push_back("{\n  emili::InitialSolution* comp = nullptr;");
    std::string initsol("/gr:grammar/gr:derivations/*[contains(name(),\"");
    initsol = initsol + INITSOL +  "\")]";
    pugi::xpath_query all_elements(initsol.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n ");
}

void grammar::emili_conf::writeAlgosCode()
{
    header_.push_back("virtual emili::LocalSearch* buildAlgo();");
    parameters_.push_back("emili::LocalSearch* prs::"+cname+"Builder::buildAlgo()");
    parameters_.push_back("{\n  emili::LocalSearch* comp = nullptr;");
    std::string algorithms("/gr:grammar/gr:derivations/*[contains(name(),\"");
    algorithms = algorithms + ALGO + "\")]";
    pugi::xpath_query all_elements(algorithms.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::writeTerminationCode()
{
    header_.push_back("virtual emili::Termination* buildTermination();");
    parameters_.push_back("emili::Termination* prs::"+cname+"Builder::buildTermination()");
    parameters_.push_back("{\n  emili::Termination* comp = nullptr;");
    std::string termination("/gr:grammar/gr:derivations/*[contains(name(),\"");
    termination = termination + TERM + "\")]";
    pugi::xpath_query all_elements(termination.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::writeNeighborhoodCode()
{
    header_.push_back("virtual emili::Neighborhood* buildNeighborhood();");
    parameters_.push_back("emili::Neighborhood* prs::"+cname+"Builder::buildNeighborhood()");
    parameters_.push_back("{\n  emili::Neighborhood* comp = nullptr;");
    std::string neighborhood("/gr:grammar/gr:derivations/*[contains(name(),\"");
    neighborhood = neighborhood + NEIHG + "\")]";
    pugi::xpath_query all_elements(neighborhood.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::writePerturbationCode()
{
    header_.push_back("virtual emili::Perturbation* buildPerturbation();");
    parameters_.push_back("emili::Perturbation* prs::"+cname+"Builder::buildPerturbation()");
    parameters_.push_back("{\n  emili::Perturbation* comp = nullptr;");
    std::string perturbation("/gr:grammar/gr:derivations/*[contains(name(),\"");
    perturbation = perturbation + PERT + "\")]";
    pugi::xpath_query all_elements(perturbation.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::writeAcceptanceCode()
{
    header_.push_back("virtual emili::Acceptance* buildAcceptance();");
    parameters_.push_back("emili::Acceptance* prs::"+cname+"Builder::buildAcceptance()");
    parameters_.push_back("{\n  emili::Acceptance* comp = nullptr;");
    std::string acceptance("/gr:grammar/gr:derivations/*[contains(name(),\"");
    acceptance = acceptance + ACCP + "\")]";
    pugi::xpath_query all_elements(acceptance.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::writeTabuMemoryCode()
{
    header_.push_back("virtual emili::TabuMemory* buildTabuTenure();");
    parameters_.push_back("emili::TabuMemory* prs::"+cname+"Builder::buildTabuTenure()");
    parameters_.push_back("{\n  emili::TabuMemory* comp = nullptr;");
    std::string tabu_memory("/gr:grammar/gr:derivations/*[contains(name(),\"");
    tabu_memory = tabu_memory + TABUM + "\")]";
    pugi::xpath_query all_elements(tabu_memory.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::writeShakeCode()
{
    header_.push_back("virtual emili::Shake* buildShake();");
    parameters_.push_back("emili::Shake* prs::"+cname+"Builder::buildShake()");
    parameters_.push_back("{\n  emili::Shake* comp = nullptr;");
    std::string shake("/gr:grammar/gr:derivations/*[contains(name(),\"");
    shake = shake + SHAKE + "\")]";
    pugi::xpath_query all_elements(shake.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::writeNeighborhoodChangeCode()
{
    header_.push_back("virtual emili::NeighborhoodChange* buildNeighborhoodChange();");
    parameters_.push_back("emili::NeighborhoodChange* prs::"+cname+"Builder::buildNeighborhoodChange()");
    parameters_.push_back("{\n  emili::NeighborhoodChange* comp = nullptr;");
    std::string neighborhood_change("/gr:grammar/gr:derivations/*[contains(name(),\"");
    neighborhood_change = neighborhood_change + NEIGHBORHOODCHANGE + "\")]";
    pugi::xpath_query all_elements(neighborhood_change.c_str());
    writeComponentsCode(all_elements);
    parameters_.push_back("  prs::decrementTabLevel();\n  return comp;\n}\n");
}

void grammar::emili_conf::startClass()
{    
    std::string emp(" ");
    header_.push_back(FILE_header);
    header_.push_back("#ifndef BUILDER" + cname + "_H\n#define  BUILDER" + cname + "_H");
    header_.push_back("namespace prs \n{\nnamespace " + cname + "\n{\n\nclass "+ cname +"Builder: public Builder");
    header_.push_back("{");
    header_.push_back("public:");
    header_.push_back(cname + "Builder(GeneralParserE& generalParser,TokenManager& tokenManager):Builder(generalParser,tokenManager) { }");
    header_.push_back("virtual bool isCompatibleWith(char* problem_definition){return true;}");
    parameters_.push_back(FILE_header);
    parameters_.push_back("#include \""+cname+".h\"");
}

 void grammar::emili_conf::endClass()
{
    header_.push_back("\n};\n}\n}#endif\n");
}

void grammar::emili_conf::printToFile(const std::string& filename)
{
    //setup class names
    cname = filename;
    //Clear vectors
    parameters_.clear();
    header_.clear();
    //BUILD classes
    startClass();
    writeAlgosCode();
    writeInitialSolutionsCode();
    writeTerminationCode();
    writeNeighborhoodCode();
    writePerturbationCode();
    writeAcceptanceCode();
    writeTabuMemoryCode();
    writeShakeCode();
    writeNeighborhoodChangeCode();
    endClass();
    //Write header file
    boost::filesystem::path header_file_name = filename+".h";
    std::ofstream header_file(header_file_name.string());
    if (header_file.good()) {
        for (auto& header : header_)
        {
             header_file << header << std::endl;
        }
    } else {
        Error::fatal("Could not open " + filename+".h " + ".");
    }
    header_file.close();
    //Write code file
    boost::filesystem::path code_file_name = filename+".cpp";
    std::ofstream cpp_file(code_file_name.string());
    if (cpp_file.good()) {
        for (auto& parameter : parameters_)
        {
            cpp_file << parameter << std::endl;
        }
    } else {
        Error::fatal("Could not open " + filename+".cpp " + ".");
    }
    cpp_file.close();
}
