// **********************************************************
// Assignment3:
// CDF user_name:c5songzh
//
// Author:Zhancheng Song
// 1000787758
//
//
// Honor Code: I pledge that this program represents my own
// program code and that I have coded on my own. I received
// help from no one in designing and debugging my program.
// *********************************************************
package driver;

import java.util.*;



public class MyParser {


  /**
   * @param args : args[0] filenames ,args[1] outfile with name This is the main
   *        functionality of the program and it will use other classes to
   *        implement the functionality
   * 
   *        This function will either write the output to file if given as a
   *        parameters or will print to the console if no outfile name is given
   */
  public static void main(String[] args) {
    GoogleScholarHTMLExtractor GSHE = new GoogleScholarHTMLExtractor();
    String inputFiles[] = args[0].split(",");
    ArrayList<RawContentGrabber> RCGs = new ArrayList<RawContentGrabber>();
    String formatted_information = "";
    for (String inputFile : inputFiles) {
      try {

        String raw_html = GSHE.getHTML(inputFile);
        // Get the raw html from addressed file
        RawContentGrabber RCG = new RawContentGrabber(raw_html);
        // RCG is the processing unit of raw html
        RCGs.add(RCG);
        // RCGs is the list of all the content grabber according to the file
        // list

        OutputFormatter OF = new OutputFormatter(RCG);
        formatted_information += OF.Generate_Formatted_Information();



      } catch (Exception e) {

        System.err.println("Error when reading HTML files");
        System.exit(0);
      }

    }
    formatted_information += RawContentGrabber.Generate_Ending(RCGs);


    if (args.length == 1) {
      ConsoleOutput COUT = new ConsoleOutput();

      COUT.output(formatted_information);
      // if the filename is not provided
      // dump to the console
    } else if (args[1].isEmpty()) {
      ConsoleOutput COUT = new ConsoleOutput();

      COUT.output(formatted_information);
    } else {
      FileOutput FO = new FileOutput(args[1]);
      FO.output(formatted_information);
      // if the filename is correct, create and write file

    }



  }



}


