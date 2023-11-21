set(srcs "")
file(GLOB_RECURSE all_c_files "./*.c")
list(APPEND srcs "${all_c_files}")

idf_component_register(SRCS "${srcs}"
                      INCLUDE_DIRS "." "sesame" "utils")
