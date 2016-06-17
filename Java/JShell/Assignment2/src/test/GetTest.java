package test;

import static org.junit.Assert.*;

import java.awt.List;
import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.Exceptions.JShellException;
import driver.command.Cd;
import driver.command.Get;
import driver.command.Man;
import driver.framework.Directory;
import driver.framework.FileSystem;

public class GetTest {

	@Test
	public void test() {
		
		@Before
		  public void setUp() throws Exception {}

		  @After
		  public void tearDown() throws Exception {}
		  
		  @Test
		  public void testValidGet() {
	          FileSystem fs = new FileSystem();
			  ArrayList<String> test_params = new ArrayList<String>();  
			  test_params.add("http://www.cs.cmu.edu/~spok/grimmtmp/073.txt");
			    
			  Get testGet = new Get();
			  testGet.set_cmd_params(testParams);
			  testGet.Run(fs);
			  
			  assertFalse(fs.get_cur_dir().get_child().isEmpty());			    
			  }
		  
		  @Test
		  public void testInvalidGet() {
	          FileSystem fs = new FileSystem();
			  ArrayList<String> test_params = new ArrayList<String>();  
			  test_params.add("google.ca");
			    
			  Get testGet = new Get();
			  testGet.set_cmd_params(testParams);
			  testGet.Run(fs);
			  
			  assertTrue(fs.get_cur_dir().get_child().isEmpty());			    
			  }
		  
		  @Test
		  public void testValidContent() {
	          FileSystem fs = new FileSystem();
			  ArrayList<String> test_params = new ArrayList<String>();  
			  test_params.add("http://www.cs.cmu.edu/~spok/grimmtmp/073.txt");
			    
			  Get testGet = new Get();
			  testGet.set_cmd_params(testParams);
			  testGet.Run(fs);
			  
			  ArrayList<String> lines = 
			      fs.get_cur_dir().get_child().get(0).get_content.split("\n");
			  String firstLine = lines.get(0);
			  
			  assertTrue(firstLine == "There was once a king who had an "
			  		+ "illness, and no one believed that he");			    
			  }
		  
		  @Test
		  public void testValidFileName() {
	          FileSystem fs = new FileSystem();
			  ArrayList<String> test_params = new ArrayList<String>();  
			  test_params.add("http://www.cs.cmu.edu/~spok/grimmtmp/073.txt");
			    
			  Get testGet = new Get();
			  testGet.set_cmd_params(testParams);
			  testGet.Run(fs);
			  
			  ArrayList<String> dirName = 
			      fs.get_cur_dir().get_child().get(0).get_dir_names();
			  
			  assertTrue(dirName == "073.txt");			    
			  }
		  
		  @Test
		  public void testIsValidFalse() {
		    ArrayList<String> testParams = new ArrayList<String>();		  
		    Get testGet = new Get();
		    testGet.set_cmd_params(testParams);
		    assertFalse(testGet.isValid());
		  }
		  
		  @Test
		  public void testIsValidTrue() {
		    ArrayList<String> testParams = new ArrayList<String>();	
		    testParams.add("www.google.ca");
		    Get testGet = new Get();
		    testGet.set_cmd_params(testParams);
		    assertTrue(testGet.isValid());
		  }
	}

}
