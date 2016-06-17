package driver;

public class ConsoleOutput implements GeneralOutput {
  @Override
  public void output(String formatted_result) {

    System.out.println(formatted_result);
  };

}
