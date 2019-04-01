package test.com.eprosima.fastrtps;

import org.junit.Test;

import com.eprosima.integration.Command;
import com.eprosima.integration.IDL;
import com.eprosima.integration.TestManager;
import com.eprosima.integration.TestManager.TestLevel;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;

public class FastRTPSGenTest
{
    private static final String INPUT_PATH = "../thirdparty/idl/test/idls";
    private static final String OUTPUT_PATH = "build/test/integration";

    private static boolean isUnix()
    {
        String os = System.getProperty("os.name").toLowerCase();
        return os.contains("nix") || os.contains("nux") || os.contains("aix");
    }

    @Test
    public void runTests()
    {
        if(!isUnix())
        {
            System.out.println("WARNING: The tests are only available with an unix system");
            return;
        }

        // Get client's branch against test will compile.
        String branch = System.getProperty("branch");
        if(branch == null || branch.isEmpty())
        {
            branch = "master";
        }

        //Configure Fast-RTPS for the tests
        ArrayList<String[]> commands = new ArrayList<String[]>();
        commands.add(new String[]{"mkdir -p " + OUTPUT_PATH, "."});
        commands.add(new String[]{"rm -rf Fast-RTPS", OUTPUT_PATH});
        commands.add(new String[]{"git clone -b " + branch + " https://github.com/eProsima/Fast-RTPS.git", OUTPUT_PATH});
        commands.add(new String[]{"mkdir build", OUTPUT_PATH + "/Fast-RTPS"});
        commands.add(new String[]{"cmake .. -DTHIRDPARTY=ON -DCMAKE_INSTALL_PREFIX=install", OUTPUT_PATH + "/Fast-RTPS/build"});
        commands.add(new String[]{"make install", OUTPUT_PATH + "/Fast-RTPS/build"});

        for(String[] command: commands)
        {
            if(!Command.execute(command[0], command[1], true))
            {
                System.exit(-1);
            }
        }

        //Configure idl tests
        TestManager tests = new TestManager(TestLevel.RUN, "share/fastrtps/fastrtpsgen", INPUT_PATH, OUTPUT_PATH + "/idls", "CMake");
        tests.addCMakeArguments("-DCMAKE_PREFIX_PATH=" + System.getProperty("user.dir") + "/" + OUTPUT_PATH + "/Fast-RTPS/build/install");
        //tests.addCMakeArguments("-DCMAKE_PREFIX_PATH=" + "/home/lgasco/git/eProsima/Fast-RTPS/master/build/install");
        tests.removeTests(IDL.ARRAY_NESTED, IDL.SEQUENCE_NESTED);
        boolean testResult = tests.runTests();
        System.exit(testResult ? 0 : -1);
    }
}