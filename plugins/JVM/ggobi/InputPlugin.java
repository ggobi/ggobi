package ggobi;

public interface InputPlugin {
  public int      getNumRecords();
  public int      getNumVariables();
  public String[] getVariableNames();

    /**
       Get a long or short description of this input source
       @param brief a logical value indicating whether to return
          the brief description that can be used as the file name
          or input source in the menus, etc.
         or, if FALSE, the long version the gives a more
         comprehensive description of the data.
     */
  public String   getSourceDescription(boolean brief);
}

