package driver.command;

import driver.JShell;
import driver.Exceptions.JShellException;
import driver.framework.FileSystem;

/**
 * This command will recall any of previous history by its number(>=1) preceded
 * by an exclamation point (!).
 * 
 * @author JacksonMa
 *
 */
public class RecallHistory extends Command {

  public RecallHistory() {
    super();
    this.doc = "!number \n" + "Addition: This is a new command in Assignment2B"
        + "This command will recall any of previous history by its number(>=1)"
        + "preceded by an exclamation point (!). For instance, if your history"
        + "looks like above, you could type the following on the command line "
        + "of your JShell i.e."
        + "!3 This will immediately recall and execute the command associated "
        + "with the history number 3."
        + " The above command of !3 will indirectly execute echo "
        + "\"Hello World\"";


    // TODO Auto-generated constructor stub
  }

  /**
   * This method check whether the arguments of "!number" command are valid.
   * 
   * Note: "!1" is invalid
   * 
   * @return whether the argument is valid.
   */
  public boolean isValid() {
    return this.params.size() == 1;

  }

  /**
   * This method executes the "!number" command.
   * 
   * @param FileSystem the file system the command collaborates with.
   * @return the result of execution of number n of the history
   * @throw JShellException if anything goes wrong
   */
  public String Run(FileSystem FS) throws JShellException {
    String outcome = "";
    int number = Integer.parseInt(this.params.get(0));
    if (number == FS.get_CH().Size()) {
      outcome = "Invalid command: the command you want to recall is just the "
          + "recall command you typed just now, please try another one!";
      throw new JShellException(outcome);
    } else if (number > FS.get_CH().Size()) {
      outcome = "Invalid command: the index of recall command exceeds the "
          + "number of history commands, please try a new index!";
      throw new JShellException(outcome);
    } else {
      outcome = FS.get_CH().get(number - 1);
      return JShell.run_command(outcome, FS);
    }


    // String outcome = "";
    // int number = Integer.parseInt(this.params.get(0));
    // System.out.println(FS.get_CH().Size());
    // if (number == FS.get_CH().Size()) {
    // outcome = "The command you want to recall is just the recall "
    // + "command you typed just now, please try another one!";
    // } else if (number > FS.get_CH().Size()) {
    // outcome = "Invalid command: the index of of recall command exceeds the "
    // + "number of history commands.";
    // } else {
    // String historyCommand = FS.get_CH().get(number - 1);
    //
    // String[] latex; // to store the segmented input
    // String cmd_name;
    //
    // if (historyCommand.contains("\"")) {
    // latex = historyCommand.trim().split("\"");
    //
    // } else {
    // latex = historyCommand.trim().split(" +");
    // } // latex now contains the list of strings
    // cmd_name = latex[0].trim();
    //
    // for (int i = 1; i < latex.length; i++) {
    // params.add(latex[i]);
    // } // get the parameters ready
    // Command cmd;
    // try {
    // cmd = (Command) Class.forName("driver.command."
    // + cmd_name.substring(0, 1).toUpperCase() + cmd_name.substring(1))
    // .newInstance();
    // cmd.set_cmd_params(params);
    // if (cmd.isValid()) {// if the command is valid, the checking
    // // responsibility is handled in each class
    // try {
    // outcome = cmd.Run(FS);
    // } catch (JShellException je) {
    // outcome = je.get_message();
    // }
    // } else {
    // outcome = "Invalid command, please try again";
    // }
    // } catch (InstantiationException e) {
    // // TODO Auto-generated catch block
    // e.printStackTrace();
    // } catch (IllegalAccessException e) {
    // // TODO Auto-generated catch block
    // e.printStackTrace();
    // } catch (ClassNotFoundException e) {
    // // TODO Auto-generated catch block
    // e.printStackTrace();
    // }
    // }
    // return outcome;
  }


}
