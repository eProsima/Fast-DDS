package com.eprosima.fastrtps;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Vector;

import javax.swing.plaf.basic.BasicFormattedTextFieldUI;

import org.antlr.stringtemplate.CommonGroupLoader;
import org.antlr.stringtemplate.StringTemplate;
import org.antlr.stringtemplate.StringTemplateErrorListener;
import org.antlr.stringtemplate.StringTemplateGroup;
import org.antlr.stringtemplate.StringTemplateGroupLoader;
import org.antlr.stringtemplate.language.DefaultTemplateLexer;

import com.eprosima.rtps.exceptions.BadArgumentException;
import com.eprosima.rtps.idl.grammar.Context;
import com.eprosima.rtps.solution.Project;
import com.eprosima.rtps.solution.Solution;
import com.eprosima.rtps.util.Utils;
import com.eprosima.rtps.util.VSConfiguration;
import com.eprosima.idl.generator.manager.TemplateGroup;
import com.eprosima.idl.generator.manager.TemplateManager;
import com.eprosima.idl.parser.grammar.IDLLexer;
import com.eprosima.idl.parser.grammar.IDLParser;
import com.eprosima.idl.parser.tree.Interface;
import com.eprosima.idl.parser.typecode.TypeCode;
import com.eprosima.idl.util.Util;
import com.eprosima.log.ColorMessage;

// TODO: Implement Solution & Project in com.eprosima.rtps.solution

public class fastrtpsgen {
	
	/*
	 * ----------------------------------------------------------------------------------------
	 * 
	 * Attributes
	 */
	
	private static ArrayList<String> m_platforms = null;
	
	private Vector<String> m_idlFiles;
	protected static String m_appEnv = "fastrtpsHOME";
	private String m_exampleOption = null;
	private String m_languageOption = "C++";
    private boolean m_ppDisable = false; //TODO
    private boolean m_replace = false;
    private String m_ppPath = null;
    private final String m_defaultOutputDir = "." + File.separator;
    private String m_outputDir = m_defaultOutputDir;
    private String m_tempDir = null;
    protected static String m_appName = "fastrtpsgen";
    
    private boolean m_publishercode = true;
    private boolean m_subscribercode = true;
    protected static String m_localAppProduct = "fastrtps";
    private ArrayList m_includePaths = new ArrayList();
    
    private String m_command = null;
    private String m_extra_command = null;
    private ArrayList m_lineCommand = null;
    private ArrayList m_lineCommandForWorkDirSet = null;	
    private String m_spTemplate = "main";
    
    private static VSConfiguration m_vsconfigurations[]={new VSConfiguration("Debug DLL", "Win32", true, true),
        new VSConfiguration("Release DLL", "Win32", false, true),
        new VSConfiguration("Debug", "Win32", true, false),
        new VSConfiguration("Release", "Win32", false, false)};
	
	private String m_os = null;
	
	/*
	 * ----------------------------------------------------------------------------------------
	 * 
	 * Constructor
	 */
	
	public fastrtpsgen(String [] args) throws BadArgumentException {
		
		int count = 0;
		String arg;
		
		// Detect OS
		m_os = System.getProperty("os.name");
		
		m_idlFiles = new Vector<String>();
		
		// Check arguments
		while (count < args.length) {
			
			arg = args[count++];
			
			if (!arg.startsWith("-")) {
				m_idlFiles.add(arg);
			} else if (arg.equals("-example")) {
				if (count < args.length) {
					m_exampleOption = args[count++];
					if (!m_platforms.contains(m_exampleOption)) {
						throw new BadArgumentException("Unknown example arch " + m_exampleOption);
					}
				} else {
					throw new BadArgumentException("No architecture speficied after -example argument");
				}
			} else if (arg.equals("-language")) {
				if (count < args.length) {
					m_languageOption = args[count++];
					if (!m_languageOption.equals("C++") && !m_languageOption.equals("c++")) {
						throw new BadArgumentException("Unknown language " + m_languageOption);
					}
				} else {
					throw new BadArgumentException("No language specified after -language argument");
				}
			} else if(arg.equals("-ppPath")) {
				if (count < args.length) {
					m_ppPath = args[count++];
				} else {
					throw new BadArgumentException("No URL specified after -ppPath argument");
				}
			} else if (arg.equals("-ppDisable")) {
				m_ppDisable = true;
			} else if (arg.equals("-replace")) {
				m_replace = true;
			} else if (arg.equals("-d")) {
				if (count < args.length) {
					m_outputDir = Utils.addFileSeparator(args[count++]);
				} else {
					throw new BadArgumentException("No URL specified after -d argument");
				}
			} else if (arg.equals("-version")) {
				showVersion();
				System.exit(0);
			} else if (arg.equals("-help")) {
				printHelp();
				System.exit(0);
			} else { // TODO: More options: -local, -rpm, -debug -I
				throw new BadArgumentException("Unknown argument " + arg);
			}
			
		}
		
		if (m_idlFiles.isEmpty()) {
			throw new BadArgumentException("No input files given");
		}
		
	}
	
	/*
	 * ----------------------------------------------------------------------------------------
	 * 
	 * Listener classes
	 */
	
	class TemplateErrorListener implements StringTemplateErrorListener
    {  
        public void error(String arg0, Throwable arg1)
        {
            System.out.println(ColorMessage.error() + arg0);
            arg1.printStackTrace();
        }

        public void warning(String arg0)
        {
            System.out.println(ColorMessage.warning() + arg0);   
        }   
    }
	
	/*
	 * ----------------------------------------------------------------------------------------
	 * 
	 * Main methods
	 */
	
	public boolean execute() {
		
	
		if (!m_outputDir.equals(m_defaultOutputDir)) {
			File dir = new File(m_outputDir);
			
			if (!dir.exists()) {
				System.out.println(ColorMessage.error() + "The specified output directory does not exist");
				return false;
			}
		}
		
		boolean returnedValue = globalInit();
		
		if (returnedValue) {
			
			Solution solution = new Solution(m_exampleOption, getVersion(), m_publishercode, m_subscribercode);
			
			// Load string templates
			System.out.println("Loading templates...");
			StringTemplateGroupLoader loader = new CommonGroupLoader("com/eprosima/rtps/idl/templates", new TemplateErrorListener());
			StringTemplateGroup.registerGroupLoader(loader);
			
			// Load IDL types for stringtemplates
			TypeCode.idltypesgr = StringTemplateGroup.loadGroup("idlTypes", DefaultTemplateLexer.class, null);
			
			// In local for all products
			//solution.addInclude("$(EPROSIMADIR)/code");
			solution.addInclude("$(" + m_appEnv + ")/include");
			if(m_exampleOption != null) {
				solution.addLibraryPath("$(" + m_appEnv + ")/lib/" + m_exampleOption);
			}
			
			// Protocol FASTCDR
			TypeCode.cpptypesgr = StringTemplateGroup.loadGroup("Types", DefaultTemplateLexer.class, null); //TODOQuitar Types.stg de com.eprosima.rtps.idl.templates y copiarlo con el build
			TemplateManager.middlgr = StringTemplateGroup.loadGroup("eprosima", DefaultTemplateLexer.class, null);
			
			if (m_exampleOption != null && m_exampleOption.contains("Linux")) {
				solution.addLibrary("boost_system");
				solution.addLibrary("boost_thread");
			}
			
			if(m_exampleOption != null && m_exampleOption.contains("Win"))
            {
				solution.addInclude("$(LIB_BOOST_PATH)");
            }
			
			// m_local = true
			//solution.addInclude("$(FAST_BUFFERS)/include");
			//solution.addLibraryPath("$(FAST_BUFFERS)/lib/" + m_exampleOption);
			
			
			if (m_exampleOption != null && !m_exampleOption.contains("Win")) {
				solution.addLibrary("fastcdr");
			}
			
			// Add product library
			solution.addLibrary("fastrtps");
			
			for (int count = 0; returnedValue && (count < m_idlFiles.size()); ++count) {
				Project project = process(m_idlFiles.get(count));
				
				if (project != null) {
					solution.addProject(project);
				} else {
					returnedValue = false;
				}
			}
			
			// Generate solution
			if (returnedValue && m_exampleOption != null) {
				if ((returnedValue = genSolution(solution)) == false) {
					System.out.println(ColorMessage.error() + "While the solution was being generated");
				}
			}
			
		}
		
		return returnedValue;
		
	}
	
	
	
	
	/*
	 * ----------------------------------------------------------------------------------------
	 * 
	 * Auxiliary methods
	 */
	
	public static boolean loadPlatforms() {
		
		boolean returnedValue = false;
		
		fastrtpsgen.m_platforms = new ArrayList<String>();
		
		try {
			
			InputStream input = fastrtpsgen.class.getClassLoader().getResourceAsStream("platforms"); // TODO Modificar esto antes de exportarlo
			InputStreamReader ir = new InputStreamReader(input);
			BufferedReader reader = new BufferedReader(ir);
			String line = null;
			while ((line = reader.readLine()) != null) {
				fastrtpsgen.m_platforms.add(line);
			}
			
			returnedValue = true;
			
		} catch (Exception e) {
			
			System.out.println(ColorMessage.error() + "Getting platforms. " + e.getMessage());
			
		}
		
		return returnedValue;
	}
	
	private String getVersion()
    {
        try
        {
            //InputStream input = this.getClass().getResourceAsStream("/rtps_version.h");
        	
        	InputStream input = this.getClass().getClassLoader().getResourceAsStream("fastrtps_version.h");
            byte[] b = new byte[input.available()];
            input.read(b);
            String text = new String(b);
            int beginindex = text.indexOf("\"");
            int endindex = text.indexOf("\"", beginindex + 1);
            return text.substring(beginindex + 1, endindex);
        }
        catch(Exception ex)
        {
            System.out.println(ColorMessage.error() + "Getting version. " + ex.getMessage());
        }

        return "";
    }
	
	private void showVersion()
    {
        String version = getVersion();
		System.out.println(m_appName + " version " + version);
    }
	
	public static void printHelp()
    {
        System.out.println(m_appName + " usage:");
        System.out.println("\t" + m_appName + " [options] <file> [<file> ...]");
        System.out.println("\twhere the options are:");
        System.out.println("\t\t-help: shows this help");
        System.out.println("\t\t-version: shows the current version of eProsima RTPS.");
		System.out.println("\t\t-example <platform>: Generates a solution for a specific platform (example: x64Win64VS2010)");
        System.out.println("\t\t\tSupported platforms:");
        for(int count = 0; count < m_platforms.size(); ++count)
            System.out.println("\t\t\t * " + m_platforms.get(count));
        System.out.println("\t\t-language <C++>: Programming language (default: C++).");
        System.out.println("\t\t-replace: replaces existing generated files.");
        System.out.println("\t\t-ppDisable: disables the preprocessor.");
        System.out.println("\t\t-ppPath: specifies the preprocessor path.");
        System.out.println("\t\t-d <path>: sets an output directory for generated files.");
		System.out.println("\t\t-t <temp dir>: sets a specific directory as a temporary directory.");
        System.out.println("\tand the supported input files are:");
        System.out.println("\t* IDL files.");
        
    }
	
	public boolean globalInit() {
		String dds_root = null, tao_root = null, fastrtps_root = null;
		
		// Set the temporary folder
		if (m_tempDir == null) {
			if (m_os.contains("Windows")) {
				String tempPath = System.getenv("TEMP");
				
				if (tempPath == null) {
					tempPath = System.getenv("TMP");
				}

				m_tempDir = tempPath;
			} else if (m_os.contains("Linux")) {
				m_tempDir = "/tmp/";
			}
		}
		
		if (m_tempDir.charAt(m_tempDir.length() - 1) != File.separatorChar) {
			m_tempDir += File.separator;
		}
		
		// Set the line command
		m_lineCommand = new ArrayList();
		
		return true;
	}
	
	private Project process(String idlFilename) {
		Project project = null;
		System.out.println("Processing the file " + idlFilename + "...");
		
		try {
			// Protocol CDR
			project = parseIDLtoCDR(idlFilename); // TODO: Quitar archivos copiados TypesHeader.stg, TypesSource.stg, PubSubTypeHeader.stg de la carpeta com.eprosima.rtps.idl.templates
		} catch (Exception ioe) {
			System.out.println(ColorMessage.error() + "Cannot generate the files");
			if (!ioe.getMessage().equals("")) {
				System.out.println(ioe.getMessage());
			}
		} 
		
		return project;
		
	}
	
	private Project parseIDLtoCDR(String idlFilename) {
		boolean returnedValue = false;
		String idlParseFileName = idlFilename;
		Project project = null;
		
		String onlyFileName = Util.getIDLFileNameOnly(idlFilename);
		
		if (!m_ppDisable) {
			idlParseFileName = callPreprocessor(idlFilename);
		}
		
		if (idlParseFileName != null) {
			Context ctx = new Context(onlyFileName, idlFilename, m_includePaths, m_subscribercode, m_publishercode, m_localAppProduct);
			
			// Create template manager
			TemplateManager tmanager = new TemplateManager();
			
			//tmanager.registerRenderer(String.class, new A());

			// Load common types template
			tmanager.addGroup("TypesHeader");
			tmanager.addGroup("TypesSource");
			
			// TODO: Uncomment following lines and create templates
			
			// Load Types common templates
			tmanager.addGroup("RTPSPubSubTypeHeader");
			tmanager.addGroup("RTPSPubSubTypeSource");
			
			// Load Publisher templates
			tmanager.addGroup("RTPSPublisherHeader");
			tmanager.addGroup("RTPSPublisherSource");
			
			// Load Subscriber templates
			tmanager.addGroup("RTPSSubscriberHeader");
			tmanager.addGroup("RTPSSubscriberSource");
			
			// Load PubSubMain template
			tmanager.addGroup("RTPSPubSubMain");
			
			// Create main template
			TemplateGroup maintemplates = tmanager.createTemplateGroup("main");
			maintemplates.setAttribute("ctx", ctx);
			
			try {
				InputStream input = new FileInputStream(idlParseFileName);
				IDLLexer lexer = new IDLLexer(input);
				lexer.setContext(ctx);
				IDLParser parser = new IDLParser(lexer);
				// Pass the finelame without the extension
				returnedValue = parser.specification(ctx, tmanager, maintemplates);
				
			} catch (FileNotFoundException ex) {
				System.out.println(ColorMessage.error("FileNotFounException") + "The File " + idlParseFileName + " was not found.");
			}/* catch (ParseException ex) {
				System.out.println(ColorMessage.error("ParseException") + ex.getMessage());
			}*/ catch (Exception ex) {
				System.out.println(ColorMessage.error("Exception") + ex.getMessage());
			}
			
			if (returnedValue) {
				// Create information of project for solution
				project = new Project(onlyFileName, idlFilename, ctx.getDependencies());
				
				System.out.println("Generating Type definition files...");
				if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + ".h", maintemplates.getTemplate("TypesHeader"), m_replace)) {
					if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + ".cxx", maintemplates.getTemplate("TypesSource"), m_replace)) {
						project.addCommonIncludeFile(onlyFileName + ".h");
						project.addCommonSrcFile(onlyFileName + ".cxx");
					}
				}
				
				// TODO: Uncomment following lines and create templates
				
				System.out.println("Generating generic type files...");
				if (m_exampleOption != null) {
					if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + "PubSubType.h", maintemplates.getTemplate("RTPSPubSubTypeHeader"), m_replace)) {
						if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + "PubSubType.cxx", maintemplates.getTemplate("RTPSPubSubTypeSource"), m_replace)) {
							project.addProjectIncludeFile(onlyFileName + "PubSubType.h");
							project.addProjectSrcFile(onlyFileName + "PubSubType.cxx");
						}
					}
					
					System.out.println("Generating Publisher files...");
					if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + "Publisher.h", maintemplates.getTemplate("RTPSPublisherHeader"), m_replace)) {
						if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + "Publisher.cxx", maintemplates.getTemplate("RTPSPublisherSource"), m_replace)) {
							project.addProjectIncludeFile(onlyFileName + "Publisher.h");
							project.addProjectSrcFile(onlyFileName + "Publisher.cxx");
						}
					}
					
					System.out.println("Generating Subscriber files...");
					if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + "Subscriber.h", maintemplates.getTemplate("RTPSSubscriberHeader"), m_replace)) {
						if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + "Subscriber.cxx", maintemplates.getTemplate("RTPSSubscriberSource"), m_replace)) {
							project.addProjectIncludeFile(onlyFileName + "Subscriber.h");
							project.addProjectSrcFile(onlyFileName + "Subscriber.cxx");
						}
					}
					
					System.out.println("Generating main file...");
					if (returnedValue = Utils.writeFile(m_outputDir + onlyFileName + "PubSubMain.cxx", maintemplates.getTemplate("RTPSPubSubMain"), m_replace)) {
						project.addProjectSrcFile(onlyFileName + "PubSubMain.cxx");
					}
				}
			}
			
		}
		
		return returnedValue ? project : null;
	}
	
	private boolean genSolution(Solution solution) {
		
		final String METHOD_NAME = "genSolution";
		boolean returnedValue = true;
		
		if (m_exampleOption != null) {
			System.out.println("Generating solution for arch " + m_exampleOption + "...");
			
			if (m_exampleOption.substring(3, 6).equals("Win")) {
				System.out.println("Generating Windows solution");
				
				if (m_exampleOption.startsWith("i86")) 
				{
					if(m_exampleOption.charAt(m_exampleOption.length()-1) == '3')
						returnedValue = genVS2013(solution, null);
					else
						returnedValue = genVS2010(solution, null);
				} else if (m_exampleOption.startsWith("x64")) {
					for (int index = 0; index < m_vsconfigurations.length; index++) {
						m_vsconfigurations[index].setPlatform("x64");
					}
					if(m_exampleOption.charAt(m_exampleOption.length()-1) == '3')
						returnedValue = genVS2013(solution, "x64");
					else
						returnedValue = genVS2010(solution, "x64");
				} else {
					returnedValue = false;
				}
			} else if (m_exampleOption.substring(3, 8).equals("Linux")) {
				System.out.println("Generating makefile solution");
				
				if (m_exampleOption.startsWith("i86")) {
					returnedValue = genMakefile(solution, "32");
				} else if (m_exampleOption.startsWith("x64")) {
					returnedValue = genMakefile(solution, "64");
				} else {
					returnedValue = false;
				}
			}
		}
		
		return returnedValue;
	}
	
	private boolean genVS2010(Solution solution, String arch) {
		
		final String METHOD_NAME = "genVS2010";
		boolean returnedValue = false;
		
		StringTemplateGroup vsTemplates = StringTemplateGroup.loadGroup("VS2010", DefaultTemplateLexer.class, null);
		
		if (vsTemplates != null) {
			StringTemplate tsolution = vsTemplates.getInstanceOf("solution");
			StringTemplate tproject = vsTemplates.getInstanceOf("project");
			StringTemplate tprojectFiles = vsTemplates.getInstanceOf("projectFiles");
			StringTemplate tprojectPubSub = vsTemplates.getInstanceOf("projectPubSub");
			StringTemplate tprojectFilesPubSub = vsTemplates.getInstanceOf("projectFilesPubSub");
			
			returnedValue = true;
			
			System.out.println("Proyectos: "+solution.getProjects().size());
			for (int count = 0; returnedValue && (count < solution.getProjects().size()); ++count) {
				Project project = (Project) solution.getProjects().get(count);
				
				tproject.setAttribute("solution", solution);
				tproject.setAttribute("project", project);
				tproject.setAttribute("example", m_exampleOption);
				//tproject.setAttribute("local",  m_local);
				
				tprojectFiles.setAttribute("project", project);
				
				tprojectPubSub.setAttribute("solution", solution);
				tprojectPubSub.setAttribute("project", project);
				tprojectPubSub.setAttribute("example", m_exampleOption);
				
				tprojectFilesPubSub.setAttribute("project", project);
				
				for (int index = 0; index < m_vsconfigurations.length; index++) {
					tproject.setAttribute("configurations", m_vsconfigurations[index]);
					tprojectPubSub.setAttribute("configurations", m_vsconfigurations[index]);
				}
				
				if (returnedValue = Utils.writeFile(m_outputDir + project.getName() + "Types-" + m_exampleOption + ".vcxproj", tproject, m_replace)) {
					if (returnedValue = Utils.writeFile(m_outputDir + project.getName() + "Types-" + m_exampleOption + ".vcxproj.filters", tprojectFiles, m_replace)) {
						if (returnedValue = Utils.writeFile(m_outputDir + project.getName() + "PublisherSubscriber-" + m_exampleOption + ".vcxproj", tprojectPubSub, m_replace)) {
							returnedValue = Utils.writeFile(m_outputDir + project.getName() + "PublisherSubscriber-" + m_exampleOption + ".vcxproj.filters", tprojectFilesPubSub, m_replace);
						}
					}
				}
				
				tproject.reset();
				tprojectFiles.reset();
				tprojectPubSub.reset();
				tprojectFilesPubSub.reset();
				
			}
			
			if (returnedValue) {
				tsolution.setAttribute("solution", solution);
				tsolution.setAttribute("example", m_exampleOption);
				
				// Project configurations
				for (int index = 0; index < m_vsconfigurations.length; index++) {
					tsolution.setAttribute("configurations", m_vsconfigurations[index]);
				}
				
				returnedValue = Utils.writeFile(m_outputDir + "solution-" + m_exampleOption + ".sln", tsolution, m_replace);
			}
			
		} else {
			System.out.println("ERROR<" + METHOD_NAME + ">: Cannot load the template group VS2010");
		}
		
		return returnedValue;
	}
	
	private boolean genVS2013(Solution solution, String arch) {
		
		final String METHOD_NAME = "genVS2013";
		boolean returnedValue = false;
		
		StringTemplateGroup vsTemplates = StringTemplateGroup.loadGroup("VS2013", DefaultTemplateLexer.class, null);
		
		if (vsTemplates != null) {
			StringTemplate tsolution = vsTemplates.getInstanceOf("solution");
			StringTemplate tproject = vsTemplates.getInstanceOf("project");
			StringTemplate tprojectFiles = vsTemplates.getInstanceOf("projectFiles");
			StringTemplate tprojectPubSub = vsTemplates.getInstanceOf("projectPubSub");
			StringTemplate tprojectFilesPubSub = vsTemplates.getInstanceOf("projectFilesPubSub");
			
			returnedValue = true;
			
			System.out.println("Proyectos: "+solution.getProjects().size());
			for (int count = 0; returnedValue && (count < solution.getProjects().size()); ++count) {
				Project project = (Project) solution.getProjects().get(count);
				
				tproject.setAttribute("solution", solution);
				tproject.setAttribute("project", project);
				tproject.setAttribute("example", m_exampleOption);
				//tproject.setAttribute("local",  m_local);
				
				tprojectFiles.setAttribute("project", project);
				
				tprojectPubSub.setAttribute("solution", solution);
				tprojectPubSub.setAttribute("project", project);
				tprojectPubSub.setAttribute("example", m_exampleOption);
				
				tprojectFilesPubSub.setAttribute("project", project);
				
				for (int index = 0; index < m_vsconfigurations.length; index++) {
					tproject.setAttribute("configurations", m_vsconfigurations[index]);
					tprojectPubSub.setAttribute("configurations", m_vsconfigurations[index]);
				}
				
				if (returnedValue = Utils.writeFile(m_outputDir + project.getName() + "Types-" + m_exampleOption + ".vcxproj", tproject, m_replace)) {
					if (returnedValue = Utils.writeFile(m_outputDir + project.getName() + "Types-" + m_exampleOption + ".vcxproj.filters", tprojectFiles, m_replace)) {
						if (returnedValue = Utils.writeFile(m_outputDir + project.getName() + "PublisherSubscriber-" + m_exampleOption + ".vcxproj", tprojectPubSub, m_replace)) {
							returnedValue = Utils.writeFile(m_outputDir + project.getName() + "PublisherSubscriber-" + m_exampleOption + ".vcxproj.filters", tprojectFilesPubSub, m_replace);
						}
					}
				}
				
				tproject.reset();
				tprojectFiles.reset();
				tprojectPubSub.reset();
				tprojectFilesPubSub.reset();
				
			}
			
			if (returnedValue) {
				tsolution.setAttribute("solution", solution);
				tsolution.setAttribute("example", m_exampleOption);
				
				// Project configurations
				for (int index = 0; index < m_vsconfigurations.length; index++) {
					tsolution.setAttribute("configurations", m_vsconfigurations[index]);
				}
				
				returnedValue = Utils.writeFile(m_outputDir + "solution-" + m_exampleOption + ".sln", tsolution, m_replace);
			}
			
		} else {
			System.out.println("ERROR<" + METHOD_NAME + ">: Cannot load the template group VS2013");
		}
		
		return returnedValue;
	}
	
	private boolean genMakefile(Solution solution, String arch) {
		
		boolean returnedValue = false;
		StringTemplate makecxx = null;
		
		StringTemplateGroup makeTemplates = StringTemplateGroup.loadGroup("makefile", DefaultTemplateLexer.class, null);
		
		if (makeTemplates != null) {
			makecxx = makeTemplates.getInstanceOf("makecxx");
			
			makecxx.setAttribute("solution", solution);
			makecxx.setAttribute("example", m_exampleOption);
			makecxx.setAttribute("arch", arch);
			
			returnedValue = Utils.writeFile(m_outputDir + "makefile_" + m_exampleOption, makecxx, m_replace);
			
		}
		
		return returnedValue;
	}
	
	String callPreprocessor(String idlFilename) {
		final String METHOD_NAME = "callPreprocessor";
		
		// Set line command.
		ArrayList lineCommand = new ArrayList();
		String [] lineCommandArray = null;
		String outputfile = Util.getIDLFileOnly(idlFilename) + ".cc";
		int exitVal = -1;
		OutputStream of = null;
		
		// Use temp directory.
		if (m_tempDir != null) {
			outputfile = m_tempDir + outputfile;
		}
		
		if (m_os.contains("Windows")) {
			try {
				of = new FileOutputStream(outputfile);
			} catch (FileNotFoundException ex) {
				System.out.println(ColorMessage.error(METHOD_NAME) + "Cannot open file " + outputfile);
				return null;
			}
		}
		
		// Set the preprocessor path
		String ppPath = m_ppPath;
		
		if (ppPath == null) {
			if (m_os.contains("Windows")) {
				ppPath = "cl.exe";
			} else if (m_os.contains("Linux")) {
				ppPath = "cpp";
			}
		}
		
		// Add command
		lineCommand.add(ppPath);
		
		// Add the include paths given as parameters.
		for (int i=0; i < m_includePaths.size(); ++i) {
			if (m_os.contains("Windows")) {
				lineCommand.add(((String) m_includePaths.get(i)).replaceFirst("^-I", "/I"));
			} else if (m_os.contains("Linux")) {
				lineCommand.add(m_includePaths.get(i));
			}
		}
		
		if (m_os.contains("Windows")) {
			lineCommand.add("/E");
			lineCommand.add("/C");
		}
		
		// Add input file.
		lineCommand.add(idlFilename);
		
		if(m_os.contains("Linux")) {
			lineCommand.add(outputfile);
		}
		
		lineCommandArray = new String[lineCommand.size()];
		lineCommandArray = (String[])lineCommand.toArray(lineCommandArray);
		
		try {
			Process preprocessor = Runtime.getRuntime().exec(lineCommandArray);
			ProcessOutput errorOutput = new ProcessOutput(preprocessor.getErrorStream(), "ERROR", false, null);
			ProcessOutput normalOutput = new ProcessOutput(preprocessor.getInputStream(), "OUTPUT", false, of);
			errorOutput.start();
			normalOutput.start();
			exitVal = preprocessor.waitFor();
			errorOutput.join();
			normalOutput.join();
		} catch (Exception e) {
			System.out.println(ColorMessage.error(METHOD_NAME) + "Cannot execute the preprocessor. Reason: " + e.getMessage());
			return null;
		}
		
		if (of != null) {
			try {
				of.close();
			} catch (IOException e) {
				System.out.println(ColorMessage.error(METHOD_NAME) + "Cannot close file " + outputfile);
			}
			
		}
		
		if (exitVal != 0) {
			System.out.println(ColorMessage.error(METHOD_NAME) + "Preprocessor return an error " + exitVal);
			return null;
		}
		
		return outputfile;
	}
	
	/*
	 * ----------------------------------------------------------------------------------------
	 * 
	 * Main entry point
	 */
	
	public static void main(String[] args) {
		ColorMessage.load();
		
		if(loadPlatforms()) {
			
			try {
				
				fastrtpsgen main = new fastrtpsgen(args);
				if (main.execute()) {
					System.exit(0);
				}
				
			} catch (BadArgumentException e) {
				
				System.out.println(ColorMessage.error("BadArgumentException") + e.getMessage());
                printHelp();
				
			}
			
		}
		
		System.exit(-1);
	}
	
}

class ProcessOutput extends Thread
{
    InputStream is = null;
    OutputStream of = null;
    String type;
    boolean m_check_failures;
    boolean m_found_error = false;
    final String clLine = "#line";

    ProcessOutput(InputStream is, String type, boolean check_failures, OutputStream of)
    {
        this.is = is;
        this.type = type;
        m_check_failures = check_failures;
        this.of = of;
    }

    public void run()
    {
        try
        {
            InputStreamReader isr = new InputStreamReader(is);
            BufferedReader br = new BufferedReader(isr);
            String line=null;
            while ( (line = br.readLine()) != null)
            {
                if(of == null)
                    System.out.println(line);
                else
                {
                    // Sustituir los \\ que pone cl.exe por \
                    if(line.startsWith(clLine))
                    {
                        line = "#" + line.substring(clLine.length());
                        int count = 0;
                        while((count = line.indexOf("\\\\")) != -1)
                        {
                            line = line.substring(0, count) + "\\" + line.substring(count + 2);
                        }
                    }

                    of.write(line.getBytes());
                    of.write('\n');
                }

                if(m_check_failures)
                {
                    if(line.startsWith("Done (failures)"))
                    {
                        m_found_error = true;
                    }
                }
            }
        }
        catch (IOException ioe)
        {
            ioe.printStackTrace();  
        }
    }

    boolean getFoundError()
    {
        return m_found_error;
    }
}
