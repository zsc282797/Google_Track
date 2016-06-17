package driver.framework;

import java.util.*;
import driver.Exceptions.*;

/**
 * This class sets up a directory system contains directory
 *
 */
public class Directory {
  private String dir_name;
  private Directory parent;
  private ArrayList<Directory> child;

  /**
   * A constructor of class Directory
   * 
   * @param dir_name
   * @param Parent
   * @param child
   */
  public Directory(String dir_name, Directory Parent,
      ArrayList<Directory> child) {
    this.dir_name = dir_name;
    this.parent = Parent;
    this.child = child;
  }

  public Directory(String dir_name) {
    this.dir_name = dir_name;
    this.parent = null;
    this.child = new ArrayList<Directory>();
  }

  public Directory() {
    this.dir_name = "";
    this.parent = null;
    this.child = new ArrayList<Directory>();

  }

  /**
   * This method sets up the directory name
   * 
   * @param dir_name
   */
  public void set_dir_name(String dir_name) {

    this.dir_name = dir_name;

  }

  /**
   * This method gets the directory name
   * 
   * @return the directory's name
   */
  public String get_dir_names() {

    return this.dir_name;
  }

  /**
   * This method sets up a parent directory
   * 
   * @param parent
   */
  public void set_parent(Directory parent) {

    this.parent = parent;

  }

  /**
   * This class returns a parent directory
   * 
   * @return
   */
  public Directory get_parent() {

    return this.parent;
  }

  /**
   * This class sets up a child directory
   * 
   * @param child
   */
  public void set_child(ArrayList<Directory> child) {
    this.child = child;

  }

  /**
   * This method returns the child directory
   * 
   * @return value of child directory
   */
  public ArrayList<Directory> get_child() {
    return this.child;
  }

  /**
   * This method add a child directory under parent directory
   * 
   * @param dir
   */
  public void add_dir(Directory dir) {
    if (this.child == null) {

      this.child = new ArrayList<Directory>();
    } else {
    }
    this.child.add(dir);
    dir.set_parent(this);

  }

  public void remove_dir(Directory dir) {

    this.get_child().remove(dir);


  }

  public static Directory find_dir(FileSystem FS, String path)
      throws JShellException

  {
    boolean found = false; // flag for found of not
    Directory start = new Directory(); // This is our probe
    String[] path_layer = path.split("/");
    if (path.lastIndexOf('/') == 0) {
      return FS.get_root();
    }
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
        throw (new JShellException(start.get_dir_names() + " is a File"));
      } else {
        return start;
      }
    }

    return null;
  }

  public static Directory find(FileSystem FS, String path)
      throws JShellException

  {
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
      return start;
    } else {
      throw (new JShellException("No such File or Directory"));
    }

  }

  public static ArrayList<File> get_all_files(Directory root) {
    ArrayList<File> files = new ArrayList<File>();
    get_rec(root, files);
    return files;
  }

  public static void get_rec(Directory root, ArrayList<File> found) {
    for (Directory dir : root.get_child()) {
      if (dir.getClass().getName().equals("driver.framework.File")) {
        found.add((File) dir);
      } else {
        get_rec(dir, found);
      }
    }


  }

  public static ArrayList<Directory> get_all_dirs(Directory root) {
    ArrayList<Directory> dirs = new ArrayList<Directory>();
    get_rec_dir(root, dirs);
    return dirs;
  }

  public static void get_rec_dir(Directory root, ArrayList<Directory> found) {
    for (Directory dir : root.get_child()) {

      found.add(dir);

      get_rec_dir(dir, found);
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
    return (path.substring(0, path.lastIndexOf('/')));

  }
}
