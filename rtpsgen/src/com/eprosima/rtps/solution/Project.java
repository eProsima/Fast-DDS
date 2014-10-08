package com.eprosima.rtps.solution;

import java.util.ArrayList;
import java.util.HashSet;

import com.eprosima.solution.GUIDGenerator;
import com.eprosima.idl.util.Util;

public class Project extends com.eprosima.solution.Project
{
	public Project(String name, String file, HashSet dependencies)
	{
        super(name, file, dependencies);

		m_subscribersrcfiles = new ArrayList();
		m_subscriberincludefiles = new ArrayList();
		m_publishersrcfiles = new ArrayList();
		m_publisherincludefiles = new ArrayList();
		m_projectincludefiles = new ArrayList();
		m_projectsrcfiles = new ArrayList();
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
	
	private boolean m_containsInterfaces = false;
	private ArrayList m_subscribersrcfiles = null;
	private ArrayList m_subscriberincludefiles = null;
	private ArrayList m_publishersrcfiles = null;
	private ArrayList m_publisherincludefiles = null;
	
	private ArrayList m_projectsrcfiles = null;
	private ArrayList m_projectincludefiles = null;
	String m_guid = null;
}
