package driver;

import java.util.ArrayList;

public class OutputFormatter {
  private RawContentGrabber RCG;

  /**
   * @param RCG : A RawContentGrabber object This is the constructor that will
   *        take a RCG as a parameter
   * 
   */
  public OutputFormatter(RawContentGrabber RCG) {
    this.RCG = RCG;
  }


  public String Generate_Formatted_Information() {

    String formatted_information = "------------------------"
        + "-----------------------------------------------";
    // Initial result as a sperating line for files
    formatted_information += "\n1. Name of Author:\n\t"; // name of author
    formatted_information += RCG.get_author_name(); // use RCG's method
    formatted_information += "\n2. Number of All Citations:\n\t";
    formatted_information += RCG.get_num_citation();
    formatted_information += "\n3. Number of i10-index after 2009:\n\t";
    formatted_information += RCG.get_i10_index();
    formatted_information += "\n4. Title of the first 3 publications:";
    formatted_information += RCG.get_first_three_pubs();
    formatted_information += "5. Total paper citation (first 5 papers):\n\t";
    formatted_information += RCG.get_total_paper_citation();
    formatted_information += "6. Total Co-Authors:\n\t";
    RCG.get_co_author_names();
    formatted_information += RCG.get_co_author_number() + "\n";

    return formatted_information;
  }


}
