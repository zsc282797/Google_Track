package driver;

import org.apache.commons.lang3.StringEscapeUtils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.*;

public class RawContentGrabber {

  private String HTML_Content;

  private ArrayList<String> co_author_names;

  /**
   * 
   * @param raw_HTML: A string of html content This is the constructor of
   *        RawCOntentGrabber class which takes an unprocessed raw HTML content
   *        string as input
   */
  public RawContentGrabber(String raw_HTML) {
    this.HTML_Content = raw_HTML;

  }

  /**
   * @param re : Regular Expression required to find information
   * @param Raw_HTML : A string of html content
   * @return A string of result searched from the HTML, if exists else return
   *         null;
   */
  private static String GetInfoFromRe(String re, String Raw_HTML) {
    Pattern pattern = Pattern.compile(re);
    Matcher matcher = pattern.matcher(Raw_HTML);
    if (matcher.find()) {
      return matcher.group(1);
    }
    return null;
  }

  /**
   * 
   * @return the author name if searched from the HTML content
   */
  public String get_author_name() {
    String reForAuthorExtraction = "<span id=\"cit-name-display\" "
        + "class=\"cit-in-place-nohover\">(.*?)</span>";
    return GetInfoFromRe(reForAuthorExtraction, this.HTML_Content);

  }

  /**
   * 
   * @return the number of total citations if searched from the HTML content
   */
  public String get_num_citation() {
    String reForCitationExtraction =
        "Citations</a></td><td class=\"cit-borderleft cit-data\">"
            + "(.*?)</td>";
    return GetInfoFromRe(reForCitationExtraction, this.HTML_Content);

  }

  /**
   * 
   * @return the i10_index if searched from the HTML content
   */
  public String get_i10_index() {
    String reForI10Extraction =
        "i10-index</a></td><td class=\"cit-borderleft cit-data\">"
            + "(.*?)</td>";
    return GetInfoFromRe(reForI10Extraction, this.HTML_Content);

  }

  /**
   * 
   * @return the first three publications if searched from the HTML content
   */
  public String get_first_three_pubs() {
    String result = "";
    int count = 1;
    String reForTitileExtraction =
        "class=\"cit-dark-large-link\">" + "(.*?)</a><br>";
    Pattern patternTittle = Pattern.compile(reForTitileExtraction);
    Matcher matcherTitle = patternTittle.matcher(this.HTML_Content);
    while (matcherTitle.find() && count < 4) {
      result += "\n\t" + count + "- " + matcherTitle.group(1);
      count++;
    }
    result += "\n";
    if (!(result.equals(""))) {
      return result;
    } else {
      return null;
    }
  }

  /**
   * 
   * @return the total number of paper citeted if searched from the HTML content
   */
  public String get_total_paper_citation() {
    String result = "";
    String reForNumCitation =
        "class=\"cit-dark-link\" href=\"http://scholar.google.ca/scholar\\?oi"
            + "=bibs&hl=en&cites=\\d+\">" + "(.*?)</a></td>";
    Pattern patternNumCitation = Pattern.compile(reForNumCitation);
    Matcher matcherNumCitation = patternNumCitation.matcher(this.HTML_Content);
    int count = 0;
    int sum = 0;
    for (count = 0; count < 5; count++) {
      if (matcherNumCitation.find()) {
        sum += Integer.parseInt(matcherNumCitation.group(1));
      }
    }
    result += sum + "\n";
    return result;
  }

  // This is the helper function generate the arraylist of co-author names
  private ArrayList<String> generate_co_author_list() {
    this.co_author_names = new ArrayList<String>();
    String reForCoAuthors = "&hl=en\" title=\"(.*?)\">";
    Pattern pattern = Pattern.compile(reForCoAuthors);
    Matcher matcher = pattern.matcher(this.HTML_Content);
    while (matcher.find()) {

      co_author_names.add(StringEscapeUtils.unescapeHtml3(matcher.group(1)));
    }
    return co_author_names;
  }


  public String get_co_author_names() {
    String names = "";
    this.co_author_names = generate_co_author_list();
    for (String name : this.co_author_names) {
      names += name + "\n";
    }
    return names;
  }

  public String get_co_author_number() {
    return Integer.toString(this.co_author_names.size());

  }

  public static String Generate_Ending(ArrayList<RawContentGrabber> RCGs) {
    String result = "\n----------------------------------"
        + "-------------------------------------\n";

    int sum = 0;
    ArrayList<String> all_co_names = new ArrayList<String>();

    for (RawContentGrabber RCG : RCGs) {
      sum += Integer.parseInt(RCG.get_co_author_number());
      all_co_names.addAll(RCG.generate_co_author_list());
    }



    result += "7. Co-Author list sorted (Total: " + sum + "):\n";

    Collections.sort(all_co_names);
    for (String name : all_co_names) {
      result += name + "\n";
    }


    return result;
  }

}
