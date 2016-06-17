package test;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.GoogleScholarHTMLExtractor;
import driver.RawContentGrabber;

public class RawContentGrabberTest {

  @Before
  public void setUp() throws Exception {}

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testGet_author_name() {
    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      assertEquals("Ola Spjuth", RCG.get_author_name());
    } catch (Exception e) {
      fail("Error occured!");
    }
  }

  @Test
  public void testGet_num_citation() {
    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      assertEquals("437", RCG.get_num_citation());
    } catch (Exception e) {
      fail("Error occured!");
    }
  }

  @Test
  public void testGet_i10_index() {
    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      assertEquals("12", RCG.get_i10_index());
    } catch (Exception e) {
      fail("Error occured!");
    }
  }

  @Test
  public void testGet_first_three_pubs() {

    String expected = "1- Bioclipse: an open source workbench for "
        + "chemo-and bioinformatics" + "\n\t2- The LCB data warehouse"
        + "\n\t3- XMPP for cloud computing in bioinformatics s"
        + "upporting discovery and invocation of asynchronous web services";



    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      assertEquals(expected, RCG.get_first_three_pubs().trim());
    } catch (Exception e) {
      fail("Error occured!");
    }
  }

  @Test
  public void testGet_total_paper_citation() {
    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      assertEquals("239", RCG.get_total_paper_citation().trim());
    } catch (Exception e) {
      fail("Error occured!");
    }
  }

  @Test
  public void testGet_co_author_names() {

    String expected =
        "Egon Willighagen" + "\nJonathan Alvarsson" + "\nChristoph Steinbeck"
            + "\nNina Jeliazkova" + "\nRajarshi Guha" + "\nSam Adams"
            + "\nJanna Hastings" + "\nSamuel Lampa" + "\nValentin Georgiev"
            + "\nAdam Ameur" + "\nKomorowski Jan" + "\ngilleain torrance"
            + "\nAntony John Williams" + "\nNoel M. O'Boyle" + "\nSean Ekins";

    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      RCG.get_co_author_names();
      assertEquals(expected, RCG.get_co_author_names().trim());
    } catch (Exception e) {
      fail("Error occured!");
      e.printStackTrace();
    }
  }


  @Test
  public void testGet_co_author_number() {

    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    RawContentGrabber RCG;
    try {
      RCG = new RawContentGrabber(GSHE.getHTML("sample1.html"));
      RCG.get_co_author_names();
      assertEquals("15", RCG.get_co_author_number());
    } catch (Exception e) {
      fail("Error occured!");
      // e.printStackTrace();
    }
  }

}
