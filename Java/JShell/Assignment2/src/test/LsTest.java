import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.Before;
import org.junit.Test;

public class LsTest {

  @Before
  public void setUp() throws Exception {}

  @Test
  public void testIsValid_empty() {
    ArrayList<String> test_params = new ArrayList<String>();
    Ls test_ls = new Ls();
    test_ls.set_cmd_params(test_params);
    assertFalse(test_ls.isValid());
    
  }

  @Test
  public void testRun_correct_children() {
    FileSystem test_fs = new FileSystem();
    Directory test_a = new Directory();
    ArrayList<String> test_params = new ArrayList<String>();
    test_a.set_dir_name("test_a");
    test_fs.get_cur_dir().add_dir(test_a);
    
    test_params.add("test_a");
    
    Ls test_ls = new ls();
    test_ls.set_cmd_params(test_params);
    
    try {
      String result = test_ls.Run(test_fs);
    }
    catch (JShellException je){
      fail(je.get_message());
      
    }
    assertEquals("test_a",test_fs.get_cur_dir().get_dir_names());
    
  }

}


}
