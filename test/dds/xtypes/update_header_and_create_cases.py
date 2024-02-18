import os
import sys
import re
import json

class IDLProcessor:
    def __init__(self):
        self.idl_without_typeobjects = {"XtypesTestsTypeNoTypeObject", "declarations", "external"}
        self.struct_info = set()

    def process_idl_files(self, folder_path):
        # Search for .idl files in the specified folder and its subdirectories
        print("Searching for .idl files...")
        for root, dirs, files in os.walk(folder_path):
            for file_name in files:
                if file_name.endswith('.idl'):
                    file_path = os.path.join(root, file_name)
                    print(f"Found .idl file: {file_path}")
                    # Get the relative path of the .idl file from the specified folder
                    idl_file_relative_path = os.path.relpath(file_path, folder_path)
                    # Remove the file extension to get the IDL file name
                    idl_file_name = os.path.splitext(idl_file_relative_path)[0]
                    with open(file_path, 'r') as file:
                        content = file.read()
                        # Find all struct declarations in the IDL file
                        matches = re.findall(r'(?:module\s+(\w+)\s*\{([\s\S]*?)\})|(struct\s+(\w+))', content)
                        for match in matches:
                            module_name, module_content, struct_declaration, struct_name = match
                            if module_name:
                                # Struct declared inside a module
                                structs = re.findall(r'struct\s+(\w+)', module_content)
                                for struct in structs:
                                    # Store struct information along with the IDL file name and module name
                                    self.struct_info.add((struct, idl_file_name, module_name, folder_path))
                            elif struct_name:
                                # Struct declared without a module
                                self.struct_info.add((struct_name, idl_file_name, "", folder_path))


def create_case_files(struct_info):
    # Create 'Cases' folder if it doesn't exist
    cases_folder = os.path.join(os.path.dirname(__file__), 'TypesTestsCases')
    if not os.path.exists(cases_folder):
        os.makedirs(cases_folder)
        print(f"Created 'TypesTestsCases' folder: {cases_folder}")

    # Generate case files for each struct
    print("Creating case files...")
    for struct_name, idl_file_name, _, folder_path in struct_info:
        if folder_path != "BaseCasesIDLs/": # Ignore cases for the IDL files in the 'BaseCasesIDLs' folder
            idl_file_name = idl_file_name.replace("/", "_")  # Replace "/" with "_"
            case_data = {
                "participants": [
                    {
                        "kind": "publisher",
                        "samples": "10",
                        "timeout": "2",
                        "expected_matches": "1",
                        "known_types": [
                            struct_name
                        ]
                    },
                    {
                        "kind": "subscriber",
                        "samples": "10",
                        "timeout": "2",
                        "expected_matches": "1",
                        "known_types": []
                    }
                ]
            }
            file_name = f"Case_{idl_file_name}_{struct_name}.json"
            file_path = os.path.join(cases_folder, file_name)
            with open(file_path, 'w') as f:
                json.dump(case_data, f, indent=4)
            print(f"Created case file: {file_path}")


def update_types_header_file(struct_info):
    # Update types header file with necessary includes
    header_file_path = os.path.join(os.path.dirname(__file__), 'TypeLookupServiceTestsTypes.h')

    with open(header_file_path, 'r') as header_file:
        content = header_file.read()

    # Remove existing include lines
    content = re.sub(r'#include\s+".*?"\n', '', content)

    endif_index = content.rfind("#endif")

    # Add new include lines before the #endif directive
    new_include_lines = set()
    for _, idl_file_name, _, folder_path in struct_info:
        # include_line = f'#include "../../dds-types-test/{idl_file_name}PubSubTypes.h"\n'
        include_line = f'#include "{folder_path}{idl_file_name}PubSubTypes.h"\n'
        new_include_lines.add(include_line)

    content = content[:endif_index].rstrip() + '\n' + '\n' + ''.join(sorted(list(new_include_lines))) + '\n' + content[endif_index:]

    with open(header_file_path, 'w') as header_file:
        header_file.write(content)

    print(f"Header file '{header_file_path}' updated successfully.")


def update_participant_headers_file(file_name, macro_name, struct_info, idl_without_typeobjects):
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
        updated_func = insert_macros(func_declaration.group(1), macro_name, struct_info, idl_without_typeobjects)
        content = content.replace(func_declaration.group(1), updated_func)

    with open(temp_file_path, 'w') as file:
        file.write(content)

    os.replace(temp_file_path, file_path)

    print(f"Type lookup file '{file_name}' updated successfully.")


def insert_macros(func_declaration, macro_name, struct_info, idl_without_typeobjects):
    # Insert macros into the function declaration
    lines = func_declaration.split('\n')
    # Remove all lines starting with the macro_name
    lines = [line for line in lines if not re.match(rf'\s*{macro_name}\((\w+|\w+::\w+)\);', line)]
    updated_func = '\n'.join(lines)
    
    idx = updated_func.rfind('    }')
    if idx != -1:
        # Convert struct_info to a list and sort it alphabetically by idl_file_name and then by struct_name
        struct_info = sorted(list(struct_info), key=lambda x: (x[1], x[2], x[0]))
        for struct_name, idl_file_name, module_name, _ in struct_info:
            if idl_file_name in idl_without_typeobjects:
                if module_name:
                    updated_func = (updated_func[:idx] +
                                    f"        {macro_name}({module_name}::{struct_name});\n"
                                    f"        types_without_typeobject_.insert(\"{struct_name}\");\n" +
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
    folder_path = os.path.join(os.path.dirname(__file__), 'TypesTestsCases')
    if os.path.exists(folder_path) and os.path.isdir(folder_path):
        for filename in os.listdir(folder_path):
            file_path = os.path.join(folder_path, filename)
            try:
                if os.path.isfile(file_path):
                    os.unlink(file_path)
            except Exception as e:
                print(f"Error deleting {file_path}: {e}")


def main():
    if len(sys.argv) != 2:
        print("Usage: python update_header_and_create_cases.py <folder_path to IDL files>")
        print(" <../../../thirdparty/dds-types-test/IDL>")
        return

    folder_path = sys.argv[1]
    if not os.path.exists(folder_path):
        print(f"The directory '{folder_path}' does not exist.")
        return

    processor = IDLProcessor()
    processor.process_idl_files("BaseCasesIDLs/")
    processor.process_idl_files(folder_path)

    if not processor.struct_info:
        print("No structures found in the IDL files.")
        return

    delete_files_in_cases_folder()  # Delete files before creating new ones

    # Perform necessary operations
    create_case_files(processor.struct_info)
    update_types_header_file(processor.struct_info)
    update_participant_headers_file("TypeLookupServicePublisher.h", "PUBLISHER_TYPE_CREATOR_FUNCTION",
                                     processor.struct_info, processor.idl_without_typeobjects)
    update_participant_headers_file("TypeLookupServiceSubscriber.h", "SUBSCRIBER_TYPE_CREATOR_FUNCTION",
                                     processor.struct_info, processor.idl_without_typeobjects)

if __name__ == "__main__":
    main()
