package driver.command;

import driver.command.Command;
import driver.framework.*;
import driver.Exceptions.*;

/**
 * A command that prints the current working directory(including the whole path)
 * 
 * @author songzh
 */
public class Pwd extends Command {
  /**
   * This is the override constructor from Command It takes no parameters
   * 
   */

  public Pwd() {
    super();
    this.doc =
        "Print the current working directory (including the whole " + "path).";
  }

  /**
   * Check the number of parameters and tell if it is correct
   * 
   * @return boolean the result of the test
   */



  public boolean isValid() {

    return params.size() == 0; // We cannot have any paramters with pwd

  }

  /**
   * Run the command, print out the whole current working directory.
   * 
   * @param filsSystem
   * @return String :the current working path
   */

  public String Run(FileSystem FS) {
    String pwd = "";
    Directory temp = FS.get_cur_dir();
    while (temp.get_parent() != null) {
      String parent_name = temp.get_dir_names();
      pwd = parent_name + "/" + pwd;
      temp = temp.get_parent();

    }
    pwd = "/" + pwd;
    return (pwd);

  }

}
