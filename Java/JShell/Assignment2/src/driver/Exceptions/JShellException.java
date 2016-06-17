package driver.Exceptions;

public class JShellException extends Exception {
  private String message;

  public JShellException(String msg) {
    this.message = msg;

  }

  public String get_message() {
    return this.message;

  }
}
