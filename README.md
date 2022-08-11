grammar2code
============

```grammar2code``` is a tool for mapping grammars to their parametric
representation. When used in combination with a tool of automatic algorithm
configuration, ```grammar2code``` can be used to automatically design
algorithms for a given problem.

This software is [open source](http://opensource.org/) and is distributed
under the terms of the
[BSD 2-Clause License](http://opensource.org/licenses/BSD-2-Clause).

If you use ```grammar2code``` in your scientific work, please cite the works
below; if you do a derivative work, you should say it is derivative and also
cite the papers below:

 *  Franco Mascia, Manuel López-Ibáñez, Jérémie Dubois-Lacoste, and Thomas
    Stützle.
    **Grammar-based Generation of Stochastic Local Search Heuristics Through
    Automatic Algorithm Configuration Tools.**
    Computers & Operations Research, 51(0):190-199, 2014.
    DOI: [10.1016/j.cor.2014.05.020](http://dx.doi.org/10.1016/j.cor.2014.05.020)

#### License ####

Copyright (c) 2013, Franco Mascia
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#### Download ####

The latest version (```0.2-internal```) can be downloaded here:

| operating system                        | download                                                       |
| --------------------------------------- | -------------------------------------------------------------- |
| OS X 10.9 (Mavericks)                   | [grammar2code_osx.zip](distribution/grammar2code_osx.zip)             |
| *Unix systems (not tested on Windows).* | [grammar2code_sources.zip](distribution/grammar2code_sources.zip)     |


Introduction
-------------

#### How does it work ####

Given a grammar describing algorithmic building blocks and how they can be
combined together, ```grammar2code``` produces a set of parameters that
describe all possible derivations defined in the grammar. To each
instantiation of the parameters corresponds a sequence of derivations that
produces one of the algorithms in the design space defined in the grammar.

#### A simple example ####

To be understood by ```grammar2code```, grammars are specified in an XML
format. The set of parameters produced in output is in a format suitable for
[irace](http://iridia.ulb.ac.be/irace/),
[ParamILS](http://www.cs.ubc.ca/labs/beta/Projects/ParamILS/), or
[SMAC](http://www.cs.ubc.ca/labs/beta/Projects/SMAC/).
```grammar2code``` can be easily extended to produce parameters for other
tools for automatic algorithm configuration. Most examples in this document
will show parameters in [irace](http://iridia.ulb.ac.be/irace/) format.

For example the following grammar:

```xml
    <?xml version="1.0" encoding="UTF-8" ?>
    <gr:grammar xmlns:gr="grammar">
        <gr:derivations>
            <start output="hello.c">
                <![CDATA[
                  #include <stdio.h>

                  int main() {
                    printf("]]><message/><![CDATA[\n");
                  }
                  ]]>
            </start>
            <message>
              <![CDATA[hello, world]]>
              <or/>
              <![CDATA[Hello World!]]>
            </message>
        </gr:derivations>
    </gr:grammar>
```

when passed as input to ```grammar2code```, the following parameter list (in
this case consisting of only one parameter) will be produced:

```
    startmessage  "--start%message="  c (0, 1)
```

Given a grammar and a parameter instantiation, ```grammar2code``` will produce
the corresponding algorithm. For example, provided with the XML grammar above
and the parameter ```--start%message=1```, ```grammar2code``` will produce a
file called ```hello.c``` containing the following program:

```c
    #include <stdio.h>

    int main() {
        printf("Hello World!\n");
    }
```

#### Why it is useful ####

```grammar2code``` is useful when used in combination with a tool for
automatic algorithm configuration. The following example gives you a high
level overview about how you can setup an automatic procedure for designing
efficient algorithms for a problem to be solved:

  1. define the building blocks, as well as the possible ways to combine them;
  2. use ```grammr2code``` to generate a list of parameters;
  3. feed the list of parameters to a tool for automatic algorithm
     configuration (for example [irace](http://iridia.ulb.ac.be/irace/)) along
     with a set of training instances, and the scripts necessary for compiling
     and testing the code;
  4. let the tool for automatic algorithm configuration do for you a series of
     experiments (up to hundreds of thousands on a cluster) to test an
     extremely large number of design choices, and select a high performing
     algorithm for the problem being tackled;
  5. publish.

```
   +--------------+      +---------+
   | grammar2code | <--- | grammar |
   +--------------+      +---------+
          |
          v
   +------------+        +---------------------------------------------------+
   | parameters | --+    | irace                                             |
   +------------+   |    |---------------------------------------------------+
                    +--> | irace will search in the design space by testing  |
   +------------+   |    | automatically several parameter instantiations:   |
   | instances  | --+    | +---------------+                                 |
   +------------+        | | parameter     |        +--------------+         |
                         | | instantiation | -----> | grammar2code | <--+    |
                         | +---------------+        +--------------+    |    |
                         |       ^                        |     +---------+  |
                         |       |                        |     | grammar |  |
                         | performance are                v     +---------+  |
                         | measured and used to    +-----------------------+ |
                         | generate the next       | source code           | |
                         | candidate parameter     |-----------------------| |
                         | instantiations          | compiled and executed | |
                         |       |                 | on the training       | |
                         |       +---------------- | instances             | |
                         |                         +-----------------------+ |
                         +---------------------------------------------------+
                                                  |
                                                  v
                                       +---------------------+
                                       | efficient algorithm |
                                       | given the instances |
                                       | in the training set |
                                       +---------------------+
```

For more details about ```grammar2code``` see the following paper:

 *  Franco Mascia, Manuel López-Ibáñez, Jérémie Dubois-Lacoste, and Thomas
    Stützle.
    **Grammar-based Generation of Stochastic Local Search Heuristics Through
    Automatic Algorithm Configuration Tools.**
    Computers & Operations Research, 51(0):190-199, 2014.
    DOI: [10.1016/j.cor.2014.05.020](http://dx.doi.org/10.1016/j.cor.2014.05.020)


Documentation
-------------

#### Grammar XML format: namespaces and special attributes ####

As mentioned above, the grammar is described in an XML format that allows to
define flexibly both the terminals (as CDATA sections), as well as the ways in
which terminals and non-terminals can be combined together. Moreover, in the
same XML file, several grammars can be defined to produce separately different
source files of the program to be derived.

The ```grammar``` namespace in the XML file is used by ```grammar2code``` to
distinguish elements that serve a specific function. For example, the root
node of the xml tree is:

```xml
    <gr:grammar xmlns:gr="grammar">
```

and all derivations are specified inside the element:

```xml     <gr:derivations> ```

You will find these elements in all XML grammars. Grammar files can be merged
together by means of the following element:

```xml
    <gr:include source="general.xml"/>
```

This can be useful in complicated projects, where splitting the derivations
between different grammars allows for better documenting and reuse of the
blocks of code. Since ```grammar2code``` accepts in input only one grammar
file, it is necessary that in such file all derivations from all XML files are
included. Note that the ```gr:include``` element is a child of
```gr:grammar``` and not ```gr:derivations```. Special nodes among the
derivations are the ```copy``` and ```copyall``` elements:

```xml
    <gr:copy source="source/a_file.h" destination="destination/a_file.h" />
    <gr:copyall source_dir="source/" destination_dir="destination/" regex_filter=".*\.h" />
```

the first element tells ```grammar2code``` when it generates an algorithm from
a parameter instantiation to contextually copy a file from a source to a
destination. The second one allows to specify a regular expression to copy
several files in a single command.

The only reserved element name that does not belong to the ```gr``` namespace
is ```<or/>``` and it is used as a separator of alternative elements. As for all
other reserved element names it has to be written in lower case.

There are also special attributes that are interpreted specifically by
```grammar2code``` regardless of the element name or of the namespace. For
example, if a tag has an ```output``` attribute:

```xml
    <start output="hello.c">
```

it means that the code generated by the element should be written into the
file specified in output. Numerical ranges can be specified by means of a
series of attributes:

```xml
    <ps:steps type="int" min="0" max="10" stepIfEnumerated="1" default="5"/>
    <ps:temperature type="real" min="0" max="1" log-scale="true" />
```

the nodes should not have children, the ```type``` attribute specifies if it is
real-valued or integer, while the ```min``` and ```max``` attributes define the
range. If the parameter file format supports it, as in the case of
[ParamILS](http://www.cs.ubc.ca/labs/beta/Projects/ParamILS/) or
[SMAC](http://www.cs.ubc.ca/labs/beta/Projects/SMAC/), the ranges can also have
a default value. If no default is specified, then the min is taken. In the case
of a parameter configuration generated for
[irace](http://iridia.ulb.ac.be/irace/), the default values are not taken in
consideration, this could cahnge in the future when also the categorical and
recursive parameter will have default values, and in the case of
[irace](http://iridia.ulb.ac.be/irace/) an initial candidate configuration will
be provided.  In the case of parameter files generated for
[SMAC](http://www.cs.ubc.ca/labs/beta/Projects/SMAC/), the user can also specify
that the parameter varies on a log scale with the ```log-scale``` parameter.
The attribute ```stepIfEnumerated``` is used during the generation of parameters
for the tools for automatic algorithm configuration, such as
[ParamILS](http://www.cs.ubc.ca/labs/beta/Projects/ParamILS/), that can not deal
directly with real-valued or integer parameters. In such a case,
```temperature``` would be considered as if it was defined as:

```xml
    <ps:temperature>
        <![CDATA[0.0]]]><or/><![CDATA[0.2]]]><or/>
        <![CDATA[0.4]]]><or/><![CDATA[0.6]]]><or/>
        <![CDATA[0.8]]]><or/><![CDATA[1.0]]]>
    </ps:temperature>
```

The ```append``` attribute allows to merge together two derivations (possibly
of two different XML files) into a disjunction of the children of the
elements. For example:

```xml
    <local_search>
        <ls_1/><or/><ls_2/>
    </local_search>

    <local_search append="disjunction">
        <ls_3/>
    </local_search>
```

becomes:

```xml
    <local_search>
        <ls_1/><or/><ls_2/><or/><ls_3/>
    </local_search>
```

#### Generating an irace configuration ####

Every time you execute ```grammar2code``` the full list of parameters is
generated and printed on the terminal. To store it in a file (e.g.,
```tuning/parameters.txt```) and overwrite if it already exists, you have
to run:

```bash
    ./grammar2code grammar.xml --depth=3 --parameters=tuning/parameters.txt
```

#### Generating a ParamILS configuration ####

A parameter configurations for
[ParamILS](http://www.cs.ubc.ca/labs/beta/Projects/ParamILS/) can be generated
with:

```bash
    ./grammar2code grammar.xml --depth=3 --params_format=ParamILS \
                               --parameters=tuning/parameters.txt

```

#### Generating a SMAC configuration ####

A parameter configurations for
[SMAC](http://www.cs.ubc.ca/labs/beta/Projects/SMAC/) can be generated
in a similar way:

```bash
    ./grammar2code grammar.xml --depth=3 --params_format=SMAC \
                               --parameters=tuning/parameters.pcs

```


#### Generating the code ####

To generate the code, it is sufficient to run ```grammar2code``` with the an
instantiation of the list of parameters generated in the previous step.
[irace](http://iridia.ulb.ac.be/irace/) should present a list of
```# Best candidates (as commandlines)``` at the end of its output:

```bash
    ./grammar2code grammar.xml --target_dir=temp_build \
                               --parameter1=value1 --parameter2=value2 ...
```

The parameter instantiations can be passed to ```grammar2code``` also in a
[ParamILS](http://www.cs.ubc.ca/labs/beta/Projects/ParamILS/)-
or [SMAC](http://www.cs.ubc.ca/labs/beta/Projects/SMAC/)-like format:

```bash
    ./grammar2code grammar.xml --target_dir=temp_build \
                               -parameter1 value1 -parameter2 value2 ...
```

All the output files and all the other files to be copied will be written in
the ```code``` target directory. If the code generated has some significant
whitespace (e.g., Python), the ```--do_not_reindent``` option prevents the
automatic removal of the extra leading whitespace in the generated code.

####Replacing derivations####

Sometimes it is handy to specify a second grammar to replace some derivations
in the original one (especially for quickly testing slight variations of the
original grammar). For example a ```test1.xml``` file:

```xml
    <?xml version="1.0" encoding="UTF-8" ?>
    <gr:grammar xmlns:gr="grammar">
        <gr:derivations>
          <local_search>
              <ls_3/>
          </local_search>
        </gr:derivations>
    </gr:grammar>
```

could be used to restrict the possible values assumed by ```<local_search>```
in the original grammar to ```<ls_3>```. Being able to define extensions and
restrictions of the derivations without changing the original grammar allows
to test and keep track of several what-if scenarios.

The second grammar must be specified when generating the parameters:

```bash
    ./grammar2code grammar.xml --overwrite=test1.xml --depth=3 \
                               --parameters=tuning/parameters.txt
```

as well as when generating the code:

```bash
    ./grammar2code grammar.xml --overwrite=test1.xml --target_dir=temp_build \
                               --parameter1=value1 --parameter2=value2 ...
```

Quick-start guide
-----------------

The following is a minimal working example that shows how to define a grammar,
generate a list of  parameters for [irace](http://iridia.ulb.ac.be/irace/),
and then generate the actual code to be compiled.
[irace](http://iridia.ulb.ac.be/irace/) installation is not covered in the
example.

#### Files and directory layout ####

The code in the example will not make anything useful, and is more structured
and complex than needed to illustrate grammar2code features. You can download
it from here: [grammar2code_example.zip](files/grammar2code_example.zip).

*TODO: upload the example directory in the same place with the executables*

In the directory tree below, the ```build``` directory contains the
```grammar2code``` executable (see the section below about requirements and
building) and the ```example``` directory contains the sample grammar and
code:

```
    build/
    example/
        sat.xml
        sls_algorithm.xml
        sat_sources/
            sat.h
        sources/
            Makefile
            main.c
        tuning/
            hook-run
            temp/
            test.py
```

If you downloaded a compiled version of ```grammar2code``` either you move
the executable in a ```build``` directory, or you update the pahts in
```test.py```.

The algorithm building blocks and grammar are defined in ```sat.xml``` and
```sls_algorithm.xml```. The algorithm is split in two files, because
```sls_algorithm.xml``` could contain rules that are useful across different
projects, while ```sat.xml``` could contain problem specific building blocks.
For an example see the paper:

 *  Marie-Eléonore Marmion, Franco Mascia, Manuel López-Ibáñz, and Thomas
    Stützle.
    **Towards the Automatic Design of Metaheuristics.**
    In Hoong Chuin Lau, Günther Raidl, and Pascal Van Hentenryck, editors,
    Proceedings of 10th Metaheuristic International Conference (MIC 2013),
    215-217, Singapore, August 5-8, 2013.
    DOI: [10.1007/978-3-642-38516-2_12](http://dx.doi.org/10.1007/978-3-642-38516-2_12)

 *  Franco Mascia, Manuel López-Ibáñez, Jérémie Dubois-Lacoste, Marie-Éléonore
    Marmion, and Thomas Stützle.
    Algorithm comparison by automatically configurable stochastic local search
    frameworks: a case study using flow-shop scheduling problems.
    In Maria J. Blesa, Christian Blum and Stefan Voß editors, Proceedings of
    Hybrid Metaheuristics 9th International Workshop (HM 2014), Hamburg,
    Germany, June 11-13, 2014, volume 8457 of Lecture Notes in Computer
    Science, pages 30-44. Springer International Publishing, Switzerland 2014.
    DOI: [10.1007/978-3-319-07644-7_3](http://dx.doi.org/10.1007/978-3-319-07644-7_3)

*TODO: update the conferences with the journal (when published)*

#### The example ####

The XML grammar ```sls_algorithm.xml``` contains the derivations that describe
the schema of a simulated annealing algorithm. This file could be extended to
describe problem independent algorithmic building blocks as well as other
stochastic local search algorithms. The grammar generates a ```general.h```
header file that will be included in the main program. The XML grammar
```sat.xml``` describes the problem specific building blocks. This includes
local search operators, and all the files that should be included in the main
program to deal with the specific problem to be optimised. For example,
```sat_sources/sat.h``` contains an (empty) function to load a problem
instance and could as well contain specific data structures for dealing with
the problem representation. This grammar generates a ```specific.h``` header
file. In ```sources/``` there is a ```main.c``` that includes both
```general.h``` and ```specific.h``` and tuns the stochastic local search
algorithm with the building blocks of that have been generated by the two
grammars. It contains also a ```Makefile``` for compiling and automatically
test the generated algorithms. The directory ```tuning``` contains some files
for automatically generate an algorithm with
[irace](http://iridia.ulb.ac.be/irace/). The file ```test.py``` is a helper
script that given a parameter configuration and a target instance, it calls
```grammar2code``` to generate an algorithm in a temporary directory and test
it on the instance given. The file ```hook-run``` is the script that allows
for testing the configurations through ```test.py``` and pass the results back
to [irace](http://iridia.ulb.ac.be/irace/).

To generate the list of parameters from the XML grammar files it is sufficient
to specify the grammar file, the maximum depth for the recursive derivations,
the name of the parameters file, and the directory where the file should be
written:

```sh
    ./build/grammar2code sat.xml --depth=1 --parameters=tuning/parameters.txt
```

From the directory ```tuning``` one can launch the automatic generation and
testing of the algorithms with [irace](http://iridia.ulb.ac.be/irace/)
(you will find how to install the [irace](http://iridia.ulb.ac.be/irace/)
package and how to export the ```IRACE_HOME``` environment variable in
[irace's README file](http://iridia.ulb.ac.be/irace/files/README)). To launch
the tuning locally on a workstation with 16 cores, you can run:

```sh
    $IRACE_HOME/bin/irace --exec-dir=temp --parallel 16
```

Alternatively you can use the tune-main scripts that you find in the
```$IRACE_HOME/examples``` directory. You will find scripts to launch the
tuning locally or alternatively on a cluster using SGE + MPI:

```sh
    tune-main $IRACE_HOME/bin temp --parallel 16
    tune-main-cluster-mpi $IRACE_HOME/bin temp --parallel 50
```

Now, suppose that after the tuning the best configuration is the following one:

```
    --sa-start%ps-step=1
```

In this case to test the configuration on a hypothetical test instance
```instance0.dat``` one can run from the ```tuning``` directory:

```sh
    python test.py sat.xml instance0.dat 1 100 0 --sa-start%ps-step=1
```

Alternatively can generate the code and test it manually:

```bash
    mkdir temp
    ./build/grammar2code sat.xml --target_dir=temp_build --sa-start%ps-step=1
    cd temp
    make
    ./auto_algo instance0.dat 100 0
```

Requirements and building
-------------------------

The code depends on the [Boost](http://www.boost.org) and the
[pugixml](http://pugixml.org) libraries. Pugixml 1.2 is included in the
```3d_party``` directory and compiled along with ```grammar2code```, while
Boost should be installed in the system. The code was tested with Boost 1.53;
```CMakeFiles.txt``` requires at least Boost 1.50 but it could possibly
compile also with earlier versions.

The code makes use of some c++11 constructs and should be compiled with a
modern c++ compiler. It has been tested on GNU/Linux with gcc 4.7.2 and on OS
X with clang 4.2 (LLVM 3.2).

To compile:

```bash
    mkdir build
    cd build
    cmake ..
    make
```

**Note that since ```grammar2code``` is built with c++11 support, also the
boost libraries should be compiled for c++11. It is possible that the one on
your system are compiled for c++03. In this case you will get linker errors
because of binary incompatibilities between the two versions.**

#### Building your version of Boost####

For example on OS X to build your version of boost with support for c++11:

```bash
    tar jxvf boost_1_53_0.tar.bz2
    cd boost_1_53_0
    ./bootstrap.sh
    ./b2 clean
    sudo ./b2 toolset=clang cxxflags="-std=c++11 -stdlib=libc++" linkflags="-stdlib=libc++" install --prefix=/usr/local
    cd ..
    rm -rf boost_1_53_0*
```

if you use brew:

```bash
    brew install --with-c++11 boost
```

On GNU/Linux:

```bash
    tar jxvf boost_1_53_0.tar.bz2
    cd boost_1_53_0
    ./bootstrap.sh
    ./b2 clean
    sudo ./b2 toolset=gcc cxxflags="-std=c++11" install --prefix=/usr/local
    cd ..
    rm -rf boost_1_53_0*
```

#### More than one version of Boost installed ####

If you have a custom version of Boost (because the one installed on your
system is too old), you can specify the location of the more recent version
with the variable ```BOOST_ROOT```. To avoid cmake searching anyway in the
system path you should also set ```Boost_NO_SYSTEM_PATHS``` to true:

```bash
    mkdir build
    cd build
    cmake -DBOOST_ROOT:PATH=... -DBoost_NO_SYSTEM_PATHS:BOOL=ON ..
    make
```

#### Custom compiler ####

To pass custom compilers you can set the ```CC``` and ```CXX``` environmental
variables, with their full path:

```bash
    CC=/usr/local/bin/gcc CXX=/usr/local/bin/g++ cmake ..
```

or define the ```CMAKE_CC_COMPILER``` and ```CMAKE_CXX_COMPILER``` variables.
In this case the full path is not required:

```bash
    cmake -DCMAKE_CXX_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
```
custom compiler flags are set by passing the ```CMAKE_CXX_FLAGS``` variable.

#### Debugging ####

In general to see the command line used when compiling you have to pass the
```VERBOSE``` option to ```make```:

```bash
    make VERBOSE=1
```

and to compile a debug version you have to specify the ```CMAKE_BUILD_TYPE```

```bash
    cmake -DCMAKE_BUILD_TYPE=Debug ..
```

you can also set specific ```CMAKE_CXX_FLAGS_DEBUG``` but the
```CMakeFileLists.txt``` should have already sensible values for
```CMAKE_CXX_FLAGS_DEBUG``` and ```CMAKE_CXX_FLAGS_RELEASE```.

#### Distribution ####

The build type ```distribution``` links the boost library statically and when
building under GNU/Linux it links all libraries statically:

```bash
  cmake -DCMAKE_BUILD_TYPE=distribution ..
```

Extending the code
------------------

The high-level overview below will help to navigate through the code and
extend it if needed.

#### Preprocessing ####

When reading an XML grammar as input, ```grammar2code``` goes through a series
of pre-processing steps aimed at merging and simplifying the grammar
production rules. Then the derivations are simplified through the following
steps:

  1.  if the grammar includes other XML files, those are also parsed, and all
      derivations from different files are merged into the same XML tree;
  2.  empty CDATAs are removed (sometimes there are empty CDATA to define
      non-terminals that should not simplified before appending rules from
      different grammars;
  3.  temporary empty derivation or derivations that contained empty CDATAs are
      removed;
  4.  after removing empty derivations there could be rules with sequences of
      adjacent ```<or/>``` tags, or leading/tailing ```<or/>``` tags;
  5.  some recursive rules are simplified
  6.  rules that contain no actual choice are removed, their content is copied
      wherever the non-terminal was used;
  7.  rules that contain choices are copied directly where such rules were
      used; for the time being this is done in a limited number of cases;
  8.  possible duplicate rules (after merging grammars) are removed; this is
      different the warning that is printed by the class ```configuration```
      when detects two rules having the same derivation but different names;
      having the same rule with different names;
  9.  adjacent CDATAs are merged together;
  10. conjunctions of multiple instantiations of the same rule are renamed, for
      example ```A ::= B C B``` will be translated to ```A ::= B C B2```; in
      fact, each instantiation of rule ```B``` should be assigned to a specific
      parameter and renaming the derivation rule when parsing the model allows
      to have a simpler code for the generation of the parameters and
      conditions;

The preprocessing step number 5 is detailed in the paper:

 *  Franco Mascia, Manuel López-Ibáñez, Jérémie Dubois-Lacoste, and Thomas
    Stützle.
    **Grammar-based Generation of Stochastic Local Search Heuristics Through
    Automatic Algorithm Configuration Tools.**
    Computers & Operations Research, 51(0):190-199, 2014.
    DOI: [10.1016/j.cor.2014.05.020](http://dx.doi.org/10.1016/j.cor.2014.05.020)

the reasoning behind the transformation is explained in:

 *  Franco Mascia, Manue López-Ibáñez, Jérémie Dubois-Lacoste, and Thomas
    Stützle.
    **From Grammars to Parameters: Automatic Iterated Greedy Design for the
    Permutation Flow-shop Problem with Weighted Tardiness.** In Panos Pardalos
    and Giuseppe Nicosia editors, Proceedings of Learning and Intelligent
    Optimization 7th International Conference (LION 7), 321-334, Catania,
    Italy, January 7-11, 2013. Lecture Notes in Computer Science, Springer 2013.
    DOI: [10.1007/978-3-642-44973-4_36](http://dx.doi.org/10.1007/978-3-642-44973-4_36)

For example a rule as the following one ```A ::= B | BA```  can be simplified
into the two non terminals ```BA``` (wherever ```A``` is used) and
```A ::= [] | BA```. This will reduce the number of actual parameters
generated. More complex cases like ```A ::= CB | DBA``` are not simplified.

The typical example with its expansion is the following:

```xml
    <ps:start output="...">
      <ps:A/>
      <![CDATA[ something ... ]]>
      <ps:C/>                                       _
    </ps:start>                                    ⇙ ⇘
    <ps:A>                                        B   B+
      <ps:B/>                                        ⇙ ⇘
      <or/>                                         B   B+
      <ps:B/>                                          ⇙
      <ps:A/>                                         B
    </ps:A>
    <ps:B>
      ...
    </ps:B>
```

In the example ```B``` is both on the left-hand side and on the right-hand
side of the ```<or/>``` symbol and if not properly detected, the derivation
tree (depicted above) will contain both the ```B+``` (```B``` and continue the
recursion) and its duplicate ```B``` in the left branch. If the non terminal
```B``` expands to 10 parameters, ```grammar2code``` will generate around 50
parameters, i.e., 10 for all nodes in the tree.

After the simplification step the grammar will be the following:

```xml
    <ps:start output="...">
      <ps:B/>
      <ps:A/>
      <![CDATA[ something ... ]]>
      <ps:C/>                                       B
    </ps:start>                                    ⇙ ⇘
    <ps:A>                                        []  B
      <![CDATA[ ]]>                                  ⇙ ⇘
      <or/>                                         []  B
      <ps:B/>
      <ps:A/>
    </ps:A>
    <ps:B>
    ...
    </ps:B>
```

The new grammar can be easily mapped to only 30 parameters. The code is
equivalent, but the number of parameters much smaller.

So whenever ```A ::= B | BA``` (or ```A ::= B | AB```, or ```A ::= AB | B```,
etc.) is detected it is replaced by ```A :== [] | BA``` and wherever the
non-terminal ```A```  is used in the grammar it becomes a ```BA```.

The simplified grammar is printed on the terminal at each execution for debug
purposes.

#### Parameter generation ####

<!-- question marks in the CDATA below are U+01C3 and not U+0021 otherwise
     they would have been interpreted when producing the HTML from this
     markdown -->

The class ```walker``` implements a depth first search (DFS) in the XML tree
and recognizes each type of node it is visiting. The nodes are categorised in
the following types:

  - *call* (e.g., ```<D/>```)<br/>
    this is a non-terminal that should be replaced with its definition in the
    list of derivations;

  - *categorical* (e.g., ```<D/><or/><E/><or/><ǃ[CDATA[foo]]>```)<br/>
    this is a node whose alternatives can represented by a categorical
    parameter;

  - *recursive* (e.g., for a node ```<D>```, ```<D/><E/><or/><C/>```)<br/>
    this is a recursive rule, the DFS will keep looping on this node until for
    a number of times specified by the ```depth``` parameter;

  - *range* (e.g., ```<D type="int" min="0" max="100" stepIfEnumerated="1"/>```)<br/>
    this is a rule that will be mapped to a numerical parameter;

  - *copy* (e.g., ```<gr:copy source="problem/init.h" destination="algo/src/init.h"/>```)<br/>
    the rule just lists a file to be copied during the generation of the code;

  - *cdata* (e.g., ```<ǃ[CDATA[bar]]>```)
    a block of code;

  - *plain* (e.g.,)
    node that contains only cdatas or "calls" to derivations; these nodes are
    usually top-level nodes with an output attribute to generate a source
    file.

For each type, call-back functions can be registered (by extending the
```walker``` class) and called to produce, for example, a list of parameters
in the format for a specific tool automatic configuration, or the code of the
algorithm that is derived.

The class ```irace_conf``` does exactly this for
[irace](http://iridia.ulb.ac.be/irace/): it extends ```configuration```, a
subclass of ```walker```. Since the whole process of instantiating and testing
algorithms is automated it is not necessary to give to the parameter
meaningful names. Nevertheless, for easier debugging and easier identification
of the generated algorithms form a list of parameters, the parameter names
correspond to the concatenation of the  rules names and alternatives selected
in the path traversed by the DFS to reach the current node.

In the case of ```irace_conf```, the parameter name of the current node is the
sequences of nodes traversed in the derivation tree along with the values
assumed by such parameters to reach the current node.

For example:

```
    algoselecttype3schedule2temperature  "--algo%selecttype%3%schedule2%temperature=" r (0, 1) | algoselecttype3schedule %in% c(2)
```

indicates the the nodes in the grammar that have been traversed are:
```<algo/>```, ```<selecttype/>``` which has assumed the fourth of its
alternatives, ```<schedule/>``` which has assumed the third of its
alternatives and now the parameter is describing the node
```<temperature type="real" min="0" max="1" stepIfEnumerated="0.1"/>```.

For recursive rules, given a maximum depth *n*, ```irace_conf``` visits the
recursive node *n - 1* times generating all parameters, and one last *n*th
time where the parameter representing the recursive rule does not have among
the possible value to assume, a value that leads to a further recursion. The
number of recursion is also indicated in the parameter name. For example
```--algo@2%3%temperature=``` indicates that the third time that ```<algo/>```
recursive rule is called, the fourth alternative was chosen, which led to the
current parameter representing the ```<temperature/>``` node. In the case of
recursive rules, the name of the parameter does not represent the full path
traversed in the derivation tree. Imagine in the example above that the
alternative for the recursion is the first one:

```xml
    <algo>
      <algo/><![CDATA[foo]]>
      <or/>
      <something/>
      <or/>
      <![CDATA[bar]]>
      <or/>
      <![CDATA[set_temp(]]><temperature/><![CDATA[);]]>
    </algo>
```

the actual path for the the parameter ```<temperature/>``` at the third f
recursion would actually be: ```--algo@0%0algo@1%0algo@2%3%temperature=```.
This is conveniently compressed into the equivalent parameter
```--algo@2%3%temperature=```.

Several recursive rules can be defined in the same grammar but the maximum
depth of the recursion specified by the ```depth``` parameter is the same for
all rules.

Each parameter is generated along with a list of conditional expressions that
help to reduce the design space the tool for automatic algorithm configuration
has to explore. Such conditional expression describe the dependencies between
parameters allowing to consider a parameter only if other parameters assume
specific values that lead to the part of the grammar where the parameter is
actually needed. The last choice in the path traversed by ```grammar2code```
in the DFS up to the current rule is used as conditional expression of the
generated parameter.

Parameters for other tools for automatic algorithm configuration can be
implemented by extending the ```configuration``` class in a similar manner.

#### Code generation ####

The class ```params2code``` will take a list of parameter instantiation and
produce the source code. The class extends the ```walker``` class and
overrides some callbacks to be able to produce the code. It copies the files
as described in the *copy* nodes, it emits the code contained in the *cdata*,
by writing it into the current file as specified by ```output``` attribute of
the last *plain* node. Every time a *recursive*, or *categorical* node is
encountered ```params2code``` looks for the corresponding parameter in the
list of instantiated parameters and prunes the search accordingly. Therefore
when *cdata* or *range* nodes are encountered, the code emitted is the one
that is actually defined by the parameter instantiation.

Other ways to produce the source code (from different representations than the
parameters) can be implemented by extending the ```walker``` class in a
similar manner.
