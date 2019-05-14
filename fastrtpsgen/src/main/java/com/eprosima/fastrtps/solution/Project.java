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

package com.eprosima.fastrtps.solution;

import java.util.ArrayList;
import java.util.LinkedHashSet;

import com.eprosima.solution.GUIDGenerator;
import com.eprosima.idl.util.Util;

public class Project extends com.eprosima.solution.Project
{
    public Project(String name, String file, LinkedHashSet dependencies)
    {
        super(name, file, dependencies);

        m_subscribersrcfiles = new ArrayList();
        m_subscriberincludefiles = new ArrayList();
        m_publishersrcfiles = new ArrayList();
        m_publisherincludefiles = new ArrayList();
        m_projectincludefiles = new ArrayList();
        m_projectsrcfiles = new ArrayList();
        m_jnisrcfiles = new ArrayList<String>();
        m_jniincludefiles = new ArrayList<String>();
        m_idlincludefiles = new ArrayList<String>();
        m_idlincludefiles.addAll((LinkedHashSet<String>)dependencies);
    }

    public void addSubscriberSrcFile(String file)
    {
        m_subscribersrcfiles.add(file);
    }

    public ArrayList getSubscriberSrcFiles()
    {
        return m_subscribersrcfiles;
    }

    public void addSubscriberIncludeFile(String file)
    {
        m_subscriberincludefiles.add(file);
    }

    public ArrayList getSubscriberIncludeFiles()
    {
        return m_subscriberincludefiles;
    }

    public void addPublisherSrcFile(String file)
    {
        m_publishersrcfiles.add(file);
    }

    public ArrayList getPublisherSrcFiles()
    {
        return m_publishersrcfiles;
    }

    public void addPublisherIncludeFile(String file)
    {
        m_publisherincludefiles.add(file);
    }

    public ArrayList getPublisherIncludeFiles()
    {
        return m_publisherincludefiles;
    }

    public void addProjectIncludeFile(String file)
    {
        m_projectincludefiles.add(file);
    }

    public ArrayList getProjectIncludeFiles()
    {
        return m_projectincludefiles;
    }

    public void addProjectSrcFile(String file)
    {
        m_projectsrcfiles.add(file);
    }

    public ArrayList getProjectSrcFiles()
    {
        return m_projectsrcfiles;
    }

    public void addJniSrcFile(String file)
    {
        m_jnisrcfiles.add(file);
    }

    public ArrayList<String> getJniSrcFiles()
    {
        return m_jnisrcfiles;
    }

    public void addJniIncludeFile(String file)
    {
        m_jniincludefiles.add(file);
    }

    public ArrayList<String> getJniIncludeFiles()
    {
        return m_jniincludefiles;
    }

    public boolean getContainsInterfaces()
    {
        return m_containsInterfaces;
    }

    public void setContainsInterfaces(boolean containsInterfaces)
    {
        m_containsInterfaces = containsInterfaces;
    }

    /*!
     * @brief Used in string templates.
     */
    public String getSubscriberGuid()
    {
        return GUIDGenerator.genGUID(getFile() + "Subscriber");
    }

    /*!
     * @brief Used in string templates.
     */
    public String getSubscriberExampleGuid()
    {
        return GUIDGenerator.genGUID(getFile() + "SubscriberExample");
    }

    /*!
     * @brief Used in string templates.
     */
    public String getPublisherGuid()
    {
        return GUIDGenerator.genGUID(getFile() + "Publisher");
    }

    /*!
     * @brief Used in string templates.
     */
    public String getPublisherExampleGuid()
    {
        return GUIDGenerator.genGUID(getFile() + "PublisherExample");
    }

    /*!
     * @brief Used in string templates.
     */
    public String getExampleGuid()
    {
        return GUIDGenerator.genGUID(getFile() + "PubSubExample");
    }

    /*!
     * @brief Used in string templates.
     */
    public String getJniGuid()
    {
        return GUIDGenerator.genGUID(getFile() + "JNI");
    }

    public ArrayList<String> getDependenciesJNIGuids()
    {
        ArrayList<String> deps = new ArrayList<String>(getFullDependencies());
        ArrayList<String> array = new ArrayList<String>();

        for(int count = 0; count < deps.size(); ++count)
        {
            if(!getParent().getOS().contains("Windows") ||
                    getParent().existsProject(deps.get(count)))
            {
                //System.out.println("Adding GUID of " + deps.get(count));
                array.add(GUIDGenerator.genGUID(deps.get(count) + "JNI"));
            }
        }

        return array;
    }

    public boolean getHasStruct()
    {
        return m_hasStruct;
    }
    public void setHasStruct(boolean bol)
    {
        m_hasStruct = bol;
    }

    public void addIDLIncludeFile(String file)
    {
        m_idlincludefiles.add(file);
    }

    public ArrayList<String> getIDLIncludeFiles()
    {
        return m_idlincludefiles;
    }

    private boolean m_containsInterfaces = false;
    private ArrayList m_subscribersrcfiles = null;
    private ArrayList m_subscriberincludefiles = null;
    private ArrayList m_publishersrcfiles = null;
    private ArrayList m_publisherincludefiles = null;

    private ArrayList m_projectsrcfiles = null;
    private ArrayList m_projectincludefiles = null;
    private boolean m_hasStruct = false;
    private ArrayList<String> m_jnisrcfiles = null;
    private ArrayList<String> m_jniincludefiles = null;
    private ArrayList<String> m_idlincludefiles = null;
    String m_guid = null;
}
