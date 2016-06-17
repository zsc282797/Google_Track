package driver.command;

/**
 * A command that changes the present working directory change File system's
 * directory to DIR which maybe relative to current directory or a full path
 * 
 * @author songzh
 *
 */
import driver.framework.*;
import driver.Exceptions.*;

public class Cd extends Command {
  /**
   * constructor of class Cat, inherit from super class Command Default
   * constructor of the command that takes no arguments
   * 
   * @param None
   * @return None
   * 
   */
  public Cd() {
    super();
    this.doc = "cd DIR\n\tChange directory to DIR, which may be relative to "
        + "the current "
        + "directory or may be a full path. As with Unix, .. means a parent "
        + "directory and a . means the current directory. The directory must "
        + "be /, the forward slash. The foot of the File system is a single "
        + "slash: /.";
  }

  /**
   * This method determines if a command is valid. If its size equals to
   * one,then it's valid otherwise it's invalid.
   * 
   * @return True or False
   */
  public boolean isValid() {
    return (this.params.size() == 1);
  }

  /**
   * This method changes the file system's current directory to DIR if DIR
   * exists.
   * 
   * @param FS determines a file system contains the potential Directroy.
   * @return Empty String
   * @throw JShellException "No such File or Directory" if Directory not found
   */
  public String Run(FileSystem FS) throws JShellException {
    String ret_string = "";
    if (this.params.get(0).equals("..")) {
      if (FS.get_cur_dir().get_parent() != null) {
        FS.set_cur_dir(FS.get_cur_dir().get_parent());
      } else {
      }
    }

    else {
      Directory result = Directory.find_dir(FS, this.params.get(0));
      if (result == null) {
        // System.out.println("No such file or directory");
        throw (new JShellException("No such File or Directory"));
      } else {
        FS.set_cur_dir(result);

      }
    }
    return ret_string;
  }
}
