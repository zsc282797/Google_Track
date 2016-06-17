package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.Exceptions.JShellException;
import driver.command.Cp;
import driver.command.Mv;
import driver.framework.Directory;
import driver.framework.File;
import driver.framework.FileSystem;

public class MvTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    
    Mv test_mv = new Mv();
    test_mv.set_cmd_params(test_params);
    assertFalse(test_mv.isValid());
  }

  @Test
  public void testRun_normal() {
    String origin = "/test1.txt";
    String des = "/a/";

    FileSystem test_fs = new FileSystem();
    Directory a = new Directory();
    a.set_dir_name("a");
    test_fs.get_root().add_dir(a);
    
    try {File.create_file(test_fs, "test1.txt");}
    catch(JShellException je){
      fail(je.get_message());
    }
    
    ArrayList<String> test_params = new ArrayList<String>();
    test_params.add(origin);
    test_params.add(des);
    
    Mv test_mv = new Mv();
    test_mv.set_cmd_params(test_params);
    try {test_mv.Run(test_fs);}
    catch(JShellException je){fail(je.get_message());}
    
    assertEquals("test1.txt",test_fs.get_root().get_child().get(0).get_child().get(0).get_dir_names());
    //Test file was correctly moved
    assertEquals(1,test_fs.get_root().get_child().size());
    //Test that original file was deleted and only a was left
    
  }

}
