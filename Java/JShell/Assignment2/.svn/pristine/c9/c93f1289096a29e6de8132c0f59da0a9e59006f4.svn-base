package driver.command;

import driver.command.Command;
import driver.framework.*;
import driver.Exceptions.*;
import java.util.*;

/**
 * A command prints the contents(given file or directory) of the current working
 * directory,with a new line following each of the content. If path specifies a
 * file, print this path If path specifies a directory, print this path with a
 * colon, then the content of this directory and a new line. If path does not
 * exist, return "No such file or directory"
 * 
 * @author c5songzh
 *
 */
public class Ls extends Command {
  /**
   * The default constructor of Ls class that takes no parameters
   * 
   * @param None
   * @return None
   */
  public Ls() {
    super();
    this.doc = "ls [PATH...]\n\tIf no paths are given, print the contents"
        + "(file or directory) "
        + "of the current directory, with a new line following each of the "
        + "content (file or directory).if –R is present, recursively "
        + "list all subdirectories" + "\n" + "\n\tOtherwise, for each "
        + "path p, the order listed:" + "\n" + "\n" + "     - If p specifies a "
        + "file, print p" + "\n" + "\n" + "     - If p specifies a directory, "
        + "print p, a colon, then the contents of that directory, then an extra"
        + " new line." + "\n" + "\n" + "     - If p does not exist, print a "
        + "suitable message.";
  }

  /**
   * Return a boolean that indicates if the current directory is valid to use
   * the command Ls. If its size greater than 0 then it's valid otherwise it's
   * invalid.
   * 
   * @return True of False indicates if the command valid.
   */
  public boolean isValid() {
    return this.params.size() >= 0;
    // return exception?
    // check valid path?
  }

  /**
   * Return the contents in the current directory, with a new line following
   * each of the content.
   * 
   * @param filesystem that is shared among all the commands
   * @throws JShellException if return correct value is not possible
   * @return Empty String
   */
  public String Run(FileSystem FS) throws JShellException {
    String ret_msg = "";
    ArrayList<Directory> all_dirs = new ArrayList<Directory>();
    if (!this.params.isEmpty()) {

      // recursive
      if (this.params.get(0).equals("-R") || this.params.get(0).equals("-r")) {
        Directory start = Directory.find_dir(FS, params.get(1));
        if (start == null) {
          throw new JShellException("Directory incorrect!");
        } else {
          all_dirs = Directory.get_all_dirs(start);
        }
        for (Directory dir : all_dirs) {
          if (dir.getClass().getName().equals("driver.framework.Directory")) {
            ret_msg += Directory.get_path(dir) + ":";
          }

          for (Directory sub_dir : dir.get_child()) {
            ret_msg += " " + sub_dir.get_dir_names();
          }
          ret_msg += "\n";
        }
      }
    } else {

      // ls is called with no arguments
      ret_msg += (FS.get_cur_dir().get_dir_names() + ": ");

      // check for children of current directory
      if (FS.get_cur_dir().get_child().size() > 0) {
        for (Directory dir : FS.get_cur_dir().get_child()) {
          ret_msg += (dir.get_dir_names() + " ");
        }
      }
      ret_msg += ("\n");

    }
    return ret_msg;
  }



}
