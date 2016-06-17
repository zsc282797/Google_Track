
package driver;

import driver.framework.*;
import driver.Exceptions.*;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Scanner;

import driver.command.Command;

import driver.command.RecallHistory;

public class JShell {

  public static void main(String[] args) {

    Scanner in = new Scanner(System.in);
    String s;

    FileSystem FS = new FileSystem();
    CmdHistory CH = new CmdHistory();
    FS.set_CH(CH);


    System.out.print(FS.get_cur_dir().get_dir_names() + "# ");
    while (!(s = in.nextLine()).equals("exit")) {

      if (s.lastIndexOf(">>") > 0) { // if ">>" is present in the command
        String[] words = s.split(" ");
        String destination = words[words.length - 1];
        try {
          File found = File.find_file(FS, destination);
          if (found == null) {
            File created = File.create_file(FS, destination);
            String cmd = s.substring(0, s.lastIndexOf(">>"));
            created.set_content(run_command(cmd, FS));
          } else {
            String cmd = s.substring(0, s.lastIndexOf(">>"));
            found.set_content(found.get_content() + run_command(cmd, FS));
          }
        } catch (JShellException je) {
          System.err.print(je.get_message() + "\n");
        }
        System.out.print(FS.get_cur_dir().get_dir_names() + "# ");
        continue;
      }

      else if (s.lastIndexOf(">") > 0) { // if ">" is present in the command
        String[] words = s.split(" ");
        String destination = words[words.length - 1];
        String result = "";
        String cmd = s.substring(0, s.lastIndexOf(">"));
        try {
          File created = File.create_file(FS, destination);

          result = run_command(cmd, FS);
          created.set_content(result);


        } catch (JShellException e) {
          // TODO Auto-generated catch block
          if (e.get_message().contains("already")) {
            try {
              result = run_command(cmd, FS);
              File found = File.find_file(FS, destination);
              found.set_content(result);

            } catch (JShellException e1) {
              // TODO Auto-generated catch block
              System.err.println(e1.get_message());
            }

          } else {
            System.err.println(e.get_message());
          }
          System.out.print(FS.get_cur_dir().get_dir_names() + "# ");
          continue;
        }
        System.out.print(FS.get_cur_dir().get_dir_names() + "# ");

        continue;
      }

      else {
        String result = run_command(s, FS);
        if (result.equals("")) {
          // pass
        } else {
          System.out.println(result);
        }


        System.out.print(FS.get_cur_dir().get_dir_names() + "# ");
      } // end of the case where normal operation is in effect
    } // end of while loop

  }// end of main

  public static String run_command(String s, FileSystem FS) {

    FS.get_CH().add(s);

    String[] parts = s.split(" ");
    String cmd_name = parts[0];
    ArrayList<String> cmd_params = new ArrayList<String>();

    if (cmd_name.equals("echo")) {
      if (parts[1].charAt(0) == '\"') {
        parts = s.split("\"");
        cmd_params.add(parts[1]);
      } else {
        for (int i = 1; i < parts.length; i++) {
          cmd_params.add(parts[i]);
        }
      }
    } else if (cmd_name.charAt(0) == '!') {
      // System.out.println("Debug: cmd_name.split(!)[1] is"
      // +cmd_name.split("!")[1]);
      cmd_params.add(cmd_name.split("!")[1]);
      for (int i = 1; i < parts.length; i++) {
        cmd_params.add(parts[i]);
      }
      RecallHistory rh = new RecallHistory();
      rh.set_cmd_params(cmd_params);
      try {
        return rh.Run(FS);
      } catch (JShellException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
      }
    }


    else {

      for (int i = 1; i < parts.length; i++) {
        cmd_params.add(parts[i]);
      }
    }


    // Generate a command
    try {
      Command cmd = (Command) Class.forName("driver.command."
          + cmd_name.substring(0, 1).toUpperCase() + cmd_name.substring(1))
          .newInstance();
      cmd.set_cmd_params(cmd_params);
      if (cmd.isValid()) {
        return cmd.Run(FS);
      } else {
        return ("Invalid Command, please try again");
      }
    } catch (JShellException je) {
      return (je.get_message());
    } catch (InstantiationException e) {
      // TODO Auto-generated catch block
      return ("Invalid Command, please try again");
    } catch (IllegalAccessException e) {
      // TODO Auto-generated catch block
      return ("Invalid Command, please try again");
    } catch (ClassNotFoundException e) {
      // TODO Auto-generated catch block
      return ("Invalid Command, please try again");
    }
  }



}
