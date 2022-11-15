# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

LINE_TO_FIND=${1}
LOG_FILE=${2}
OUTPUT_FILE=${3}

# Store in OUTPUT_FILE lines in LOG_FILE that contain substring LINE_TO_FIND
# It is done with grep as it is able to do it in a very efficient way, while python
# requires a much longer process
# || true works in case there are no matches
grep "${LINE_TO_FIND}" ${LOG_FILE} > ${OUTPUT_FILE} || true;
