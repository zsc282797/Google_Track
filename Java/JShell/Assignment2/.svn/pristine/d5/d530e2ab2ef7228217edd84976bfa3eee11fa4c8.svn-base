package driver.command;

import driver.command.Command;
import driver.framework.*;
import driver.Exceptions.*;

/**
 * A command that saves the old current working directory in directory stack so
 * that it can be returned at any time via popd command. Saves current working
 * directory by pushing onto direcotry stack and changes the new current working
 * directory to DIR. recent command.
 */

public class Pushd extends Command {

  public Pushd() {
    super();
    this.doc = "Saves the current working directory by pushing onto directory "
        + "stack and then changes the new current working directory to DIR. "
        + "The push must be consistent as per the LIFO behavior of a stack. "
        + "The pushd command saves the old current working directory in "
        + "directory stack so that it can be returned to at any time (via the "
        + "popd command). The size of the directory stack is dynamic and "
        + "dependent on the pushd and the popd commands.";
  }

  /**
   * Run the command, save the old current working directory in directory stack
   * so that it can be returned at any time via popd command. Saves current
   * working directory by pushing onto direcotry stack and changes the new
   * current working directory to DIR.
   * 
   * @param filsSystem
   * @return Emepty String
   * @throws JShellException
   */

  public String Run(FileSystem FS) throws JShellException {
    FS.get_DS().push(FS.get_cur_dir());
    Cd cd = new Cd();
    cd.set_cmd_params(this.params);
    cd.Run(FS);
    return "";
  }

  /**
   * Return a boolean that indicates if the current directory is valid to use
   * the command Ls. If its size equals to 1 then it's valid otherwise it's
   * invalid.
   * 
   * @return True of False indicates if the command valid.
   */
  public boolean isValid() {
    return this.params.size() == 2;
  }
}
