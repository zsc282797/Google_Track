package test;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.util.*;
import driver.command.*;
import driver.Exceptions.*;
import driver.framework.*;

public class CdTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    Cd test_cd = new Cd();
    test_cd.set_cmd_params(test_params);
    assertFalse(test_cd.isValid());
  }

  @Test
  public void testRun_correct_path() {
    FileSystem test_fs = new FileSystem();
    Directory test_a = new Directory();
    ArrayList<String> test_params = new ArrayList<String>();
    test_a.set_dir_name("test_a");
    test_fs.get_cur_dir().add_dir(test_a);
    
    test_params.add("test_a");
    
    Cd test_cd = new Cd();
    test_cd.set_cmd_params(test_params);
    
    try {
      String result = test_cd.Run(test_fs);
    }
    catch (JShellException je){
      fail(je.get_message());
      
    }
    assertEquals("test_a",test_fs.get_cur_dir().get_dir_names());
    
  }
  @Test
  public void testRun_wrong_path() {
    FileSystem test_fs = new FileSystem();
    Directory test_a = new Directory();
    ArrayList<String> test_params = new ArrayList<String>();
    test_a.set_dir_name("testa");
    test_fs.get_cur_dir().add_dir(test_a);
    
    test_params.add("test_a");
    
    Cd test_cd = new Cd();
    test_cd.set_cmd_params(test_params);
    
    try {
      test_cd.Run(test_fs);
    }
    catch (JShellException je){
      assertEquals("No such File or Directory",je.get_message());
      //test that the function will throw the correct exception
    }
    assertEquals("/",test_fs.get_cur_dir().get_dir_names());
    //test that the cur_dir is indeed not changed 
  }
 
  @Test
  public void testCd() {
    assertNotNull("The constructor of Cd did not work !",new Cd());
  }

}
