#!/bin/bash

GIT_HEADER="$2"

GIT_VERSION="`git -C \"$1\" describe --always --tags`"
if grep --quiet $GIT_VERSION $GIT_HEADER; then
        echo "No need to generate new $GIT_HEADER - git hash is unchanged"
        exit 0;
fi

echo "git version is:" $GIT_VERSION

echo "#ifndef VERSION_H" > $GIT_HEADER
echo "#define VERSION_H" >> $GIT_HEADER
echo "" >> $GIT_HEADER
echo "#define GIT_VERSION \"$GIT_VERSION\"" >> $GIT_HEADER
echo "#endif //VERSION_H" >> $GIT_HEADER

echo "file is generated into" $GIT_HEADER

