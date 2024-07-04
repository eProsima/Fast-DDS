#!/bin/bash

# Define the root directory containing the folders
root_dir="/home/daniel/Documents/eprosima/dynamic_type_idl/src/fastdds/test/unittest/dds/xtypes/type_conversion/types/type_objects"

# Iterate through each folder in the root directory
for folder in "$root_dir"/*/; do
  # Extract the folder name
  folder_name=$(basename "$folder")

  # Change to the folder
  cd "$folder" || exit

  # Run the commands
  cp "../../idls/${folder_name}.idl" .
  /home/daniel/Documents/fastddsgen/scripts/fastddsgen -replace "${folder_name}.idl"
  rm "${folder_name}.idl"

  # Change back to the root directory
  cd "$root_dir" || exit
done
