package driver.command;

import driver.framework.*;

import java.util.ArrayList;

import driver.Exceptions.JShellException;
import driver.command.Command;


public class Grep extends Command {

  public Grep() {
    super();
  }

  public boolean isValid() {
    return this.params.size() == 3 || this.params.size() == 2;
    // paths must be files, how should we check for this?
  }

  public String Run(FileSystem FS) throws JShellException {
    String result = "";
    String path;
    String pattern;
    ArrayList<File> all_files = new ArrayList<File>();

    if (this.params.get(0).equals("-r") || this.params.get(0).equals("-R")) {
      path = params.get(2);
      // System.out.println("path got is : "+path);
      Directory start = Directory.find_dir(FS, path);
      if (start == null) {
        throw new JShellException("Directory incorrect");
      }
      all_files = Directory.get_all_files(start);
      pattern = params.get(1);

    } else {
      path = params.get(1);
      // System.out.println("path got is : "+path);
      Directory start = Directory.find_dir(FS, path);
      if (start == null) {
        throw new JShellException("Directory incorrect");
      }
      for (Directory dir : start.get_child()) {
        if (dir.getClass().getName().equals("driver.framework.File")) {
          all_files.add((File) dir);
        }
      }
      pattern = params.get(0);
    }
    // ArrayList now contains files from the file system from root

    for (File f : all_files) {
      String[] lines = f.get_content().split("\n");
      for (String line : lines) {
        if (line.contains(pattern)) {
          result += File.get_path(f) + ": " + line + "\n";
        }
      }
    }
    return result;
  }
}
