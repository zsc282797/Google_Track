package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.Test;

import driver.Exceptions.JShellException;
import driver.command.Cd;
import driver.command.Grep;
import driver.framework.Directory;
import driver.framework.File;
import driver.framework.FileSystem;

public class GrepTest {

	@Test
	public void test() {
		@Before
		  public void setUp() throws Exception {}

		  @After
		  public void tearDown() throws Exception {}
		  
		  @Test
		  public void testIsValid() {
			  ArrayList<String> testParams = new ArrayList<String>();		  
			  Grep testGrep = new Grep();
			  testGrep.set_cmd_params(testParams);
			  assertFalse(test_man.isValid());
		  }
		  
		  @Test
		  public void testOneLine() {
			  FileSystem fs = new FileSystem();
			  ArrayList<String> testParams = new ArrayList<String>();	
			  File file1 = new File();
			  file1.set_content("hellololo\nboop\bop");
			  testParams.add("hello"); // regex
			  testParams.add("file1");
			    
			  Grep testGrep = new Grep();
			  testGrep.set_cmd_params(testParams);

			  String line = testGrep.Run(fs);
			  
			  assertEquals(line, "hellololo");
			    
        }
		  
		@Test
		public void testLineDeeperInSystem() {
			FileSystem fs = new FileSystem();
			Directory dir1 = new Directory;
			fs.set_cur_dir(dir1);
			File file1 = new File();
			file1.set_content("hello\nhullo");
			dir1.add_dir(file1);
			ArrayList<String> testParams = new ArrayList<String>();	
			testParams.add("hello"); // regex
			testParams.add("/dir1/file1");
			    
			Grep testGrep = new Grep();
			testGrep.set_cmd_params(testParams);

			String line = testGrep.Run(fs);
			  
			assertEquals(line, "hello");
		}
	}

}
