package com.eprosima.fastrtps.idl.grammar;

import java.util.ArrayList;
import java.util.Stack;

import com.eprosima.idl.parser.tree.Interface;
import com.eprosima.idl.parser.typecode.TypeCode;
import com.eprosima.fastrtps.idl.parser.typecode.StructTypeCode;

public class Context extends com.eprosima.idl.context.Context implements com.eprosima.fastcdr.idl.context.Context
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

    @Override
    public StructTypeCode createStructTypeCode(String name)
    {
        StructTypeCode structObject = new StructTypeCode(getScope(), name);
        return structObject;
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

    public boolean isClient()
    {
        return m_subscribercode;
    }
    
    public boolean isServer()
    {
        return m_publishercode;
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
    
    /*** Functions inherated from FastCDR Context ***/

    @Override
    public boolean isPrintexception()
    {
        return false;
    }

    @Override
    public boolean isPrintoperation()
    {
        return false;
    }

    public String getProduct()
    {
        return "fastcdr";
    }

    public String getNamespace()
    {
        return "fastcdr";
    }

    public boolean isCdr()
    {
        return true;
    }

    public boolean isFastcdr()
    {
        return false;
    }

    public boolean isAnyCdr()
    {
        return true;
    }

    /*** End ***/

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
