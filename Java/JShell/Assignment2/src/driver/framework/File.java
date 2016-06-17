package driver.framework;

/**
 * A command that displays the content of file pointed to
 * 
 * @author songzh
 */
import driver.Exceptions.*;

public class File extends Directory {
  private String content;

  /**
   * A constructor of class File
   * 
   */
  public File() {

    this.content = "";
  }

  /**
   * A method sets up the content
   * 
   * @param content
   */
  public void set_content(String content) {
    this.content = content;

  }

  /**
   * This method returns the content
   * 
   * @return content
   */
  public String get_content() {
    return this.content;

  }

  public static File find_file(FileSystem FS, String path)
      throws JShellException {

    boolean found = false; // flag for found of not
    Directory start = new Directory(); // This is our probe
    String[] path_layer = path.split("/");
    if (path.charAt(0) == '/') { // absolute path
      start = FS.get_root();
    } else { // relative path
      start = FS.get_cur_dir();
    }

    for (int i = 0; i < path_layer.length; i++) {

      found = false;
      for (Directory dir : start.get_child()) {
        if (dir.get_dir_names().equals(path_layer[i])) {
          start = dir;
          found = true;
        }
      }
    }
    if (found) {
      if (start.getClass().getName().equals("driver.framework.File")) {
        return (File) start;
      } else {
        throw (new JShellException(start.get_dir_names() + " is a Directory"));
      }
    }

    return null;
  }

  public static File create_file(FileSystem FS, String path)
      throws JShellException {
    File tester = File.find_file(FS, path);
    if (tester != null) {
      throw new JShellException(path + ": File already exsited");
    }

    String parent_path;
    // flag for found of not
    if (path.lastIndexOf('/') < 0) {
      File new_file = new File();
      new_file.set_dir_name(path);
      FS.get_cur_dir().add_dir(new_file);
      return new_file;
    }
    if (path.lastIndexOf('/') == 0) {
      parent_path = "/";
    } else {
      parent_path = path.substring(0, path.lastIndexOf('/'));
    }
    // System.out.println("Index of last / is : "+path.lastIndexOf('/'));
    // System.out.println("Parent directory of the file is: "+parent_path);
    try {
      Directory result = Directory.find_dir(FS, parent_path);
      File new_file = new File();
      new_file.set_dir_name(
          path.substring(path.lastIndexOf('/') + 1, path.length()));
      result.add_dir(new_file);
      return new_file;

    } catch (JShellException je) {
      throw je;
    } catch (NullPointerException ne) {
      throw new JShellException("No such file or Directory");
    }



  }

  public static String get_path(Directory f) {
    String path = "";

    while (f.get_parent() != null) {
      String parent_name = f.get_dir_names();
      path = parent_name + "/" + path;
      f = f.get_parent();

    }
    path = "/" + path;
    return (path);

  }

}

