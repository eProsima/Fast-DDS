package com.eprosima.fastrtps.idl.grammar;

import java.util.ArrayList;
import java.util.Stack;

import com.eprosima.idl.parser.tree.Interface;
import com.eprosima.idl.parser.tree.TypeDeclaration;
import com.eprosima.idl.parser.tree.Annotation;
import com.eprosima.idl.parser.typecode.TypeCode;
import com.eprosima.fastrtps.idl.parser.typecode.StructTypeCode;

public class Context extends com.eprosima.idl.context.Context implements com.eprosima.fastcdr.idl.context.Context
{
    // TODO Remove middleware parameter. It is temporal while cdr and rest don't have async functions.
    public Context(String filename, String file, ArrayList includePaths, boolean subscribercode, boolean publishercode,
            String appProduct)
    {
        super(filename, file, includePaths);
        m_fileNameUpper = filename.toUpperCase();
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
        return new StructTypeCode(getScope(), name);
    }

    @Override
    public void addTypeDeclaration(TypeDeclaration typedecl)
    {
        super.addTypeDeclaration(typedecl);

        if(typedecl.getTypeCode().getKind() == TypeCode.KIND_STRUCT && typedecl.isInScope())
        {
            Annotation topicann = typedecl.getAnnotations().get("Topic");

            if(!m_usingTopicAnnotation ||
                    (m_usingTopicAnnotation && topicann != null && topicann.getValue("value").equalsIgnoreCase("true")))
            {
                m_lastStructure = typedecl;

                if(topicann != null)
                    m_usingTopicAnnotation = true;
            }
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

    //// Java block ////
    public void setPackage(String pack)
    {
        if(pack != null && !pack.isEmpty())
        {
            m_package = pack + ".";
            m_packageDir = m_package.replace('.', '/');
        }
    }

    public boolean isIsPackageEmpty()
    {
        return m_package.isEmpty();
    }

    public String getPackage()
    {
        return m_package;
    }

    public String getPackageDir()
    {
        return m_packageDir;
    }

    public String getPackageUnder()
    {
        return m_package.replace('.', '_');
    }
    //// End Java block ////

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
    
    private TypeDeclaration m_lastStructure = null;
    private boolean m_usingTopicAnnotation = false;

	public String getM_lastStructureTopicDataTypeName() {
		String name = new String("");

		if(m_lastStructure!=null)
		{	
            if(m_lastStructure.getParent() instanceof Interface)
			{
				name = name + ((Interface)m_lastStructure.getParent()).getScopedname() + "_" + m_lastStructure.getName();
			}
			else
				name = m_lastStructure.getScopedname();
		}
		return name;
	}
	
	public String getM_lastStructureScopedName(){
		if(m_lastStructure!=null)
		{
			return m_lastStructure.getScopedname();
		}
		return null;
	}
	
	public boolean existsLastStructure()
	{
		if(m_lastStructure != null)
			return true;
		return false;
	}

	private String m_fileNameUpper = null;
	
	public void setFilename(String filename)
    {
		super.setFilename(filename);
		m_fileNameUpper = filename.toUpperCase();
    }
	public String getFileNameUpper()
	{
		return m_fileNameUpper;
	}
	
    //// Java block ////
    // Java package name.
    private String m_package = "";
    // Java package dir.
    private String m_packageDir = "";
    //// End Java block
    
}
