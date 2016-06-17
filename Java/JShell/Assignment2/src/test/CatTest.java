package test;

import static org.junit.Assert.*;

import java.util.*;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import driver.framework.File;
import driver.framework.FileSystem;
import driver.Exceptions.JShellException;
import driver.command.*;

public class CatTest {

  

  @Before
  public void setUp() throws Exception {
    
    
  }

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
      ArrayList<String> params = new ArrayList<String>();
      Cat test_cat = new Cat();
      test_cat.set_cmd_params(params);
      assertFalse(test_cat.isValid());
  }

  @Test
  public void testRun_one_file() {
    FileSystem test_fs = new FileSystem();
    
    ArrayList<String> params = new ArrayList<String>();
    
    try {
      File test_file = File.create_file(test_fs, "test.txt");
      test_file.set_content("Hello");
      
      params.add("test.txt");
      Cat test_cat = new Cat();
      test_cat.set_cmd_params(params);
      
      assertEquals("test.txt: Hello",test_cat.Run(test_fs));
    } catch (JShellException e) {
      // TODO Auto-generated catch block
      System.out.println(e.get_message());
      fail(e.get_message());
    }
    
  }
  
  @Test
  public void testRun_wrong_file() {
    FileSystem test_fs = new FileSystem();
    
    ArrayList<String> params = new ArrayList<String>();
    
    try {
      File test_file = File.create_file(test_fs, "test.txt");
      test_file.set_content("Hello");
      
      params.add("test1.txt");
      Cat test_cat = new Cat();
      test_cat.set_cmd_params(params);
      
      assertEquals("test1.txt No such file or directory",test_cat.Run(test_fs));
    } catch (JShellException e) {
      // TODO Auto-generated catch block
      System.out.println(e.get_message());
      fail(e.get_message());
    }
    
  }

  @Test
  public void testCat() {
    assertNotNull("Initializer failed to work",new Cat());
  }

}
