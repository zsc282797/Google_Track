package driver.command;

import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import driver.*;
import driver.command.Command;
import driver.framework.*;
import driver.Exceptions.*;

/**
 * A command that creates directories in a current working directory or a full
 * path
 * 
 * @author c5songzh
 */
public class Mkdir extends Command {

  public Mkdir() {
    super();
    this.doc = "mkdir DIR ...\n\tCreate directories, each of which may be"
        + "relative to the current directory or may be a full path.";
  }

  /**
   * Return false if the parameters contain the special illegal characters, or
   * empty.
   * 
   * @return a boolean to indicate if the command is valid.
   */
  public boolean isValid() {

    List<String> special_char = Arrays.asList("!", "@", "$", "&", "*", "(", ")",
        "?", ":", "[", "]", "'", "\"", "<", ">", "`", "|", "=", "{", "}", "\\",
        "/", ",", ";", ".", "..");
    // !@$&*()?:[]"<>'`|={}.\../,;
    if (this.params.size() == 0) {
      return false;
    } else {
      // Pattern p = Pattern.compile("[^a-z0-9 ]",
      // Pattern.CASE_INSENSITIVE);
      for (String param : this.params) {
        for (String special : special_char) {
          if (param.contains(special)) {
            System.out.println("Directory or File name : " + param
                + " contians special character");
            return false;
          }

        }

      }
    }
    return true;
  }

  /**
   * Run the command, create the directory in the in the current working dir or
   * the given path.
   * 
   * @param fileSystem
   * @return None
   * @throws JShellException if problems are encountered
   */
  public String Run(FileSystem FS) throws JShellException {
    for (String cmd_param : this.params) {
      String[] dirs = cmd_param.split("/");
      if (dirs[0].equals(".")) { // case for current folder
        Directory start = FS.get_cur_dir();
        Directory to_add = new Directory();
        to_add.set_dir_name(dirs[dirs.length - 1]);
        start.add_dir(to_add);
        continue;
      }
      if (dirs[0].equals("..")) { // case for parent folder
        try {
          Directory start = FS.get_cur_dir().get_parent();
          Directory to_add = new Directory();
          to_add.set_dir_name(dirs[dirs.length - 1]);
          start.add_dir(to_add);
        } catch (NullPointerException e) {
          throw new JShellException("Cannot access the parent directory");

        }

      }
      if (dirs.length == 1) {

        boolean existed = false;
        for (Directory dir : FS.get_cur_dir().get_child()) {
          if (dir.get_dir_names().equals(dirs[dirs.length - 1])) {
            existed = true;
          }

        }

        if (!existed) {
          Directory to_add = new Directory();
          to_add.set_dir_name(dirs[0]);
          FS.get_cur_dir().add_dir(to_add);
        } else {
          throw new JShellException(
              "Directory " + dirs[dirs.length - 1] + " already existed");
        }
      } else if (cmd_param.charAt(0) == '/') {
        Directory start = new Directory();
        start = FS.get_root();
        boolean found = false;
        for (int i = 0; i < dirs.length - 1; i++) {
          found = false;
          for (Directory dir : start.get_child()) {
            if (dir.get_dir_names().equals(dirs[i])) {
              start = dir;
              found = true;
            }
          }
        }
        if (found) {

          boolean existed = false;
          for (Directory dir : start.get_child()) {
            if (dir.get_dir_names().equals(dirs[dirs.length - 1])) {
              existed = true;
            }

          }

          if (!existed) {
            Directory to_add = new Directory();
            to_add.set_dir_name(dirs[dirs.length - 1]);
            start.add_dir(to_add);
          }

          else {
            throw new JShellException(
                "Directory " + dirs[dirs.length - 1] + " already existed");
          }
        } else {
          throw new JShellException(
              dirs[dirs.length - 2] + ": Directory not" + " found");
        }
      } else if (cmd_param.charAt(0) != '/') {
        Directory start = new Directory();
        start = FS.get_cur_dir();
        boolean found = false;
        for (int i = 0; i < dirs.length - 1; i++) {
          found = false;
          for (Directory dir : start.get_child()) {
            if (dir.get_dir_names().equals(dirs[i])) {
              start = dir;
              found = true;
            }
          }
        }
        if (found) {
          boolean existed = false;
          for (Directory dir : start.get_child()) {
            if (dir.get_dir_names().equals(dirs[dirs.length - 1])) {
              existed = true;
            }

          }
          if (!existed) {
            Directory to_add = new Directory();
            to_add.set_dir_name(dirs[dirs.length - 1]);
            start.add_dir(to_add);
          } else {
            throw new JShellException(
                "Directory " + dirs[dirs.length - 1] + " already existed");
          }
        } else {
          throw new JShellException(
              dirs[dirs.length - 2] + ": Directory not" + " found");
        }
      }

    } // End of big loop
    return "";
  }

}
