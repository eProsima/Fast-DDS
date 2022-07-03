To launch this test open two different consoles:

In the first one launch: ./DDSHelloWorldExample publisher (or DDSHelloWorldExample.exe publisher on windows).
In the second one: ./DDSHelloWorldExample subscriber (or DDSHelloWorldExample.exe subscriber on windows).

In order to use xml profiles (--env or shorcut -e cli flags):
    - reference the xml profiles file setting the environment variable FASTRTPS_DEFAULT_PROFILES_FILE to its path.
    - called it DEFAULT_FASTRTPS_PROFILES.xml and make sure the file is besides the DDSHelloWorldExample binary.
The profile loaded will be the mark as default one with the corresponding attribute. For example:

    <?xml version="1.0" encoding="UTF-8" ?>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <participant profile_name="name_is_mandatory" is_default_profile="true">
            <rtps>
                <name>Example dummy name</name>
            </rtps>
        </participant>
    </profiles>

will create a participant called "Example dummy name". Note the "profile_name" attribute is mandatory.
