#!/bin/bash
#
# Copyright 2013 Produban
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

OLDDIR=`pwd`
BASEDIR=$(readlink -ne `dirname $0`)

mkdir -p $BASEDIR/build/release
cd $BASEDIR/build/release

#CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBUILD_DOC:BOOL=ON"

cmake ../../ ${CMAKE_OPTIONS} 
make

if [ -n "$1" ]; then
	make $1
fi

cd $OLDDIR

