package test;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import driver.GoogleScholarHTMLExtractor;

public class GoogleScholarHTMLExtractorTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testGetHTML_NoFile() {
    // test case for GetHTML method if name is invalid
    String result;
    GoogleScholarHTMLExtractor gs = new GoogleScholarHTMLExtractor();
    try {
      result = gs.getHTML("nofile.html");
    } catch (Exception e) {

      assertEquals("java.io.FileNotFoundException", e.getClass().getName());
    }
  }

  @Test
  public void testGetHTML_isFile() {
    // Test case for GetHTML method if name is a valid file
    String result;
    GoogleScholarHTMLExtractor gs = new GoogleScholarHTMLExtractor();

    try {
      result = gs.getHTML("sample1.html");
      assertNotNull(result);
    } catch (Exception e) {

      fail("Unexpected Error!");
    }
  }



}
