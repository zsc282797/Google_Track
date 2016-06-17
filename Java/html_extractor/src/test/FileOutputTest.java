package test;

import static org.junit.Assert.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.FileOutput;

public class FileOutputTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}


  @Test
  public void testOutput()
  // test if the files are created
  {
    String test_output = "Test output";
    FileOutput FO = new FileOutput("test1.txt");
    FO.output(test_output);

    File file = new File("test1.txt");
    assertTrue(file.exists());
    // test if the content of the file is correct
    try {
      BufferedReader br = new BufferedReader(new FileReader("test1.txt"));
      StringBuilder sb = new StringBuilder();
      String line;
      try {
        line = br.readLine();
        while (line != null) {
          sb.append(line);
          sb.append(System.lineSeparator());
          line = br.readLine();
          String everything = sb.toString();
          assertEquals("Test output", everything.trim());
        }



      } catch (IOException e) {
        // TODO Auto-generated catch block
        fail("File reading error");
      }

    } catch (FileNotFoundException e) {

      fail("File reading error");
    }

  }


}
