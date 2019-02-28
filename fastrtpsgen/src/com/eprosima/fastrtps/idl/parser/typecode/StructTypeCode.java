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

        for (int count = 0; count < getMembers().size() && !returnedValue; ++count)
        {
            Member member = getMembers().get(count);
            Annotation key = member.getAnnotations().get("Key");

            if (key != null)
            {
                String value = key.getValue("value");

                if(value != null && value.equals("true"))
                    returnedValue = true;
            }
            else // Try with lower case
            {
                key = member.getAnnotations().get("key");
                if (key != null)
                {
                    String value = key.getValue("value");

                    if(value != null && value.equals("true"))
                        returnedValue = true;
                }
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

    private boolean istopic_ = true;
}
