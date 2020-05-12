#!/bin/bash

##############################################################################
#                              helper functions                              #
##############################################################################

function build {
    pushd build
    rm -rf *
    cmake -DCMAKE_BUILD_TYPE=Distribution $@ ..
    make
    popd
}

function prepare_dir {
    rm -rf grammar2code
    mkdir grammar2code
    cp LICENSE.txt grammar2code/
    cp README.md grammar2code/
    if [ "$1" == "sources" ]; then
        cp CMakeLists.txt grammar2code/
        cp -r 3d_party grammar2code/
        cp -r src grammar2code/
    else
        cp build/grammar2code grammar2code
    fi
}

##############################################################################
#                         build distribution archives                        #
##############################################################################

VERSION=$(cat src/grammar.hpp | grep G2C_VERSION | sed 's/.*"\(.*\)"/\1/')

echo ""
echo -n "Building version ${VERSION}, do you want to update README.md? [Y/n] "
read answer
if [ "$answer" != "n" ]; then
    sed -i .old 's/\(The latest version (```\).*\(```).*\)/\1'${VERSION}'\2/' README.md
    rm README.md.old
fi

echo ""
echo -n "Build sources archive? [Y/n] "
read answer
if [ "$answer" != "n" ]; then
    rm -f distribution/grammar2code_sources.zip
    prepare_dir sources
    zip -mr9 distribution/grammar2code_sources.zip grammar2code -x "*.DS_Store"
fi

echo ""
echo -n "Build example archive? [Y/n] "
read answer
if [ "$answer" != "n" ]; then
    rm distribution/grammar2code_example.zip
    zip -r9 distribution/grammar2code_example.zip example -x "*.DS_Store"
fi


echo ""
echo -n "Build OS X executable? [Y/n] "
read answer
if [ "$answer" != "n" ]; then
    rm -f distribution/grammar2code_osx.zip
    build
    prepare_dir exe
    zip -mr9 distribution/grammar2code_osx.zip grammar2code -x "*.DS_Store"
fi

echo ""
echo -n "Build Linux x86_64 executable? [Y/n] "
read answer
if [ "$answer" != "n" ]; then
    rm -f distribution/grammar2code_linux.tar
    rm -f distribution/grammar2code_linux.tar.bz2
    # building with locally installed boost with support to c++11
    build -DBOOST_ROOT:PATH=~/boost -DBoost_NO_SYSTEM_PATHS:BOOL=ON
    prepare_dir exe
    tar cvf distribution/grammar2code_linux.tar grammar2code --exclude=.DS_Store
    bzip2 -9 distribution/grammar2code_linux.tar
    rm -rf grammar2code
fi

echo ""
echo -n "Commit and tag current version? [Y/n] "
read answer
if [ "$answer" != "n" ]; then
    git pull
    git commit -a -m "Deploying version: ${VERSION}."
    git tag -a v${VERSION} -m "version ${VERSION}"
    echo -n "Pushing to the remote repository? [Y/n] "
    read answer
    if [ "$answer" != "n" ]; then
        git push
        git push --tags
    fi
fi

echo ""
echo -n "Update the website? [Y/n] "
read answer
if [ "$answer" != "n" ]; then
    echo "TO BE IMPLEMENTED..."
fi
