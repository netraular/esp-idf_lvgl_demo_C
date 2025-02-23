idf_component_register(SRCS "main.cpp" "screen_manager.cpp" "views/clock_view.cpp"
                   INCLUDE_DIRS "." "views" ".")
idf_build_set_property(PARTITION_TABLE_FILENAME partitions.csv)