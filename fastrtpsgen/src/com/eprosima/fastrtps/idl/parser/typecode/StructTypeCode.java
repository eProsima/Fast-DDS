package com.eprosima.fastrtps.idl.parser.typecode;

import com.eprosima.idl.parser.typecode.Member;
import com.eprosima.idl.parser.tree.Annotation;

public class StructTypeCode extends com.eprosima.idl.parser.typecode.StructTypeCode
{
    public StructTypeCode(String scope, String name)
    {
        super(scope, name);
    }

    public boolean isHasKey()
    {
        boolean returnedValue = false;
        
        for(int count = 0; count < getMembers().size() && !returnedValue; ++count)
        {
            Member member = getMembers().get(count);
            Annotation key = member.getAnnotations().get("Key");

            if(key != null)
            {
                String value = key.getValue("value");

                if(value != null && value.equals("true"))
                    returnedValue = true;
            }
        }

        return returnedValue;
    }

    public void setIsTopic(boolean value)
    {
        istopic_ = value;
    }

    public boolean isIsTopic()
    {
        return istopic_;
    }

    private boolean istopic_ = false;
}
