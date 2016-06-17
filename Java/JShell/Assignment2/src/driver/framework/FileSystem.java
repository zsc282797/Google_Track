package driver.framework;

import java.util.*;

public class FileSystem {

  private Directory root;
  private Directory current_directory;
  private CmdHistory CH;
  private DirStack DS = new DirStack();

  public FileSystem() {
    this.root = new Directory("/", null, new ArrayList<Directory>());
    this.current_directory = root;

  }

  public void set_root(Directory root) {

    this.root = root;
  }

  public Directory get_root() {

    return this.root;
  }

  public void set_cur_dir(Directory cur_dir) {
    this.current_directory = cur_dir;

  }

  public Directory get_cur_dir() {
    return this.current_directory;
  }

  public void set_CH(CmdHistory CH) {
    this.CH = CH;
  }

  public CmdHistory get_CH() {
    return this.CH;
  }

  public void add_CH(String cmd) {
    this.CH.add(cmd);
  }

  public void set_DS(DirStack DS) {
    this.DS = DS;
  }

  public DirStack get_DS() {
    return this.DS;
  }
}
