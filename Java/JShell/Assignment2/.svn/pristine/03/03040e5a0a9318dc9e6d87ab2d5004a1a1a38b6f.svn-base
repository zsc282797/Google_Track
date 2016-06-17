package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.command.Pushd;
import driver.Exceptions.*;
import driver.framework.*;

public class PushdTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    
    Pushd test_pushd = new Pushd();
    test_pushd.set_cmd_params(test_params);
    assertFalse(test_pushd.isValid());
  }

  @Test
  public void testRun() {
    FileSystem test_fs = new FileSystem();
    Directory test_dir = new Directory();
    test_dir.set_dir_name("test_dir");
    
    test_fs.get_cur_dir().add_dir(test_dir);
   
    
   
    ArrayList<String> test_params = new ArrayList<String>();
    test_params.add("test_dir");
    Pushd test_pushd = new Pushd();
    test_pushd.set_cmd_params(test_params);
    
    try {
        test_pushd.Run(test_fs);
    }
    catch(JShellException je){
      fail(je.get_message());
    }
    assertEquals("/",test_fs.get_DS().pop().get_dir_names());
    //test that directory is pushed to the stack correctly
    assertEquals("test_dir",test_fs.get_cur_dir().get_dir_names());
    //test that cur_dir has been changed to the correct one
  }

  @Test
  public void testPushd() {
    assertNotNull("Constructor did not work", new Pushd());
  }

}
