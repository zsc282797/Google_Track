package driver.command;

import driver.command.*;

import driver.framework.*;
import driver.Exceptions.*;
import java.util.*;

public class Cp extends Command {
  /**
   * This is the default constructor of the Command
   * 
   */

  public Cp() {
    this.doc = "cp OLDPATH NEWPATH \n"
        + "Addition: This is a new command in Assignment2B"
        + "Like mv, but don’t remove OLDPATH. If OLDPATH is a directory, "
        + "recursively copy the contents.";


  }


  /**
   * This method detects if a command is valid. If its size greater than 0 then
   * it's valid otherwise it's invalid.
   * 
   * @return boolean evaluates if the parameters are valid
   */
  public boolean isValid() {

    return this.params.size() == 2;
  }

  /**
   * This method copy the file defined by the first parameter to the directory
   * Specified by the second parameter
   * 
   * @param FS determines a file system contains the potential target files.
   * @return Empty String if executed correctly.
   * @throw "No such file or directory" if file or directory does not exist
   */
  public String Run(FileSystem FS) throws JShellException {
    String origin = this.params.get(0);
    String des = this.params.get(1);


    String filename = "";
    String file_content = "";
    try {
      File file_result = File.find_file(FS, this.params.get(0));
      if (file_result == null) {
        throw (new JShellException(
            this.params.get(0) + ": No such File or Directory"));
      } else {// succesfully found the file
        filename = file_result.get_dir_names();
        file_content = file_result.get_content();
        // avoid direct copy to protect pointer leak
      }
      Directory dir_result = Directory.find_dir(FS, this.params.get(1));
      if (dir_result == null) {
        throw (new JShellException(
            this.params.get(1) + ": No such File or Directory"));
      } else {
        for (Directory test : dir_result.get_child()) {
          if (test.getClass().getName().equals("driver.framework.File")
              && test.get_dir_names().equals(filename)) {
            throw new JShellException(filename + " : File already existed");
          }
        }

        File new_file = new File();
        new_file.set_content(file_content);
        new_file.set_dir_name(filename);
        dir_result.add_dir(new_file);
      }

    } catch (JShellException je) {
      throw (je);
    }

    return "";
  }
}
