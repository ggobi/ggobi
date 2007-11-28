/* 
= GUI element =

This class encapsulates the behaviour needed by a GUI element which displays
the state of a gui-stage support class, e.g. an imputation, a transformation
or a selection. (This may grow in future to include states which are currently
stored an an enum, like randomisation and jittering).

The GUI element needs to be flexible enough to be displayed in different ways,
depending on the amount of space available, and the number of other options.
This is hard to do in general, but as most cases in GGobi only require one or
two other parameters, should be feasible here.

*/

public abstract class GuiComponent : GtkHBox {
  
  // The name of the element.  
  // This will be displayed as (e.g.) the text in combo boxes, the label in
  // tabs, and the first string in radio button groups
  public abstract string get_label();
  
  // Builds box based gui
  // Subclasses should override this method, add additional gui components 
  // to the box.  I imagine that different display methods would supply 
  // different boxes - e.g. a VBox if displayed within a tab, or a HBox if
  // displayed next to a radio box.
  public virtual void build() {}

  // Build a radio button for this element
  public virtual void build_radio_button() {
    RadioButton button = new RadioButton.with_label(null, get_label());
    button.toggled += (button) => {
      if (button.active) selected() else deselected();
    }
    pack_start(button, false, false, 2);
    build_box();
  }
  
  construct {
    build_box();
  }
  
  // The element needs to be able to respond to the following events:
  //  * the underlying object has been updated
  //  * the component has been disabled

  // It also needs to notify the rest of the gui when something changes:
  public signal void updated(); 
  
  public abstract Component get_component();
  public virtual void update_component(Component c);
  
}


public class GuiImputationMean : GuiElement {
  override string get_label() {
    return "Mean";
  }
  
  override Component get_component {
    return new ImputationMean();
  }
}

public class GuiImputationFixed : GuiElement {
  private HScale percent_widget;
  private double percent;
  
  override string get_label() {
    return "Percent";
  }
  
  override void build() {
    percent_widget = new HScale.with_range(-1, 1, 0.01);
    percent_widget.value_changed += percent_widget => {
      percent = percent_widget.value;
      updated();
    };
    pack_start(percent_widget);
  }
  
  override Component get_component() {
    ImputationPercent imp = new ImputationPercent();
    imp.percent = percent;
    return new imp();
  }
  
  override void update(Component c) {
    percent_widget.value = ((ImputationPercent) c).percent;
  }

}

// Public interface for objects that can be used with GuiElement
public interface Component {
  public bool equals(Component that);
  public string get_name();
  
}

