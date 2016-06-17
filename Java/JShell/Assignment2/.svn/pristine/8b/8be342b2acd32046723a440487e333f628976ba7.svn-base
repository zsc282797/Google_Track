package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.command.Mkdir;
import driver.Exceptions.*;
import driver.framework.*;

public class MkdirTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    
    Mkdir test_mkdir = new Mkdir();
    test_mkdir.set_cmd_params(test_params);
    assertFalse(test_mkdir.isValid());
  }

  @Test
  public void testRun_normal() {
    FileSystem test_fs = new FileSystem();
    ArrayList<String> test_params = new ArrayList<String>(); 
    test_params.add("test_dir");
    Mkdir test_mkdir = new Mkdir();
    test_mkdir.set_cmd_params(test_params);
    
    try {test_mkdir.Run(test_fs);}
    catch (JShellException je){
      fail(je.get_message());
    }
    assertEquals("test_dir",test_fs.get_cur_dir().get_child().get(0).get_dir_names());
  }
  @Test
  public void testRun_existed(){
    FileSystem test_fs = new FileSystem();
    Directory test_a = new Directory();
    test_a.set_dir_name("test_a");
    test_fs.get_cur_dir().add_dir(test_a);
    
    ArrayList<String> test_params = new ArrayList<String>(); 
    test_params.add("test_a");
    Mkdir test_mkdir = new Mkdir();
    test_mkdir.set_cmd_params(test_params);
    
    try {test_mkdir.Run(test_fs);}
    catch(JShellException je){
      assertEquals("Directory test_a already existed",je.get_message());
    }
    
    
  }
  @Test
  public void testMkdir() {
    assertNotNull("Constructor failed", new Mkdir());
  }

}
