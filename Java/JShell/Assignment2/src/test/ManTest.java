package test;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.util.*;
import driver.command.*;
import driver.Exceptions.*;
import driver.framework.*;

public class ManTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
  
    Man test_man = new Man();
    test_man.set_cmd_params(test_params);
    assertFalse(test_man.isValid());
  }

  @Test
  public void testRun_normal() {
    ArrayList<String> test_params = new ArrayList<String>();
    test_params.add("cd");
    Man test_man = new Man();
    test_man.set_cmd_params(test_params);
    assertEquals("cd DIR\n\tChange directory to DIR, which may be relative to " +
        "the current "
      + "directory or may be a full path. As with Unix, .. means a parent "
      + "directory and a . means the current directory. The directory must "
      + "be /, the forward slash. The foot of the File system is a single "
      + "slash: /.", test_man.Run(null));
  }

  @Test
  public void testMan() {
    assertNotNull("The constructor did not work",new Man());
  }

}
