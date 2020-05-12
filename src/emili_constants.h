//
//  emili_constants.h
//  grammar2code
//
//  Created by Federico Pagnozzi on 21/08/2019.
//  Copyright © 2019 Franco Mascia. All rights reserved.
//

#ifndef emili_constants_h
#define emili_constants_h
// COMPONENT TYPES
#define ALGO "algos"
#define INITSOL "initial_solution"
#define LOCSER "localsearch"
#define PERT "perturbation"
#define ACCP "acceptance"
#define NEIHG "neighborhood"
#define TERM "termination"
#define TABUM "tabu_memory"
#define NEIGHBORHOODCHANGE "neighborhood_change"
#define SHAKE "shake"

// CLAS strings
#define FILE_header "// This file has been created automatically.\n// Please complete the missing code before trying to compile\n"
#define HEADER_INCLUDE "#include \"../generalParser.h\""
#define CLASS_START "Class "
#define BUILDER_DEF " : public Builder\n{\npublic:"
#define CONSTRUCTOR " "

#define CLASS_END
#endif /* emili_constants_h */
