package driver.command;

import driver.framework.*;
import driver.Exceptions.*;
import driver.command.Command;

public class Mv extends Command {

  public Mv() {
    this.doc = "mv OLDPATH NEWPATH \n"
        + "Addition: This is a new command in Assignment2B"
        + "Move item OLDPATH to NEWPATH. Both OLDPATH and NEWPATH may be "
        + "relative to the current directory or "
        + "may be full paths. If NEWPATH is a directory, move the item into"
        + " the directory.";
  }

  /**
   * Test if the parameters are of the right number
   * 
   * @return boolean
   */
  public boolean isValid() {

    return this.params.size() == 2;

  }

  /**
   * This method coporate the main functionality and mv files described in the
   * first parameters to the location in second parameter
   * 
   * @param FileSystem fs where the file and the destination location is in
   * @return Empty String
   * @throws JShellException when problems occours
   * 
   */

  public String Run(FileSystem FS) throws JShellException {

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
            this.params.get(0) + ": No such File or Directory"));
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

      file_result.get_parent().remove_dir(file_result);
    } catch (JShellException je) {

      System.err.println(je.get_message());
    }
    return "";
  }

}
