package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.Exceptions.JShellException;
import driver.command.Pwd;
import driver.framework.FileSystem;
import driver.framework.*;

public class PwdTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    
    Pwd test_pwd = new Pwd();
    test_pwd.set_cmd_params(test_params);
    assertTrue(test_pwd.isValid());
  }

  @Test
  public void testRun() {
    FileSystem test_fs = new FileSystem();
    Directory test1 = new Directory();
    Directory test2 = new Directory();
    Directory test3 = new Directory();
    
    test1.set_dir_name("Hello");
    test2.set_dir_name("Test");
    test3.set_dir_name("TestTTTTTTTT");
    
    test2.add_dir(test3);
    test1.add_dir(test2);
    test_fs.get_cur_dir().add_dir(test1);
    
    test_fs.set_cur_dir(test3);
    ArrayList<String> test_params = new ArrayList<String>();
    Pwd test_pwd = new Pwd();
    test_pwd.set_cmd_params(test_params);
    
    assertEquals("/Hello/Test/TestTTTTTTTT/",test_pwd.Run(test_fs));
    
  }

  @Test
  public void testPwd() {
    assertNotNull("Constructor failed",new Pwd());
  }

}
