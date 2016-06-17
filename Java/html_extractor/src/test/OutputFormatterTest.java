package test;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.GoogleScholarHTMLExtractor;
import driver.RawContentGrabber;
import driver.OutputFormatter;

public class OutputFormatterTest {

  @Before
  public void setUp() throws Exception {

  }

  @After
  public void tearDown() throws Exception {}



  @Test
  public void testGenerate_Formatted_Information() {

    String expected = "--------------------------------------"
        + "---------------------------------" + "\n1. Name of Author:"
        + "\n\tOla Spjuth" + "\n2. Number of All Citations:" + "\n\t437"
        + "\n3. Number of i10-index after 2009:" + "\n\t12"
        + "\n4. Title of the first 3 publications:"
        + "\n\t1- Bioclipse: an open source workbench "
        + "for chemo-and bioinformatics"
        + "\n\t2- The LCB data warehouse"
        + "\n\t3- XMPP for cloud computing in bioinformatics "
        + "supporting discovery and invocation of asynchronous web services"
        + "\n5. Total paper citation (first 5 papers):" + "\n\t239"
        + "\n6. Total Co-Authors:" + "\n\t15";



    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      String result;
      OutputFormatter test_OF = new OutputFormatter(RCG);
      result = test_OF.Generate_Formatted_Information();
      assertEquals(expected, result.trim());
    } catch (Exception e) {
      fail("HTML reading error!");
    }


  }

}
