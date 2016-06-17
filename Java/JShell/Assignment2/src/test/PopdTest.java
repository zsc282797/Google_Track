package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.Exceptions.JShellException;
import driver.command.*;
import driver.framework.*;

public class PopdTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    
    Popd test_popd = new Popd();
    test_popd.set_cmd_params(test_params);
    assertTrue(test_popd.isValid());
  }

  @Test
  public void testRun_normal() {
    FileSystem test_fs = new FileSystem();
    Directory test1 = new Directory();
    test1.set_dir_name("test_top");
    test_fs.get_DS().push(test1);
    
    ArrayList<String> test_params = new ArrayList<String>();
    Popd test_popd = new Popd();
    test_popd.set_cmd_params(test_params);
    
    try {test_popd.Run(test_fs);}
    catch(JShellException je){
      fail(je.get_message());
    }
    assertEquals("test_top",test_fs.get_cur_dir().get_dir_names());
  }
  @Test
  public void testRun_outofindex() {
    FileSystem test_fs = new FileSystem();
    Directory test1 = new Directory();
    test1.set_dir_name("test_top");
    test_fs.get_DS().push(test1);
    
    ArrayList<String> test_params = new ArrayList<String>();
    Popd test_popd = new Popd();
    test_popd.set_cmd_params(test_params);
    
    try {test_popd.Run(test_fs);}
    catch(JShellException je){
      fail(je.get_message());
    }
    try {test_popd.Run(test_fs);}
    catch(JShellException je){
      assertEquals("directory stack empty",je.get_message());
    }
    
  }
  
  @Test
  public void testPopd() {
    assertNotNull("Constructor failed",new Popd());
  }

}
