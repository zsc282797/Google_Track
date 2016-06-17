package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.command.Cp;
import driver.Exceptions.*;
import driver.framework.*;
public class CpTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    Cp test_cp = new Cp();
    test_cp.set_cmd_params(test_params);
    assertFalse(test_cp.isValid());
  }

  @Test
  public void testRun_correct_paths() {
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
    
    Cp test_cp = new Cp();
    test_cp.set_cmd_params(test_params);
    try {test_cp.Run(test_fs);}
    catch(JShellException je){fail(je.get_message());}
    
    assertEquals("test1.txt",test_fs.get_root().get_child().get(0).get_child().get(0).get_dir_names());
  }
  @Test
  public void testRun_wrong_des() {
    String origin = "/test1.txt";
    String des = "/b/";

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
    
    Cp test_cp = new Cp();
    test_cp.set_cmd_params(test_params);
    try {test_cp.Run(test_fs);}
    catch(JShellException je){
      assertEquals("/b/: No such File or Directory",je.get_message());
      }
    
    
  }

  @Test
  public void testRun_file_exists() {
    String origin = "/test1.txt";
    String des = "/a/";

    FileSystem test_fs = new FileSystem();
    Directory a = new Directory();
    a.set_dir_name("a");
    test_fs.get_root().add_dir(a);
    
    try {File.create_file(test_fs, "test1.txt");
        File.create_file(test_fs, "/a/test1.txt");}
    catch(JShellException je){
      fail(je.get_message());
    }
    
    ArrayList<String> test_params = new ArrayList<String>();
    test_params.add(origin);
    test_params.add(des);
    
    Cp test_cp = new Cp();
    test_cp.set_cmd_params(test_params);
    try {test_cp.Run(test_fs);}
    catch(JShellException je){
      assertEquals("test1.txt : File already existed",je.get_message());
    }
    
    
  }


}



