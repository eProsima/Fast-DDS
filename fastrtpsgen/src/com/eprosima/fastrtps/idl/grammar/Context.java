package com.eprosima.fastrtps.idl.grammar;

import java.util.ArrayList;
import java.util.Stack;

import com.eprosima.idl.parser.tree.Interface;
import com.eprosima.idl.parser.typecode.TypeCode;

public class Context extends com.eprosima.idl.context.Context
{
    // TODO Remove middleware parameter. It is temporal while cdr and rest don't have async functions.
    public Context(String filename, String file, ArrayList includePaths, boolean subscribercode, boolean publishercode,
            String appProduct)
    {
        super(filename, file, includePaths);
        
        
        m_subscribercode = subscribercode;
        m_publishercode = publishercode;
        m_randomGenNames = new Stack<String>();

        // TODO Remove
        m_appProduct = appProduct;
        //m_protocol = protocol;
        //m_ddstypes = ddstypes;
    }
    
    public void setTypelimitation(String lt)
    {
        m_typelimitation = lt;
    }

    public String getTypelimitation()
    {
        return m_typelimitation;
    }
    
    /*!
     * @brief This function adds a global typecode to the context.
     */
    public void addTypeCode(String name, TypeCode typecode)
    {
        super.addTypeCode(name, typecode);
        
        // TODO: Exception.
        if(typecode.getKind() == TypeCode.KIND_STRUCT && isInScopedFile()) {
        	m_lastStructure = name;
        }
    }

    /*!
     * @brief This function is used to know if a project has to generate the Types.
     */
    public boolean isProjectNeedTypes()
    {
    	com.eprosima.idl.parser.tree.Exception ex = null;
    	
    	if((ex = getFirstException()) != null)
    		return true;
    	
    	return false;
    }
    
    /*!
     * @brief This function is used in this project to get the first discovered interface.
     */
    public Interface getFirstInterface()
    { 
        for(int count = 0; m_firstinterface == null && count < getDefinitions().size(); ++count)
        {
            m_firstinterface = getDefinitions().get(count).getFirstInterface(getScopeFile());
        }
        
        return m_firstinterface;
    }
    
    // TODO Ver si es necesario.
    /*!
     * @brief This function is used in this project to get the first discovered exception.
     */
    public com.eprosima.idl.parser.tree.Exception getFirstException()
    {
    	for(int count = 0; m_firstexception == null && count < getDefinitions().size(); ++count)
        {
    		m_firstexception = getDefinitions().get(count).getFirstException(getScopeFile());
        }
        
        return m_firstexception;
    }
    
    public boolean isClient()
    {
        return m_subscribercode;
    }
    
    public boolean isServer()
    {
        return m_publishercode;
    }

    // TODO Remove
    public String getProduct()
    {
        return m_appProduct;
    }
    
    public boolean isAnyCdr()
    {
        return true;
    }

    public boolean isCdr()
    {
        return true;
    }
    public boolean isKey()
    {
    	return true;
    }
    public boolean isFastrpcProduct()
    {
        return false;
    }

    // TODO Para stringtemplate TopicsPlugin de nuestros tipos DDS.
    public String getNewRandomName()
    {
        String name = "type_" + ++m_randomGenName;
        m_randomGenNames.push(name);
        return name;
    }

    public String getNewLoopVarName()
    {
        m_loopVarName = 'a';
        return Character.toString(m_loopVarName);
    }

    public String getNextLoopVarName()
    {
        return Character.toString(++m_loopVarName);
    }

    // TODO Para stringtemplate TopicsPlugin de nuestros tipos DDS.
    public String getLastRandomName()
    {
        return m_randomGenNames.pop();
    }

    private String m_typelimitation = null;
    
    //! Cache the first interface.
    private Interface m_firstinterface = null;
    //! Cache the first exception.
    private com.eprosima.idl.parser.tree.Exception m_firstexception = null;

    // TODO Lleva la cuenta de generación de nuevos nombres.
    private int m_randomGenName = 0;
    private Stack<String> m_randomGenNames = null;
    // TODO Lleva la cuenta del nombre de variables para bucles anidados.
    private char m_loopVarName = 'a';
    
    // Stores if the user will generate the client source.
    private boolean m_subscribercode = true;
    // Stores if the user will generate the server source.
    private boolean m_publishercode = true;

    // TODO Remove
    private String m_appProduct = null;
    
    private String m_lastStructure = null;

	public String getM_lastStructure() {
		return m_lastStructure;
	}

	public void setM_lastStructure(String m_lastStructure) {
		this.m_lastStructure = m_lastStructure;
	}
    
}
