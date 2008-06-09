using GLib;
using Xml;
using Gsf;

public class GGobi.DataFactoryXML : DataFactory {

  override SList<Data>? create_for_input(Input input) {
    return read_xml_data(input);
  }
  
  private InputMode _mode = new InputMode("xml", new string[] { "xml" });
  override InputMode mode {
    get { return _mode; }
  }

  private SList<Data>? read_xml_data(Input input) {
    ParserState state = new ParserState();
    XMLIn handlers = new XMLIn(nodes, ns);
    bool result = handlers.parse(input, context, state);
    if (result)
      return state.datasets;
    else return null;
  }

  /* a simpler structure for specifying schema, needs to be converted */
  private struct ParserNode {
    public string name;
    public string parent;
    public XmlInNodeStart start;
    public XmlInNodeEnd end;
    public bool has_content;
  }

  private const ParserNode[] nodes = {
    /* name, parent, start, end, content */
    { "ggobidata", "start", null, null, false },
    { "data", "ggobidata", start_data, end_data, false },
    { "records", "data", start_records, null, false },
    { "record", "records", start_record, end_record, true },
    { "edges", "ggobidata", start_edges, null, false },
    { "variables", "data", start_variables, null, false },
    { "realvariable", "variables", start_realvariable, end_variable, false },
    { "categoricalvariable", "variables", start_categoricalvariable, null, false
    },
    { "integervariable", "variables", start_integervariable, end_variable,
      false },
    { "countervariable", "variables", start_countervariable, end_variable,
      false },
    { "randomuniformvariable", "variables", start_randomuniformvariable,
      end_variable, false },
    { "levels", "categoricalvariable", start_levels, null, false },
    { "level", "levels", start_level, end_level, true },
    { "real", "record", null, end_real, true },
    { "integer", "record", null, end_integer, true },
    { "string", "record", null, end_string, true },
    { "na", "record", null, end_na, false },
  };

  private static XMLInNode[] in_nodes;
  
  /* static constructor */
  private static DataFactoryXML() {
    in_nodes = new XMLInNode[parser_nodes.length + 1];
    /* "start" is a dummy node with a null name and itself as a parent */
    XMLInNode start = new XMLInNode();
    start.id = "start";
    start.ns_id = -1;
    start.parent_id = "start";
    in_nodes[0] = start;
    for (int i = 0; i < parser_nodes.length; i++) {
      XMLInNode in_node = new XMLInNode();
      ParserNode node = parser_nodes[i];
      in_node.id = node.name = node.name;
      in_node.ns_id = -1; // FIXME: shouldn't ggobi elements have a namespace?
      in_node.parent_id = node.parent;
      in_node.start = node.start;
      in_node.end = node.end;
      in_node.has_content = node.has_content ? XMLContent.CONTENT :
        XMLContent.NO_CONTENT;
      in_nodes[i + 1] = in_node;
    }
  }
  
  private void start_data(XmlIn xin, string[] attrs) {
    set_dataset(xin, attrs);
  }
  private void end_data(XmlIn xin, XMLBlob unknown) {
    ParserState state = (ParserState)xin.user_data;
    Data data = state.current_data;
    set_edge_partners(state);
    if (state.current_record < data.n_rows)
      emit_warning("There are fewer records than declared for '%s': %d < %d.",
                   data.name, state.current_record, data.n_rows);
    state.datasets.append(data);
  }

  private void start_edges(XmlIn xin, string[] attrs) {
    set_dataset(xin, attrs);
    set_dataset_info(xin, attrs);
  }
  
  private void start_record(XmlIn xin, string[] attrs) {
    ParserState state = (ParserState)xin.user_state;
    Data data = state.current_data;
    int i = state.current_record;
    
    if (i == data.n_rows)
      emit_warning(state, "Too many records for '%s'", data.name);
    
    state.current_element = 0;
    
    set_color(attrs, state, i);
    set_glyph(attrs, state, i);
    set_hidden(attrs, state, i);
    
    string id_attr = get_attribute(attrs, "id");
    if (id_attr != null) {
      int m = data.get_row_for_id(id_attr);
      if (m != state.current_record)
        emit_warning(state, "duplicate id (%s) in records %d and %d in %s",
                     id_attr, state.current_record + 1, m + 1, data.name);
      data.set_row_id(i, id_attr);
    }

    /* edges */

    string source_attr = get_attribute(attrs, "source");
    if (source_attr != null) {
      string dest_attr = get_attribute(attrs, "destination");
      if (dest_attr == null)
        emit_value_warning(state, "Edge", source_attr,
                           "source but no destination attribute");
      if (strcmp(source_attr, dest_attr) == 0)
        emit_value_warning(state, "Edge", source_attr,
                           "source is the same as destination");
    }
    
    return true;
  }
  private void end_record(XmlIn xin, XmlBlob unknown) {
    ParserState state = (ParserState)xin.user_state;
    set_record_values(state, xin.content.str);
    state.current_record++;
  }

  private void start_records(XmlIn xin, string[] attrs) {
    set_dataset_info(xin, attrs);
  }

  private void start_variables(XmlIn xin, string[] attrs) {
    string count_attr = get_attribute(attrs, "count");
    ParserState state = (ParserState)xin.user_state;
    Data d = state.current_data;
    if (count_attr != null)
      emit_warning("No count attribute for variables element in '%s'", d.name);
    data.add_cols(count_attr.to_int());
    data.add_attributes();
    state.var_to_col = data.translate_var_to_col();
    return(true);
  }

  private void start_realvariable(XmlIn xin, string[] attrs) {
    set_variable(xin, attrs, VariableType.REAL);
  }
  private void start_categoricalvariable(XmlIn xin, string[] attrs) {
    set_variable(xin, attrs, VariableType.CATEGORICAL);
  }
  private void start_integervariable(XmlIn xin, string[] attrs) {
    set_variable(xin, attrs, VariableType.INTEGER);
  }
  private void start_countervariable(XmlIn xin, string[] attrs) {
    set_variable(xin, attrs, VariableType.COUNTER);
  }
  private void start_randomuniformvariable(XmlIn xin, string[] attrs) {
    set_variable(xin, attrs, VariableType.UNIFORM);
  }

  private void end_variable(XmlIn xin, XmlBlob unknown) {
    ((ParserState)xin.user_state).current_variable++;
  }
  
  private void start_levels(XmlIn xin, string[] attrs) {
    ParserState state = (ParserState)xin.user_state;
    Data data = state.current_data;
    string count = get_attribute(attrs, "count");
    state.current_nlevels = G_MAXINT;
    if (count != null) {
      state.current_nlevels = count.to_integer();
      if (state.current_nlevels < 1)
        emit_warning("Level count (%d) for %s misspecified",
                     state.current_nlevels,
                     data.get_col_name(state.current_variable));
    }
    state.current_level = 0;
  }

  private void start_level(XmlIn xin, string[] attrs) {
    ParserState state = (ParserState)xin.user_state;
    Data data = state.current_data;

    if (state.current_level >= state.current_nlevels)
      emit_warning(state, "too many levels (%d) for variable '%s' in '%s'",
                   state.current_level,
                   data.get_col_name(state.current_variable), data.name);

    int value = state.current_level + 1;
    string value_attr = get_attribute(attrs, "value");
    if (value_attr != null)
      value = value_attr.to_integer();
    state.current_level_value = value;
    
    state.current_level++;
  }
  private void end_level(XmlIn xin, XmlBlob unknown) {
    ParserState state = (ParserState)xin.user_state;
    Data data = state.current_data;
    Variable var = data.get_variable(state.current_variable);
    var.add_level(state.content.str, state.current_level_value);
  }

  private void end_real(XmlIn xin, XmlBlob unknown) {
    end_value(xin, VariableType.REAL);
  }
  private void end_integer(XmlIn xin, XmlBlob unknown) {
    end_value(xin, VariableType.INTEGER);
  }
  private void end_string(XmlIn xin, XmlBlob unknown) {
    end_value(xin, VariableType.CATEGORICAL);
  }

  private void end_value(XmlIn xin, VariableType type) {
    ParserState state = (ParserState)xin.user_state;
    Data data = state.current_data;
    Variable var = data.get_variable(state.current_element);
    if (var.type != type)
      emit_warning(state, "Element %d:%d of type %d but variable of type %d",
                   state.current_record, state.current_element, type, var.type);
    set_record_value(state, xin.content.str);
    state.current_element++;
  }

  private void end_na(XmlIn xin, XmlBlob unknown) {
    ParserState state = (ParserState)xin.user_state;
    state.current_data.set_missing(state.current_record, state.current_element);
    state.current_element++;
  }
    
  private bool set_dataset(XmlIn xin, string[] attrs) {
    Data data = new Data(0, 0);
    string name = get_attribute(attrs, "name");
    if (name == null)
      emit_warning(state, "No name attribute");
    else data.set_name(name);
    string nickname = get_attribute(attrs, "nickname");
    if (nickname == null)
      nickname = name;
    data.set_nickname(nickname);
    ((ParserState)xin.user_state).current_data = data;
    ((ParserState)xin.user_state).counter_variable_index = -1;
    return true;
  }

  private void set_dataset_info(XmlIn xin, string[] attrs) {
    string count = get_attribute(attrs, "count");
    if (count == null)
      emit_warning(state, "No count attribute");

    ParserState state = (ParserState)xin.user_state;
    Data data = state.current_data;
    data.add_rows(count.to_int);

    string missingValue = get_attribute(attrs, "missingValue");
    if (missingValue != null)
      state.NA_identifier = missingValue;
    
    set_glyph(attrs, state, -1);
    set_color(attrs, state, -1);
    set_hidden(attrs, state, -1);
    
    state.current_variable = 0;
    state.current_record = 0;
    state.current_element = 0;
    state.autoLevels = null;
    
    return true;
  }

  /* for parsing the glyph specs */
  private static Regex space_splitter = new Regex(" ");
  
  private bool set_glyph(string[] attrs, ParserState state, int i) {
    Data data = state.current_data;
    int size = state.defaults.glyphSize;
    string size_attr = get_attribute(attrs, "glyphSize");
    if (size_attr != null)
      size = size_attr.to_int();
    int type = state.defaults.glyphType;
    string type_attr = get_attribute(attrs, "glyphType");
    if (type_attr != null) {
      type = mapGlyphName(type_attr);
      if (type == GlyphType.UNKNOWN_GLYPH) {
        int type_int = type_attr.to_int();
        if (type_int >= 0 && type_int < GlyphType.NGLYPHTYPES)
          type = type_int;
      }
    }
    string glyph_attr = get_attribute(attrs, "glyph");
    if (glyph_attr != null) {
      string[] split = space_splitter.split(glyph_attr);
      if (split.length != 2)
        emit_value_warning(state, "glyphSize", glyph_attr, "Out of range");
      else {
        type = mapGlyphName(split[0]);
        size = split[1].to_int();
      }
    }
    /* check type and size
       if OK and i < 0, save as defaults
       if BAD, get from default
    */
    if (size < 0 || size >= NGLYPHSIZES) {
      emit_value_warning(state, "glyphSize", size_attr, "Out of range");
      type = state.defaults.glyphType;
    } else if (i < 0)
      state.defaults.glyphSize = size;
    if (type == GlyphType.UNKNOWN_GLYPH) {
      emit_value_warning(state, "glyphType", type_attr, "Out of range");
      size = state.defaults.glyphSize;
    } else if (i < 0)
      state.defaults.glyphType = type;
    if (i >= 0) { /* set attributes on the record */
      GGOBI_STAGE_ATTR_INIT_ALL(data);
      GGOBI_STAGE_SET_ATTR_TYPE(data, i, type, ATTR_SET_SAVE);
      GGOBI_STAGE_SET_ATTR_SIZE(data, i, size, ATTR_SET_SAVE);
    }
    return size != -1 && type != -1;
  } 

  private bool set_color(string[] attrs, ParserState state, int i) {
    int color = state.defaults.color;
    Data data = state.current_data;

    string color_attr = get_attribute(attrs, "color");
    if (color_attr != null) {
      int color_int = color_attr.to_int();
      if (color_int < 0 || color_int >= MAXNCOLORS)
        emit_value_warning(state, "color", color_attr, "Out of range");
      else {
        color = color_int;
        if (i < 0)
          state.defaults.color = color;
      }
    }
    if (i >= 0) {
      GGOBI_STAGE_ATTR_INIT_ALL (data);
      GGOBI_STAGE_SET_ATTR_COLOR (data, i, color, ATTR_SET_SAVE);
    }
    return color != -1;
  }

  private bool set_hidden(string[] attrs, ParserState state, int i) {
    Data data = state.current_data;
    string hidden_attr = get_attribute(attrs, "hidden");
    if (hidden_attr != null) {
      bool hidden = as_logical(hidden_attr);
      if (i < 0)
        state.defaults.hidden = hidden;
      else {
        GGOBI_STAGE_ATTR_INIT_ALL (d);
        GGOBI_STAGE_SET_ATTR_HIDDEN (data, i, hidden, ATTR_SET_SAVE);
      }
    }
    return (hidden_attr != null);
  }

  private void apply_random_uniforms(ParserState state) {
    Data data = state.current_data;
    uint ncols = data.get_n_vars();
    for (int j = state.current_element; j < ncols &&
           data.get_col_type(j) == VariableType.UNIFORM; j++) {
      set_data_value(state, Random.next_double());
    }
  }
  
  /* for parsing record values */
  private static Regex white_splitter = new Regex(" \n\t");
  
  private bool set_record_values(ParserState state, string line) {
    Data data = state.current_data;
    int ncols = data.get_n_vars();
    if (line == null) {
      apply_random_uniforms(state);
      return(false);
    }
    string[] elements = white_splitter.split(line);
    for (element in elements)
      if (!set_record_value(state, element))
        return(false);
    if (state.current_element < ncols)
      emit_warning(state, "Not enough elements in record %d, dataset '%s'",
                   state.current_record, data.name);
    return(true);
  }

  private void set_record_value(ParserState state, string value) {
    Data data = state.current_data;
    if (!data.has_vars())
      return(true);
    if (state.current_element == state.counter_variable_index) {
      set_data_value(state, state.current_record + 1);
      state.current_element++;
    }
    if (state.current_record >= data.n_rows ||
        state.current_element >= data.get_n_vars()) {
      emit_warning("Record %d has too many elements", state.current_record + 1);
      state.current_element = 0;
      return(false);
    }
    apply_random_uniforms(state);
    if ((state.NA_identifier && !strcmp(value, data.NA_identifier)) ||
        !strcmp(value, "na") || !strcmp(value, "NA") || !strcmp(value, "."))
      data.set_missing(state.current_record, state.current_element);
    else {
      Variable var = data.get_variable(state.current_element);
      double num = as_number(value);
      if (var.is_categorical()) {
        if (state.auto_levels != null  &&
            state.auto_levels[state.current_element])
          data.set_categorical_value(state.current_record,
                                     state.current_element, value);
        else if (!var.has_level(value)) {
          emit_warning("Bad level in record %d, variable '%s', dataset '%s'",
                       state.current_record + 1, var.name, data.name);
          set_data_value(state, num);
        }
      } else set_data_value(state, num);
    }
    return(true);
  }

  private void set_data_value(ParserState state, double value) {
    uint col = state.var_to_col[state.current_element];
    data.set_raw_value(state.current_element, col, value);
  }

  private void set_variable(XmlIn xin, string[] attrs, VariableType type) {
    ParserState state = (ParserState)xin.user_state;
    Data data = state.current_data;
    if (state.current_variable >= data.get_n_vars())
      emit_warning("More variables (%d) than expected (%d) for '%s'",
                   state.current_variable, data.get_n_vars(), data.name);
    Variable var = data.get_variable(state.current_variable);
    var.name = get_attribute(attrs, "name");
    string nickname = get_attribute(attrs, "nickname");
    if (nickname != null)
      var.nickname = nickname;
    string min_attr = get_attribute(attrs, "min");
    string max_attr = get_attribute(attrs, "max");
    if (min_attr != null && max_attr != null) {
      double min = as_number(min_attr), max = as_number(max_attr);
      var.lim_specified.min = min < max ? min : max;
      var.lim_specified.max = min > max ? min : max;
      if (min > max)
        emit_warning("Min (%f) is greater than max (%f) for var '%s' in '%s'",
                     min, max, var.name, data.name);
      var.lim_specified = true;
    }
    var.type = type;
    if (type == VariableType.CATEGORICAL) {
      string levels = get_attribute(attrs, "levels");
      if (strcmp(levels, "auto") == 0) {
        if (state.auto_levels == null)
          state.auto_levels = new bool[data.get_n_vars()];
        state.auto_levels[state.current_variable] = true;
      }
    } 
  }

  private void edge_compare(void* val1, void* val2) {
    SortableEndpoints e1 = (SortableEndpoints)val1;
    SortableEndpoints e2 = (SortableEndpoints)val2;
    acomp = strcmp(e1.a, e2.a);
    bcomp = strcmp(e1.b, e2.b);
    if (acomp < 0 || (acomp == 0 && bcomp < 0))
      return(-1);
    if (acomp == 0 && bcomp == 0)
      return(0);
    return(1);
  }
  
  private void set_edge_partners(ParserState state) {
    Data e = state.current_data;
    EdgeData ed = e.get_edge_data();
    int ne = e.get_n_edges();
    if (ne == 0)
      return;
    int n = 2 * ne, k = 0;
    SortableEndpoints ep = new SortableEndpoints[n];
    for (int i = 0; i < ne; i++) {
      ep[k].a = ed.sym_endpoints[i].a;
      ep[k].b = ed.sym_endpoints[i].b;
      ep[k].jcase = i;
      k++;
    }
    // VALABUG: how to qsort? (GLib bindings insufficient)
    /*qsort ((gchar *) ep, ggobi_stage_get_n_edges (e),
      sizeof (SortableEndpoints), self_edge_compare);*/
    for (int i = 0; i < ne; i++) {
      k = i - 1;
      if (strcmp(ep[i].a, ep[k].a) == 0 && strcmp(ep[i].b, ep[k].b) == 0) {
        emit_warning("Found duplicate edge from %s to %s",
                     ed.sym_endpoints[ep[i].jcase].a,
                     ed.sym_endpoints[ep[i].jcase].b);
      }
    }
    for (int i = 0, k = ne; i < ne; i++) {
      ep[k].a = ed.sym_endpoints[i].b;
      ep[k].b = ed.sym_endpoints[i].a;
      ep[k].jcase = i;
      k++;
    }
    /*qsort ((gchar *) ep, n, sizeof (SortableEndpoints), self_edge_compare);*/
    for (int i = 0; i < n; i++) {
      k = i - 1;
      if (strcmp(ep[i].a, ep[k].a) == 0 && strcmp(ep[i].b, ep[k].b) == 0) {
        ed.sym_endpoints[ep[i].jcase].jpartner = ep[k].jcase;
        ed.sym_endpoints[ep[k].jcase].jpartner = ep[i].jcase;
      }
    }  
  }
  
  private string? get_attribute(string[] attrs, string name)
  {
    for (uint i = 0; attrs != null && attrs[i] != null; i += 2) {
      if (strcmp (name, attrs[i]) == 0)
        return attrs[i+1];
    }
    return null;
  }

  private struct DataDefaults {
    int color = -1;
    int glyphType;
    int glyphSize;
    int hidden;
  }
    
  private class ParserState {
    public ParserState() {
      defaults.glyphType = sessionOptions.info.glyph.type;
      defaults.glyphSize = sessionOptions.info.glyph.size;
    }
    
    public SList<Data> datasets; /*  The list of all datad's read. */

    public Data current_data; /* The current data object being built. */
    public int current_variable; /* Indexes the current variable. */
    public int current_record; /* Indexes the current record. */
    public int current_element; /* Indexes the values within a record. */

    public int counter_variable_index; /* the variable that is a counter */
    
    public string NA_identifier; /* The dataset missing value identifier. */

    public DataDefaults defaults; /* Attribute defaults from records/edges */

    public bool[] auto_levels; /* automatically determine levels for a var? */
    public int current_level; /* index of current level for categorical var */
    public int current_nlevels; /* number of levels in current variable */
    public int current_level_value; /* the numeric value for current level */
  }
}