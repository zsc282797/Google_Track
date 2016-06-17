package driver;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;

public class FileOutput implements GeneralOutput {
  private File output_file;

  public FileOutput(String filename) {
    output_file = new File(filename); // create file in the current


  }

  public void output(String formatted_result) {
    if (output_file.isAbsolute()) { // if the path name is Absolute path
      System.err.println("File path cannot be absolute! \n");
    } else {
      try { // convert the string to bytes according to the File class
        Files.write(output_file.toPath(), formatted_result.getBytes());
      } catch (IOException e) {
        System.err.println("Problem encountered during creation of file");
      }
    }


  };
}
