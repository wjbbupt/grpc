#!/bin/bash
# Copyright 2021 The gRPC Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -ex

ACTION=${1:---overwrite-in-place}
[[ $ACTION == '--overwrite-in-place' ]] || [[ $ACTION == '--diff' ]]

if [[ $ACTION == '--diff' ]]; then
    ACTION="--diff --check"
fi

# Change to root
cd "$(dirname "${0}")/../.."

DIRS=(
    'examples/python'
    'src/python'
    'test'
    'tools'
    'setup.py'
)

VIRTUALENV=venv_isort_code
python3 -m virtualenv $VIRTUALENV
source $VIRTUALENV/bin/activate

python3 -m pip install isort==5.9.2
python3 -m isort $ACTION --settings-path=grpc-style-config.toml --dont-follow-links "${DIRS[@]}"
