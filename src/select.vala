using GLib;

namespace GGobi {
  public enum MatchMethod {
    IDENTICAL,
    INCLUDES,
    STARTS_WITH,
    ENDS_WITH,
    EXCLUDES
  } 
}

public abstract class GGobi.Select : Object {
  public abstract void select(StageSubset stage);
  
  public virtual bool equals(Select that) {
    return this.get_type() == that.get_type();
  }
  
  // public virtual bool check_params(StageSubset stage) {
  //   return true;
  // }
  
  public abstract string description();
  
}

public class GGobi.SelectAll : Select {
  override void select(StageSubset stage) {
    stage.set_included_all(true);
  }
  
  override string description() {
    return "All";
  }
  
}


// Select n random observations
// Algorithm taken from Knuth, Seminumerical Algorithms; Vol 2
public class GGobi.SelectRandom : Select {
  public int n {get; set construct;}

  public SelectRandom(construct int n) {}

  override void select(StageSubset stage) {
    uint n_rows = stage.parent.n_rows;
    bool[] included = new bool[n_rows];

    if (n < 0 || n > n_rows) 
      return;

    stage.set_included_all(false);

    int t, m;
    for (t = 0, m = 0; t < n_rows && m < n; t++) {
      double r = Random.double();
      if (((n_rows - t) * r) < (n - m) && !included[t]) {
        stage.set_included(t, true);
        included[t] = true;
        m++;
      }
    }
  }
  
  override bool equals(Select that) {
    return base.equals(that) && 
      this.n == ((SelectRandom) that).n;
  }
  
  override string description() {
    return n.to_string("Random (%i)");
  }
  
}

// Select a contiguous block of data, from start to start + size
public class GGobi.SelectBlock : Select {
  public int start {get; set construct;}
  public int size {get; set construct;}
  
  public SelectBlock(construct int start, construct int size) {}
  
  override void select(StageSubset stage) {
    uint n_rows = stage.parent.n_rows;
    int i, k;

    if (start < 0 || start > n_rows || size < 0) {
      GGobi.message("The limits aren't correctly specified.", false);
      return;
    }

    stage.set_included_all(false);
    for (i = start, k = 1; i < n_rows && k <= size; i++, k++) {
      stage.set_included(i, true);
    }
  }
  
  override bool equals(Select that) {
    return base.equals(that) && 
      this.start == ((SelectBlock) that).start &&
      this.size  == ((SelectBlock) that).size;
  }
  
  override string description() {
    return "Block [%i-%i]".printf(start, start + size);
  }
}

// Select every step points, starting from start.
public class GGobi.SelectEveryN : Select {
  public int start {get; set construct;} 
  public int step  {get; set construct;}

  public SelectEveryN(construct int start, construct int step) {}

  override void select(StageSubset stage) {
    uint n_rows = stage.parent.n_rows;
    if (start < 0 || start > n_rows - 1 || step < 0 || step > n_rows) {
      GGobi.message ("Interval not correctly specified.", false);
      return;
    }
  
    stage.set_included_all(false);
    for(uint i = start; i < n_rows; i += step)
      stage.set_included(i, true);
  }
  
  override bool equals(Select that) {
    return base.equals(that) && 
      this.start == ((SelectEveryN) that).start &&
      this.step  == ((SelectEveryN) that).step;
  }
  
  override string description() {
    return "Every %i from %i".printf(step, start);
  }
  
}


// Selects records by their row id, according to @method.
public class GGobi.SelectLabel : Select {
  // substring matched against rownames
  public string substr {get; set construct;}

  // matching method
  public MatchMethod method {get; set construct;}

  //  TRUE for case insensitive matching
  public bool ignore_case {get; set construct;}
  
  override void select(StageSubset stage) {
    uint n_rows = stage.parent.n_rows;
    bool value;
  
    if (substr == null || substr.len() == 0) {
      stage.set_included_all(true);
      return;
    }

    stage.set_included_all(false);
    string search = substr; // needed for vala
    if (ignore_case) search = search.down();

    for (uint i = 0; i < n_rows; i++) {
      string label = stage.get_row_id(i);
    
      if (label == null) continue;
      if (ignore_case) label = label.down();
    
      switch(method) {
        case MatchMethod.INCLUDES:
          value = label.str(search) != null;
          break;
        case MatchMethod.EXCLUDES:
          value = label.str(search) == null;
          break;
        case MatchMethod.IDENTICAL:
          value = (label == search);
          break;
        case MatchMethod.ENDS_WITH:
          value = label.has_suffix(search);
          break;
        case MatchMethod.STARTS_WITH:
          value = label.has_prefix(search);        
          break;
      }   
      stage.set_included(i, value);
    }
  }
  
  override bool equals(Select that) {
    return base.equals(that) && 
      this.substr == ((SelectLabel) that).substr &&
      this.method == ((SelectLabel) that).method &&
      this.ignore_case == ((SelectLabel) that).ignore_case;
  }
  
  override string description() {
    return "Label";
  }
}
