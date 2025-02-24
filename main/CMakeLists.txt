idf_component_register(SRCS "main.cpp" "./internal_components/screen_manager/screen_manager.cpp" "./internal_components/button_manager/button_manager.cpp" "./views/apps/clock/clock_view.cpp"
                   INCLUDE_DIRS "."
                   )


idf_build_set_property(PARTITION_TABLE_FILENAME partitions.csv)