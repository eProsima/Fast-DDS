# Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

"""
Sub-Command Validate implementation.

This sub-command validates the Fast DDS XML configuration files using an
XSD schema.
"""

import os
from xml import etree

from xmlschema import XMLSchema, XMLSchemaValidationError


class Validate:
    """Fast DDS XML configuration files validator."""

    def __init__(self, xsd_file):
        """Initialize Validate class."""
        self.xsd_file = xsd_file
        self.__visited_dirs = []

    @property
    def xsd_file(self):
        """XSD schema getter."""
        return self.__xsd_file

    @xsd_file.setter
    def xsd_file(self, xsd_file):
        """XSD schema setter."""
        if not os.path.isfile(xsd_file):
            print(f'The XSD schema does not exist: {xsd_file}')
            exit(1)
        if isinstance(xsd_file, XMLSchema):
            self.__xsd_file = xsd_file
        else:
            try:
                self.__xsd_file = XMLSchema(xsd_file)
            except etree.ElementTree.ParseError as e:
                print(f'cannot load XSD file: {xsd_file}')
                print(e)
                exit(1)

    def run(self, validate_list):
        """
        Run Fast DDS XML configuration files validation.

        :param validate_list: List with the directories and files to validate.
        """
        self.__visited_dirs.clear()
        for element in validate_list:
            element = os.path.abspath(element)
            if os.path.isdir(element):
                self.__validate_xml_from_dir(element)
            elif os.path.isfile(element):
                if self.__is_xml(element):
                    self.__validate_xml_file(element)
                else:
                    print(f'The file is not an XML file: {element}')

    def __validate_xml_from_dir(self, directory, recursive=True):
        """
        Validate XML files from a given directory.

        :param directory: The directory containing the XML files.
        :param recursive: True to do an iterative search for XML files
            through all the directories contained in the given directory.
        """
        for root, dirs, files in os.walk(directory):
            print(f'Scanning directory {root}')
            if root not in self.__visited_dirs:
                self.__visited_dirs.append(root)
                for f in files:
                    if self.__is_xml(f):
                        self.__validate_xml_file(os.path.join(root, f))
                if not recursive:
                    break

    def __validate_xml_file(self, file):
        """
        Validate a single Fast DDS XML configuration file.

        :param file: The Fast DDS XML configuration file.
        """
        xml_etree = etree.ElementTree.parse(file)
        try:
            self.xsd_file.validate(xml_etree)
            print(f'Valid XML file: {file}')
        except XMLSchemaValidationError as error:
            print(f'NOT valid XML file: {file}')
            print(error)

    def __is_xml(self, file):
        """
        Check if a file is an XML file.

        :param file: The input file
        :return: True if the file is an xml file, False if not.
        """
        return os.path.splitext(str(file))[-1] == '.xml'
