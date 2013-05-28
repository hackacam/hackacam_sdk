# Base rules to create object files, install header, script and conf files and create output directories

$(OBJ_DIR)/%.o : %.cpp | $(OBJ_DIR)/
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o : %.c | $(OBJ_DIR)/
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(INCL_DIR)/$(PACKAGE)/%.h : %.h 
	install -D -m 644 $< $@

$(OBJ_DIR)/ $(BIN_DIR)/ $(LIB_DIR)/ $(TEST_DIR)/ $(CONF_DIR)/ $(HTML_DIR)/ :
	@mkdir -p $@

$(BIN_DIR)/% : $(SCRIPT_SOURCE_DIR)/% | $(BIN_DIR)/
	install -D -m 755 $< $@

$(CONF_DIR)/% : $(CONF_SOURCE_DIR)/% | $(CONF_DIR)/
	install -D -m 644 $< $@

$(HTML_DIR)/% : $(HTML_SOURCE_DIR)/% | $(HTML_DIR)/
	cp -rf $< $@

-include $(OBJ_DEPS)

.PHONY: all clean clear

