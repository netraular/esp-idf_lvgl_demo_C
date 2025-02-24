idf_component_register(SRCS "main.cpp" "./views/base_view.cpp" "./controllers/screen_manager/screen_manager.cpp" "./controllers/button_manager/button_manager.cpp" "./views/apps/clock/clock_view.cpp"  "./views/system/boot_screen/boot_view.cpp" "./views/system/settings/settings_view.cpp" "./views/system/system_info/system_info_view.cpp"
                   INCLUDE_DIRS "."
                   )


idf_build_set_property(PARTITION_TABLE_FILENAME partitions.csv)