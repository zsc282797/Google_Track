package driver.command;

import driver.command.Command;

/**
 * A command that prints a list of previous commands One command perline, and
 * the highest number points to the most recent command.
 * 
 * @author songzh
 */
import driver.framework.*;
import driver.Exceptions.*;

public class History extends Command {
  /**
   * This is the default constructor of the command
   * 
   */
  public History() {
    super();
    this.doc = "     This command will print out recent commands, "
        + "one command per line. i.e." + "\n" + "\n" + "     1. cd.." + "\n"
        + "     2. mkdir textFolder" + "\n" + "     3. echo \"Hello World\""
        + "\n" + "     4. fsjhdfks" + "\n" + "     5. history" + "\n" + "\n"
        + "     The above output from history has two columns. The first column"
        + " is numbered such that the line with the highest number is the most "
        + "recent command. The most recent command is history. The second "
        + "column contains the actual command. Note: Your output should also "
        + "contain as output any syntactical errors typed by the user as seen "
        + "on line 4." + "/n" + "/n" + "     We can truncate the output by "
        + "specifying a number (>=0) after the command. For instance, if we "
        + "want to only see the last 3 commands typed, we can type the "
        + "following on the command line:" + "\n" + "\n" + "     history 3"
        + "\n" + "\n" + "     And the output will be as follows:" + "\n" + "\n"
        + "          4. fsjhdfks" + "\n" + "          5. history" + "\n"
        + "          6. history 3";

  }

  /**
   * This command check if the parameters passed to the command are valid
   * 
   * @param None
   * @return boolean
   */
  public boolean isValid() {
    if (this.params.size() == 1) {
      return this.params.get(0).matches("[0-9+]");
    }

    else {
      return this.params.size() == 0;
    }


  }

  /**
   * This is the main functionality of the command and will return the string
   * consisting pairs of index and corresponding user inputs
   * 
   * @param FileSystem FS that is shared among all the commands
   * @return String :the histories with their index
   */
  public String Run(FileSystem FS) {
    String ret_msg = "";
    if (params.size() == 0) {
      for (int i = 0; i < FS.get_CH().Size(); i++) {
        ret_msg += (Integer.toString(i + 1) + " " + FS.get_CH().get(i));
        ret_msg += "\n";
      }
    }

    else if (Integer.parseInt(params.get(0)) <= FS.get_CH().Size()) {
      for (int i = 0; i < Integer.parseInt(params.get(0)); i++) {
        ret_msg += (Integer
            .toString(FS.get_CH().Size() - Integer.parseInt(params.get(0)) + i)
            + " " + FS.get_CH()
                .get(FS.get_CH().Size() - Integer.parseInt(params.get(0)) + i));
        ret_msg += "\n";


      }
    } else {
      for (int i = 0; i < FS.get_CH().Size(); i++) {
        ret_msg += (Integer.toString(i + 1) + " " + FS.get_CH().get(i));
        ret_msg += "\n";


      }
    }
    return ret_msg;
  }

}
