// note that if the URL specifies a.txt, then if there is an a.txt in
// our system, then it will be overwritten.
package driver.command;

import java.net.*;
import java.io.*;
import java.util.*;
import driver.framework.*;
import driver.framework.File;
import driver.Exceptions.*;
import driver.command.Command;
import driver.*;

public class Get extends Command {
  /**
   * The default constructor of the command
   * 
   * @param None
   * @return None
   */
  public Get() {
    super();
    this.doc = "get URL \n" + "Addition: This is a new command in Assignment2B"
        + "URL is a web address. Retrieve the file at that URL and add it to "
        + "the current working directory.";
  }

  /**
   * This method determines if a command is valid. If its size equals to
   * one,then it's valid otherwise it's invalid.
   * 
   * @return True or False
   */
  // we are assuming that URL ends in either .txt or .html,
  // specifying a text or html file.
  public boolean isValid() {
    return (this.params.size() == 1);
  }

  /**
   * This method gets a file specified by a URL and adds it to the current
   * working directory.
   * 
   * @param FS specifies the FileSystem in use.
   */
  public String Run(FileSystem FS) {
    try {
      // connect to URL
      URL myURL = new URL(this.params.get(0));
      URLConnection myURLConnection = myURL.openConnection();
      myURLConnection.connect();
      // read from URL
      BufferedReader in = new BufferedReader(
          new InputStreamReader(myURLConnection.getInputStream()));
      String inputLine;
      String contents = "";
      while ((inputLine = in.readLine()) != null)
        contents += inputLine + "\n";
      in.close();
      Directory start = FS.get_cur_dir();
      File to_add = new File();
      to_add.set_dir_name(this.getFileName());
      to_add.set_content(contents);
      start.add_dir(to_add);
    } catch (MalformedURLException e) {
      // new URL() failed
    } catch (IOException e) {
      // openConnection() failed
    }
    // get current dir and add new file.
    return "";
  }

  /**
   * This method finds and returns the name of the file that the URL passed in
   * by the user is specifying.
   * 
   * @return Return the string name of the file that the URL is specifying.
   */
  private String getFileName() {
    String URL = this.params.get(0);
    // gets the last argument after the final slash in the URL, which we
    // know will be a .html or .txt file, this will be used for naming
    // the new file being added to the filesystem.
    String fileName = URL.substring(URL.lastIndexOf("/") + 1, URL.length());
    return fileName;
  }
}
