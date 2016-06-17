package driver.command;

import driver.command.*;
import driver.command.Command;

import java.util.*;
/**
 * A command with [>] that prints string on shell if the outfile not provided
 * Put string into outfile if provided Creats a new file if outfile does not
 * exist and overwrites the old contents if the old outfile exists. A command
 * with [>>] has same function except it appends string to outfile instead of
 * overwrites it.
 */
import driver.framework.*;
import driver.Exceptions.*;

public class Echo extends Command {

  public Echo() {
    super();
    this.doc = "If OUTFILE is not provided, print STRING on the shell. "
        + "Otherwise, put STRING into file OUTFILE. STRING is a string of "
        + "characters surrounded by double quotation marks. This creates a new "
        + "File if OUTFILE does not exists and erases the old contents if "
        + "OUTFILE already exists. However if two greater-than signs, >>, were "
        + "used instead of a single greater-than sign, >, append instead of "
        + "overwrites.";
  }

  /**
   * This method detects if a command is valid. If there is only one parameter
   * then it's valid otherwise it's invalid.
   * 
   * @return boolean evaluates if the parameters are valid
   */
  public boolean isValid() {


    return this.params.size() == 1;
  }

  /**
   * This method runs lines return the message itself or the message without
   * qutation marks
   * 
   * @param None
   * @return String :the message
   */

  public String Run(FileSystem FS) {
    if (this.params.get(0).charAt(0) == '\"') {
      return params.get(0).split("\"")[1];
    } else {
      return this.params.get(0);
    }

  }
}
