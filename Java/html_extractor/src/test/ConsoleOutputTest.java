package test;

import static org.junit.Assert.*;
import driver.ConsoleOutput;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class ConsoleOutputTest {
  private final ByteArrayOutputStream outContent = new ByteArrayOutputStream();
  private final ByteArrayOutputStream errContent = new ByteArrayOutputStream();

  @Before
  public void setUp() throws Exception {
    System.setOut(new PrintStream(outContent));
    System.setErr(new PrintStream(errContent));
  }

  @After
  public void tearDown() throws Exception {
    System.setOut(null);
    System.setErr(null);
  }

  @Test
  public void testOutput() {
    // This test case test if the printed message matches the input
    String test_info = "This is a test";
    ConsoleOutput CO = new ConsoleOutput();
    CO.output(test_info);
    assertEquals("This is a test", outContent.toString().trim());
  }

}
