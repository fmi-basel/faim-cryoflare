#!/bin/bash
################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 2017-2020 by the CryoFLARE Authors
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3.0 of the License.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with CryoFLARE.  If not, see http://www.gnu.org/licenses/.
#
################################################################################
GIT_HEADER="$2"
GIT_VERSION="`git  --git-dir \"${1}/../../.git\" --work-tree \"${1}/../..\" describe --always --tags`"
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

