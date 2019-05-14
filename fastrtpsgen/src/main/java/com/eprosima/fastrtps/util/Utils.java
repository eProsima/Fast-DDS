// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.eprosima.fastrtps.util;

import org.antlr.stringtemplate.StringTemplate;

import java.io.*;

public class Utils
{

    public static String getWADLFileNameOnly(String wadlFilename)
    {
        int index = -1;
        String auxString = wadlFilename, returnedValue = null;

        index = wadlFilename.lastIndexOf(File.separator);

        if(index == -1)
            index = wadlFilename.lastIndexOf('/');

        if(index != -1)
            auxString = wadlFilename.substring(index + 1);

        // Remove '.wadl'
        returnedValue = auxString.substring(0, auxString.length() - 5);

        return returnedValue;
    }

    public static String getFileNameOnly(String fileName)
    {
        int index = -1;
        String auxString = fileName, returnedValue = null;

        index = fileName.lastIndexOf(File.separator);

        if(index == -1)
            index = fileName.lastIndexOf('/');

        if(index != -1)
            auxString = fileName.substring(index + 1);

        // Remove extension
        index = auxString.lastIndexOf('.');
        if(index != -1)
            auxString = auxString.substring(0, index);

           returnedValue = auxString;

        return returnedValue;
    }

    public static String addFileSeparator(String directory)
    {
        String returnedValue = directory;

        if(directory.charAt(directory.length() - 1) != File.separatorChar ||
                directory.charAt(directory.length() - 1) != '/')
            returnedValue = directory + File.separator;

        return returnedValue;
    }

    public static boolean writeFile(String file, StringTemplate template, boolean replace)
    {
        boolean returnedValue = false;

        try
        {
            File handle = new File(file);

            if(!handle.exists() || replace)
            {
                FileWriter fw = new FileWriter(file);
                String data = template.toString();
                fw.write(data, 0, data.length());
                fw.close();
            }
            else
            {
                System.out.println("INFO: " + file + " exists. Skipping.");
            }

            returnedValue = true;
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        return returnedValue;
    }

    public static String getFileExtension(String fileName)
	{
	    int lastDot = fileName.lastIndexOf(".");

	    return fileName.substring(lastDot+1);
	}
}
