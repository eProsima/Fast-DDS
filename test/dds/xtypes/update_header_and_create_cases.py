import json
import os
import re
import sys
import fnmatch

class IDLProcessor:
    def __init__(self):
        self.structs_info = set()

        # List of files to ignore
        self.files_to_ignore = {
            "external.idl",
            "member_id.idl", # Not support while @autoid(HASH) StructTypeFlag information cannot be pass to DynamicType API.
            "relative_path_include.idl"
        }
        # List of struct names to ignore
        self.struct_names_to_ignore = ["MapWString*", "MapInnerAliasBoundedWStringHelper*", "AnnotatedStruct"]
        # List of IDL files that don't have a TypeObject
        self.idl_without_typeobjects = {"XtypesTestsTypeNoTypeObject", "declarations", "external"}

    def extract_structures(self, idl_text):
        # Regular expressions for module and struct extraction
        module_regexp = r'module\s+(\w+)\s*{((?:.|[\r\n])*?)\};'
        struct_regexp = r'struct\s+(\w+)(\s*:\s*\w+)?\s*\{([^}]+)'

        # Extract structures within modules
        modules = re.findall(module_regexp, idl_text, re.DOTALL)
        module_structures = {}
        for module_match in modules:
            module_name, module_content = module_match
            structures = re.findall(struct_regexp, module_content)
            module_structures[module_name] = structures

        # Find structures outside modules
        outside_structures = re.findall(struct_regexp, idl_text)

        # Remove duplicate structures
        for module_name, structures in module_structures.items():
            for structure in structures:
                if structure in outside_structures:
                    outside_structures.remove(structure)

        return module_structures, outside_structures


    def process_idl_files(self, idls_path):
        # Search for .idl files in the specified folder and its subdirectories
        print("Searching for .idl files...")
        for root, dirs, files in os.walk(idls_path):
            for file_name in files:
                if file_name.endswith('.idl') and file_name not in self.files_to_ignore:
                    file_path = os.path.join(root, file_name)
                    print(f"Found .idl file: {file_path}")
                    idl_file_relative_path = os.path.relpath(file_path, idls_path)
                    idl_file_name = os.path.splitext(idl_file_relative_path)[0]
                    with open(file_path, 'r') as file:
                        content = file.read()
                        module_structures, outside_structures = self.extract_structures(content)

                        # Store struct information along with the IDL file name and module name
                        for module_name, structures in module_structures.items():
                            for structure in structures:
                                self.structs_info.add((structure[0], idl_file_name, module_name, idls_path))

                        # Store struct information for structures outside modules
                        for structure in outside_structures:
                            self.structs_info.add((structure[0], idl_file_name, "", idls_path))


def create_case_files(structs_info, struct_names_to_ignore):
    # Create 'Cases' folder if it doesn't exist
    cases_folder = os.path.join(os.path.dirname(__file__), 'TypesTestsCases')
    if not os.path.exists(cases_folder):
        os.makedirs(cases_folder)
        print(f"Created 'TypesTestsCases' folder: {cases_folder}")

    # Aggregate test cases for each IDL file
    idl_test_cases = {}
    for struct_name, idl_file_name, module_name, idls_path in structs_info:
        if idls_path != "BaseCasesIDLs/":
            if idl_file_name not in idl_test_cases:
                idl_test_cases[idl_file_name] = []

            if not any(fnmatch.fnmatch(struct_name, pattern) for pattern in struct_names_to_ignore):
                known_types = [struct_name]
                if module_name:
                    known_types = [f"{module_name}::{struct_name}"]

                idl_test_cases[idl_file_name].append({
                    "TestCase": f"Case_{idl_file_name}_{struct_name}",
                    "participants": [
                        {
                            "kind": "publisher",
                            "samples": "10",
                            "timeout": "10",
                            "expected_matches": "1",
                            "known_types": known_types
                        },
                        {
                            "kind": "subscriber",
                            "samples": "10",
                            "timeout": "10",
                            "expected_matches": "1",
                            "known_types": []
                        }
                    ]
                })

    # Generate case files for each IDL file
    print("Creating case files...")
    for idl_file_name, test_cases in idl_test_cases.items():
        idl_file_name = idl_file_name.replace("/", "_")
        test_cases.sort(key=lambda x: x["TestCase"])
        case_data = {"test_cases": test_cases}
        file_name = f"Cases_{idl_file_name}.json"
        file_path = os.path.join(cases_folder, file_name)
        with open(file_path, 'w') as f:
            json.dump(case_data, f, indent=4)
        print(f"Created case file: {file_path}")


def update_types_header_file(structs_info, typecode_path):
    # Update types header file with necessary includes
    header_file_path = os.path.join(os.path.dirname(__file__), 'TypeLookupServiceTestsTypes.h')

    with open(header_file_path, 'r') as header_file:
        content = header_file.read()

    # Remove existing include lines
    content = re.sub(r'#include\s+".*?"\n', '', content)

    endif_index = content.rfind("#endif")

    # Add new include lines before the #endif directive
    new_include_lines = set()
    for _, idl_file_name, _, idls_path in structs_info:
        if idls_path != "BaseCasesIDLs/":
            include_line = f'#include "{typecode_path}{idl_file_name}PubSubTypes.hpp"\n'
            new_include_lines.add(include_line)
        else:
            include_line = f'#include "BaseCasesIDLs/{idl_file_name}PubSubTypes.hpp"\n'
            new_include_lines.add(include_line)

    content = content[:endif_index].rstrip() + '\n' + '\n' + ''.join(sorted(list(new_include_lines))) + '\n' + content[endif_index:]

    with open(header_file_path, 'w') as header_file:
        header_file.write(content)

    print(f"Header file '{header_file_path}' updated successfully.")


def update_participant_headers_file(file_name, macro_name, structs_info, struct_names_to_ignore, idl_without_typeobjects):
    # Update participant header file with necessary macros and type information
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(script_dir, file_name)
    temp_file_path = file_path + ".tmp"

    with open(file_path, 'r') as file:
        content = file.read()

    # Remove existing types_without_typeobject_ insertions
    content = re.sub(r'        types_without_typeobject_.insert\("[^"]+"\);\n', '', content)

    # Find the function declaration where macros need to be inserted
    func_declaration = re.search(r'(void\s+create_type_creator_functions\(\)\s*{[\s\S]*?})', content)
    if func_declaration:
        updated_func = insert_macros(func_declaration.group(1), macro_name, structs_info, struct_names_to_ignore, idl_without_typeobjects)
        content = content.replace(func_declaration.group(1), updated_func)

    with open(temp_file_path, 'w') as file:
        file.write(content)

    os.replace(temp_file_path, file_path)

    print(f"Type lookup file '{file_name}' updated successfully.")


def insert_macros(func_declaration, macro_name, structs_info, struct_names_to_ignore, idl_without_typeobjects):
    # Insert macros into the function declaration
    lines = func_declaration.split('\n')
    lines = [line for line in lines if not re.match(rf'\s*{macro_name}\((\w+|\w+::\w+)\);', line)]  # Remove existing macro lines
    updated_func = '\n'.join(lines)

    idx = updated_func.rfind('    }')
    if idx != -1:
        structs_info = sorted(list(structs_info), key=lambda x: (x[1], x[2], x[0]), reverse=True)  # Sort structs_info alphabetically
        for struct_name, idl_file_name, module_name, _ in structs_info:
            if not any(fnmatch.fnmatch(struct_name, pattern) for pattern in struct_names_to_ignore):
                if idl_file_name in idl_without_typeobjects:
                    if module_name:
                        updated_func = (updated_func[:idx] +
                                        f"        {macro_name}({module_name}::{struct_name});\n"
                                        f"        types_without_typeobject_.insert(\"{module_name}__{struct_name}\");\n" +
                                        updated_func[idx:])
                    else:
                        updated_func = (updated_func[:idx] +
                                        f"        {macro_name}({struct_name});\n"
                                        f"        types_without_typeobject_.insert(\"{struct_name}\");\n" +
                                        updated_func[idx:])
                else:
                    if module_name:
                        updated_func = (updated_func[:idx] +
                                        f"        {macro_name}({module_name}::{struct_name});\n" +
                                        updated_func[idx:])
                    else:
                        updated_func = (updated_func[:idx] +
                                        f"        {macro_name}({struct_name});\n" +
                                        updated_func[idx:])

    return updated_func


def delete_files_in_cases_folder():
    # Delete existing case files
    cases_folder_path = os.path.join(os.path.dirname(__file__), 'TypesTestsCases')
    if os.path.exists(cases_folder_path) and os.path.isdir(cases_folder_path):
        for filename in os.listdir(cases_folder_path):
            file_path = os.path.join(cases_folder_path, filename)
            try:
                if os.path.isfile(file_path):
                    os.unlink(file_path)
            except Exception as e:
                print(f"Error deleting {file_path}: {e}")


def main():
    if len(sys.argv) != 3:
        print("This script updates the header files and creates cases files to test the BasesCasesIDL and all the structures found in the IDL files in a given path.")
        print("Usage: update_header_and_create_cases.py <path to IDL files> <path to PubSubTypes files>")
        print("Example: python3 update_header_and_create_cases.py ../../../thirdparty/dds-types-test/IDL/ ../../dds-types-test/")
        return

    idls_path = sys.argv[1]
    if not os.path.exists(idls_path):
        print(f"The directory '{idls_path}' does not exist.")
        return
    typecode_path = sys.argv[2]
    if not os.path.exists(typecode_path):
        print(f"The directory '{typecode_path}' does not exist.")
        return

    processor = IDLProcessor()
    processor.process_idl_files("BaseCasesIDLs/")
    processor.process_idl_files(idls_path)

    if not processor.structs_info:
        print("No structures found in the IDL files.")
        return

    delete_files_in_cases_folder()

    create_case_files(processor.structs_info, processor.struct_names_to_ignore)
    update_types_header_file(processor.structs_info, typecode_path)
    update_participant_headers_file("TypeLookupServicePublisher.h", "PUBLISHER_TYPE_CREATOR_FUNCTION",
                                     processor.structs_info, processor.struct_names_to_ignore, processor.idl_without_typeobjects)
    update_participant_headers_file("TypeLookupServiceSubscriber.h", "SUBSCRIBER_TYPE_CREATOR_FUNCTION",
                                     processor.structs_info, processor.struct_names_to_ignore, processor.idl_without_typeobjects)
    

if __name__ == "__main__":
    main()
