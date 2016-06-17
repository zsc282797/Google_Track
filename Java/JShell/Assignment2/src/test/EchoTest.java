package test;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.command.Echo;
import driver.Exceptions.*;
import driver.framework.*;
public class EchoTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    Echo test_echo = new Echo();
    test_echo.set_cmd_params(test_params);
    assertFalse(test_echo.isValid());
  }

  @Test
  public void testRun_with_quatation() {
    ArrayList<String> test_params = new ArrayList<String>();
    Echo test_echo = new Echo();
    
    test_params.add("\"CSC207 has a lot to write\"");
    test_echo.set_cmd_params(test_params);
    assertEquals("CSC207 has a lot to write",test_echo.Run(null));
  }
  
  @Test
  public void testRun_without_quatation() {
    ArrayList<String> test_params = new ArrayList<String>();
    Echo test_echo = new Echo();
    
    test_params.add("Helloooooo");
    test_echo.set_cmd_params(test_params);
    assertEquals("Helloooooo",test_echo.Run(null));
  }
  

}
