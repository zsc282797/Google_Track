package driver.command;

import driver.framework.*;
import driver.Exceptions.*;
import java.util.EmptyStackException;

import driver.command.Command;

/**
 * A command that removes the top most directory from the directory stack and cd
 * into it. The removal is based on LIFO behavior of a stack If there is no
 * directory onto the stack, returns appropriate error message
 */
public class Popd extends Command {

  public Popd() {
    super();
    this.doc = "Remove the top entry from the directory stack, and cd into it. "
        + "The removal must be consistent as per the LIFO behavior of a stack. "
        + "The popd command removes the top most directory from the directory "
        + "stack and makes it the current working directory. If there is no "
        + "directory onto the stack, then give appropriate error message.";
  }

  /**
   * Return a boolean that indicates if the current directory is valid to use
   * the command Ls. If its size equals to 0 then it's valid otherwise it's
   * invalid.
   * 
   * @return True of False indicates if the command valid.
   */
  public boolean isValid() {
    return this.params.size() == 0;
  }

  /**
   * Run the command, pop out the top most directory in the directory stack, and
   * cd into it. Printout the err message if the directory is empty.
   * 
   * @param fileSystem
   * @return Emepty String
   */
  public String Run(FileSystem FS) throws JShellException {

    try {
      FS.set_cur_dir(FS.get_DS().pop());
    } catch (EmptyStackException e) {
      throw new JShellException("directory stack empty");
    }

    return "";
  }
}
