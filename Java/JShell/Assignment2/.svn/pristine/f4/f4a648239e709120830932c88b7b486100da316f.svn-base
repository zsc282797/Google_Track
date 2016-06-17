package driver.command;

/**
 * It is a super class for all Jshell commands.
 * 
 * @author songzh
 */

import java.util.*;

import driver.framework.FileSystem;
import driver.Exceptions.JShellException;

public class Command {
  /**
   * constructor of class Command, set up instance variables.
   */

  protected String cmd_name;
  protected ArrayList<String> params;
  protected String doc = "Should never see this line";


  public Command() {
    this.cmd_name = "";
    this.params = new ArrayList<String>();

  }

  /**
   * This is a boolean function
   * 
   * @return True
   */
  public boolean isValid() {
    return true;
  }

  /**
   * This method sets the command's name
   * 
   * @param cmd_name
   */
  public void set_cmd_name(String cmd_name) {
    this.cmd_name = cmd_name;
  }

  /**
   * This method gets the command's name
   * 
   * @return the name of a command
   */
  public String get_cmd_name() {
    return this.cmd_name;
  }

  /**
   * This method sets the command parameter
   * 
   * @param cmd_params
   */
  public void set_cmd_params(ArrayList<String> cmd_params) {
    this.params = cmd_params;

  }

  /**
   * The method returns the value of command parameter
   * 
   * @return the parameter in an Arraylist.
   */
  public ArrayList<String> get_cmd_params() {
    return this.params;
  }

  /**
   * The method prints the document of a command
   * 
   * @return the document
   */
  public String getDoc() {
    return this.doc;
  }

  /**
   * The method prints a statement
   * 
   * @param FS
   * @throws JShellException
   */
  public String Run(FileSystem FS) throws JShellException {
    return ("Debug:Uncertain Command, Check implemetation");
  }
}

