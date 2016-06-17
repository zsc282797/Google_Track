package driver.command;

import driver.framework.*;
import driver.Exceptions.*;
import driver.command.Command;

/**
 * A command that displays the documentation of commands that are subclasses of
 * the Command super class.
 * 
 * @author BowenX
 */
public class Man extends Command {

  public Man() {
    super();
    this.doc = "Print documentation for CMD";
  }

  /**
   * Return a boolean that indicates if the current command is valid to be used.
   * If its size equals 1 then it's valid otherwise it is invalid.
   * 
   * @return True of False indicates if the command valid.
   */
  public boolean isValid() {
    return this.params.size() == 1;
  }

  /**
   * Display the documentation of the command.
   * 
   * @param fileSystem
   * @return String : the documentation from each class
   */
  public String Run(FileSystem FS) {
    try {
      Command cmd = (Command) Class.forName(
          "driver.command." + this.params.get(0).substring(0, 1).toUpperCase()
              + params.get(0).substring(1))
          .newInstance();

      return (cmd.getDoc());
    } catch (InstantiationException e) {
      // TODO Auto-generated catch block
      // e.printStackTrace();
      return ("Invalid command, please try again");
    } catch (IllegalAccessException e) {
      // TODO Auto-generated catch block
      // e.printStackTrace();
      return ("Invalid command, please try again");
    } catch (ClassNotFoundException e) {
      // TODO Auto-generated catch block
      // e.printStackTrace();
      return ("Invalid command, please try again");
    }


  }


}
