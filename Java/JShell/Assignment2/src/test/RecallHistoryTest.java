package test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import driver.Exceptions.JShellException;
import driver.command.RecallHistory;
import driver.framework.CmdHistory;
import driver.framework.File;
import driver.framework.FileSystem;

public class RecallHistoryTest {
  private FileSystem myFileSystem;
  private RecallHistory myRecallHistory;
  private CmdHistory myCmdHistory;
  private ArrayList<String> myParams;

  @Before
  public void setUp() throws Exception {
    myFileSystem = new FileSystem();
    myRecallHistory = new RecallHistory();
    myCmdHistory = new CmdHistory();
    myFileSystem.set_CH(myCmdHistory);
    myParams = new ArrayList<String>();

  }

  @After
  public void tearDown() throws Exception {}

  @Test
  public void testIsValid() {
    myParams.add("2");
    myRecallHistory.set_cmd_params(myParams);
    assertTrue(myRecallHistory.isValid());
    myParams.add("3");
    assertFalse(myRecallHistory.isValid());
  }

  @Test
  public void testRun() {
    myCmdHistory.add("echo \"hello \" > file1");
    try {
      File myFile1 = File.create_file(myFileSystem, "/file1");;
    } catch (JShellException e1) {
      // TODO Auto-generated catch block
      e1.printStackTrace();
    }
    myCmdHistory.add("cat file1");
    myCmdHistory.add("! 2");
    myParams.add("2");
    myRecallHistory.set_cmd_params(myParams);
    try {
      String outcome = myRecallHistory.Run(myFileSystem);
      assertEquals("hello", outcome);
    } catch (JShellException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }

    myParams.clear();
    myParams.add("3");
    try {
      String outcome = myRecallHistory.Run(myFileSystem);
      assertEquals(1, 2);
    } catch (JShellException e) {
      String errorMessage = "Invalid command: the command you want to recall "
          + "is just the recall command you typed just now, please try another"
          + " one!";
      assertEquals(errorMessage, e.get_message());
    }

    myParams.clear();
    myParams.add("4");
    try {
      String outcome = myRecallHistory.Run(myFileSystem);
      assertEquals(1, 2);

    } catch (JShellException e) {
      String errorMess = "Invalid command: the index of recall command exceeds "
          + "the number of history commands, please try a new index!";
      assertEquals(errorMess, e.get_message());
    }

  }

}
