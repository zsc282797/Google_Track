package driver.command;

import driver.framework.*;
import driver.Exceptions.*;

/**
 * A command that displays the content of a file.
 * 
 * @author c5songzh
 */

public class Cat extends Command {
  /**
   * constructor of class Cat, inherit from super class Command Default
   * constructor of the command that takes no arguments
   * 
   * @param None
   * @return None
   * 
   */

  public Cat() {
    super();
    this.doc = "Display the contents of FILE1 and other Files (i.e. File2...) "
        + "concatenated in the shell. You may want to use three line breaks to "
        + "separate the contents of one File from the other File.";
  }

  /**
   * This method detects if a command is valid. If its size greater than 0 then
   * it's valid otherwise it's invalid.
   * 
   * @return boolean evaluates if the parameters are valid
   */
  public boolean isValid() {
    if (params.size() > 0) {
      return true;
    } else {
      return false;
    }
  }

  /**
   * This method returns the content of the file according to its absolute path
   * If the file or path does not exist, return "no such file or directory"
   * 
   * @param FS determines a file system contains the potential target files.
   * @return the content of the file if exist
   * @throw "No such file or directory" if file or directory does not exist
   */
  public String Run(FileSystem FS) throws JShellException {
    String ret_msg = "";

    for (String cmd_param : this.params) {

      try {
        File result = File.find_file(FS, cmd_param);

        if (result == null) {
          ret_msg += (cmd_param + " No such file or directory");
        } else {
          ret_msg += (result.get_dir_names() + ": " + result.get_content());
        }

      } catch (JShellException je) {
        throw (je);
      }

    }

    return ret_msg;
  }
}
